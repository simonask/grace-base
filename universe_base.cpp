//
//  universe_base.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 08/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "object/universe_base.hpp"
#include "io/resource_manager.hpp"
#include "serialization/deserialize_object.hpp"
#include "serialization/serialize.hpp"
#include "base/parse.hpp"
#include "io/formatters.hpp"

namespace falling {
	void DeferredAttributeDeserialization::perform(IUniverse& universe) const {
		attribute->deserialize_attribute(object.get(), *node, universe);
	}

	bool UniverseBase::clear_and_instantiate(const ArchiveNode &scene_root, String &out_error) {
		clear();
		ObjectPtr<> ptr = deserialize_object(scene_root, *this);
		if (!ptr) {
			out_error = "There was an error deserializing the root object.";
			return false;
		}
		
		for (auto& deferred: deferred_) {
			deferred.perform(*this);
		}
		deferred_.clear();
		
		set_root(ptr);
		run_initializers();
		return true;
	}
	
	bool UniverseBase::serialize_root(ArchiveNode &root_node, falling::String &out_error) {
		falling::serialize(*root(), root_node, *this);
		return true; // XXX
	}
	
	void UniverseBase::defer_attribute_deserialization(ObjectPtr<> obj, const IAttribute *attr, const ArchiveNode *serialized) {
		deferred_.push_back(DeferredAttributeDeserialization{obj, attr, serialized});
	}
	
	bool BasicUniverse::retype_object(const StructuredType *new_type, StringRef object_id) {
		ASSERT(false); // NIY
		UNREACHABLE();
	}
	void error_category_already_initialized_with_different_type(StringRef name) {
		Error() << "Object category has already been initialized with a different type: " << name;
	}
	
	ObjectPtr<> BasicUniverse::create_object_and_set_as_root(const StructuredType* type, StringRef id) {
		clear();
		root_ = create_object(type, std::move(id));
		return root_;
	}
	
	ObjectPtr<> BasicUniverse::create_object(const StructuredType* type, StringRef id) {
		size_t sz = type->size();
		byte* memory = (byte*)allocator().allocate(sz, type->alignment());
		type->construct(memory, *this);
		Object* object = reinterpret_cast<Object*>(memory);
		memory_map_.push_back(object);
		rename_object(object, id);
		return object;
	}
	
	bool BasicUniverse::rename_object(ObjectPtr<> object, StringRef new_id) {
		ASSERT(object->universe() == this);
		
		// erase old name from database
		auto old_it = reverse_object_map_.find(object);
		if (old_it != reverse_object_map_.end()) {
			object_map_.erase(old_it->second);
		}
		
		// check if new name already exists
		auto it = object_map_.find(String(new_id));
		bool renamed_exact = true;
		String new_name;
		if ((it != object_map_.end()) || (new_id.size() < 2)) {
			// it does, so create a unique name from the requested name
			int n = 1;
			String base_name;
			if (new_id.size() >= 2) {
				Maybe<int> parsed = parse<int>(substr(new_id, new_id.size()-2, 2));
				maybe_if(parsed, [&](int parsed_number) {
					base_name = substr(new_id, 0, new_id.size()-2);
				}).otherwise([&]() {
					base_name = std::move(new_id);
				});
				n += 1; // n is set to 0 if recognition failed, otherwise the existing number. Add one. :)
			} else {
				base_name = object->object_type()->name();
			}
			
			// increment n and try the name until we find one that's available
			do {
				StringStream create_new_name;
				create_new_name << base_name << format("%02d", n);
				new_name = std::move(create_new_name.str());
				++n;
			} while (object_map_.find(new_name) != object_map_.end());
			
			renamed_exact = false;
		} else {
			new_name = std::move(new_id);
		}
		
		object_map_[new_name] = object;
		reverse_object_map_[object] = std::move(new_name);
		return renamed_exact;
	}
	
	StringRef BasicUniverse::get_id(ObjectPtr<const Object> object) const {
		auto it = reverse_object_map_.find(object);
		if (it != reverse_object_map_.end()) {
			return it->second;
		}
		return "";
	}
	
	void BasicUniverse::run_initializers() {
		for (auto pair: object_map_) {
			pair.second->initialize();
		}
	}
	
	void BasicUniverse::clear() {
		for (auto object: memory_map_) {
			const StructuredType* type = object->object_type();
			type->destruct(reinterpret_cast<byte*>(object), *this);
			allocator().free(object, type->size());
		}
		// TODO: Test for references?
		object_map_.clear();
		reverse_object_map_.clear();
		memory_map_.clear();
	}

}
