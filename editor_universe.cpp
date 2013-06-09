//
//  editor_universe.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 21/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "object/editor_universe.hpp"
#include "type/structured_type.hpp"
#include "base/priority_queue.hpp"
#include "base/parse.hpp"
#include "io/formatters.hpp"
#include "type/reference_type.hpp"
#include "type/attribute.hpp"
#include "serialization/binary_archive.hpp"
#include "serialization/serialize.hpp"
#include "serialization/deserialize_object.hpp"
#include "type/type_registry.hpp"
#include "base/log.hpp"

namespace grace {
	struct EditorUniverse::Impl {
		Map<StringRef, ObjectPtr<>> object_map_;
		Map<ObjectPtr<>, StringRef> reverse_object_map_;
		PriorityQueue<String> object_name_pool_;
		ObjectPtr<> root_;
		
		Array<ObjectPtrRootBase*> external_object_roots_;
		
		Impl(IAllocator& alloc) : object_map_(alloc), reverse_object_map_(alloc), object_name_pool_(alloc), external_object_roots_(alloc) {}
		void unregister_object(ObjectPtr<> obj);
		void unregister_object_name(StringRef object_id);
		void remove_object_references_from_object(ObjectPtr<> obj);
		void register_object_reference_in_object(ObjectPtr<> obj, const IAttribute* attr);
		
		template <typename F>
		void for_each_object_reference(ObjectPtr<> exclude_object, F f);
	};
	
	EditorUniverse::EditorUniverse() : UniverseBase(default_allocator()) {
		impl_ = new(allocator()) Impl(allocator());
	}
	
	EditorUniverse::~EditorUniverse() {
		clear();
		destroy(impl_, allocator());
	}
	
	void EditorUniverse::register_object_root(ObjectPtrRootBase* root) {
		impl_->external_object_roots_.push_back(root);
	}
	
	void EditorUniverse::unregister_object_root(void* root) {
		for (auto it = impl_->external_object_roots_.begin(); it != impl_->external_object_roots_.end();) {
			if ((*it)->ptr() == root) {
				destroy(*it, default_allocator());
				it = impl_->external_object_roots_.erase(it);
				return;
			} else {
				++it;
			}
		}
	}
	
	ObjectPtr<> EditorUniverse::create_object(const StructuredType *type, StringRef obj_id) {
		byte* memory = (byte*)allocator().allocate(type->size(), type->alignment());
		type->construct(memory, *this);
		ObjectPtr<> ptr((Object*)memory);
		rename_object(ptr, obj_id);
		return ptr;
	}
	
	ObjectPtr<> EditorUniverse::create_object_and_set_as_root(const StructuredType *type, StringRef obj_id) {
		ObjectPtr<> ptr = create_object(type, obj_id);
		set_root(ptr);
		return ptr;
	}
	
	ObjectPtr<> EditorUniverse::get_object(StringRef obj_id) const {
		return find_or(impl_->object_map_, obj_id, nullptr);
	}
	
	StringRef EditorUniverse::get_id(ObjectPtr<const Object> object) const {
		auto it = impl_->reverse_object_map_.find(object);
		if (it != impl_->reverse_object_map_.end()) {
			return it->second;
		}
		return "<UNNAMED OBJECT>";
	}
	
	ObjectPtr<> EditorUniverse::root() const {
		return impl_->root_;
	}
	
	void EditorUniverse::set_root(const ObjectPtr<> r) {
		if (r) {
			ASSERT(r->universe() == this);
		}
		impl_->root_ = r;
	}
	
	void EditorUniverse::clear() {
		impl_->root_ = nullptr;
		for (auto ptr: impl_->object_map_.values()) {
			if (!ptr->is_aspect_in_composite()) {
				auto type = ptr->object_type();
				auto raw_ptr = ptr.get();
				type->destruct((byte*)raw_ptr, *this);
				allocator().free(raw_ptr, type->size());
			}
		}
		impl_->object_map_.clear();
		impl_->reverse_object_map_.clear();
		impl_->object_name_pool_.clear();
	}
	
	bool EditorUniverse::rename_object(ObjectPtr<> obj, StringRef new_id) {
		auto found = impl_->object_map_.find(new_id);
		
		if (found != impl_->object_map_.end() && found->second == obj) {
			// Given the same name as it already had.
			return true;
		}
		
		bool actually_got_the_requested_name = true;
		
		StringRef pooled_new_object_id;
		if (found != impl_->object_map_.end() || new_id.size() < 2) {
			// Calculate a new name.
			actually_got_the_requested_name = false;
			
			StringRef base_name;
			int seq = 1;
			if (new_id.size() >= 2) {
				Maybe<int> parsed = parse<int>(substr(new_id, -2));
				parsed.map([&](int n) {
					base_name = substr(new_id, 0, -2);
					seq = n;
				}).otherwise([&]() {
					base_name = new_id;
				});
			} else {
				base_name = obj->object_type()->name();
			}
			
			// increment n and try the name until we find one that's available
			ScratchAllocator scratch;
			String new_name;
			do {
				StringStream create_new_name(scratch);
				create_new_name << base_name << format("%02d", seq);
				new_name = create_new_name.string(scratch);
				create_new_name.set_string("");
				++seq;
			} while (impl_->object_map_.find(new_name) != impl_->object_map_.end());
			
			auto inserted = impl_->object_name_pool_.insert(String(new_name, allocator()));
			pooled_new_object_id = StringRef(*inserted);
		} else {
			auto inserted = impl_->object_name_pool_.insert(String(new_id, allocator()));
			pooled_new_object_id = StringRef(*inserted);
		}
		
		impl_->unregister_object(obj);
		
		// Register object under new name
		impl_->object_map_[pooled_new_object_id] = obj;
		impl_->reverse_object_map_[obj] = pooled_new_object_id;
		return actually_got_the_requested_name;
	}
	
	bool EditorUniverse::recreate_object_and_initialize(const ArchiveNode& object_definition_in, StringRef old_object_id) {
		ScratchAllocator scratch;
		BinaryArchive merged(scratch);
		merge_object_templates(merged, object_definition_in);
		auto& object_definition = merged.root();
		
		const StructuredType* new_type = get_or_create_object_type(object_definition, this);
		if (new_type == nullptr) {
			Error() << "Could not recreate object; no object type.";
			return false;
		}
		
		StringRef new_id;
		if (!(object_definition["id"] >> new_id)) {
			new_id = old_object_id;
		}
		
		ObjectPtr<> obj = get_object(old_object_id);
		const StructuredType* old_type = obj->object_type();
		
		ObjectPtr<> new_obj;
		bool rewire = false;
		if (new_type != old_type) {
			impl_->unregister_object(obj);
			new_obj = create_object(new_type, new_id);
			rewire = true;
		} else {
			new_obj = obj;
			if (new_id != old_object_id) {
				rename_object(new_obj, new_id);
			}
		}
		
		new_type->deserialize_raw((byte*)new_obj.get(), object_definition, *this);
		
		for (auto& p: deferred_) {
			p.perform(*this);
		}
		deferred_.clear();
		
		if (rewire) {
			impl_->for_each_object_reference(obj, [&](ObjectPtr<>* root) {
				if (*root == obj) {
					*root = new_obj;
				}
			});
			old_type->destruct((byte*)obj.get(), *this);
			allocator().free(obj.get(), old_type->size());
		}
		
		new_obj->initialize();
		
		return true;
	}
	
	void EditorUniverse::run_initializers() {
		for (auto pair: impl_->object_map_) {
			pair.second->initialize();
		}
	}
	
	bool EditorUniverse::serialize_scene(ArchiveNode &out_scene, String &out_error) {
		Error() << "EditorUniverse cannot serialize scenes -- save the scene from the editor instead.";
		return false;
	}
	
	void EditorUniverse::Impl::unregister_object(ObjectPtr<> obj) {
		auto rit = reverse_object_map_.find(obj);
		if (rit != reverse_object_map_.end()) {
			auto it = object_map_.find(rit->second);
			auto nit = object_name_pool_.find(rit->second);
			reverse_object_map_.erase(rit);
			object_map_.erase(it);
			object_name_pool_.erase(nit);
		}
	}
	
	void EditorUniverse::Impl::unregister_object_name(StringRef object_name) {
		auto it = object_map_.find(object_name);
		if (it != object_map_.end()) {
			auto rit = reverse_object_map_.find(it->second);
			auto nit = object_name_pool_.find(object_name);
			reverse_object_map_.erase(rit);
			object_map_.erase(it);
			object_name_pool_.erase(nit);
		}
	}
	
	template <typename F>
	void EditorUniverse::Impl::for_each_object_reference(ObjectPtr<> exclude_object, F f) {
		// External roots
		for (auto root: external_object_roots_) {
			ObjectPtr<> ptr = root->get();
			f(&ptr);
			root->set(ptr);
		}
		
		// Traverse scene objects
		for (auto pair: object_map_) {
			if (pair->second == exclude_object)
				continue;
			
			auto type = pair->second->object_type();
			for (auto attr: type->attributes()) {
				auto ref_type = dynamic_cast<const ReferenceType*>(attr->type());
				if (ref_type != nullptr) {
					// We cannot provide a direct pointer, because attributes
					// may be implemented in terms of get/set methods rather than
					// a direct member, so we just store it in a temporary here.
					ObjectPtr<> tmp_root = ref_type->get_attribute_as_plain(attr, pair->second.get());
					ObjectPtr<> changed_root = tmp_root;
					
					f(&changed_root);
					
					if (changed_root != tmp_root) {
						ref_type->set_attribute_as_plain(attr, pair->second.get(), changed_root);
					}
				}
			}
		}
	}
	
	void register_object_root_impl(ObjectPtrRootBase* root, IUniverse& universe) {
		auto eu = dynamic_cast<EditorUniverse*>(&universe);
		if (eu != nullptr) {
			eu->register_object_root(root);
		}
	}
	
	void unregister_object_root_impl(void* root, IUniverse& universe) {
		auto eu = dynamic_cast<EditorUniverse*>(&universe);
		if (eu != nullptr) {
			eu->unregister_object_root(root);
		}
	}
}
