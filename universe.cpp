#include "object/universe.hpp"
#include "object/object_type.hpp"
#include "io/formatters.hpp"
#include "base/log.hpp"
#include "base/parse.hpp"

#include <sstream>

namespace falling {
	
	void error_category_already_initialized_with_different_type(const String& name) {
		Error() << "Object category has already been initialized with a different type: " << name;
	}

ObjectPtr<> BasicUniverse::create_root(const DerivedType* type, String id) {
	clear();
	root_ = create_object(type, std::move(id));
	return root_;
}

ObjectPtr<> BasicUniverse::create_object(const DerivedType* type, String id) {
	size_t sz = type->size();
	byte* memory = (byte*)allocator_.allocate(sz, 1);
	type->construct(memory, *this);
	Object* object = reinterpret_cast<Object*>(memory);
	memory_map_.push_back(object);
	rename_object(object, id);
	return object;
}

bool BasicUniverse::rename_object(ObjectPtr<> object, String new_id) {
	ASSERT(object->universe() == this);
	
	// erase old name from database
	auto old_it = reverse_object_map_.find(object);
	if (old_it != reverse_object_map_.end()) {
		object_map_.erase(old_it->second);
	}

	// check if new name already exists
	auto it = object_map_.find(new_id);
	bool renamed_exact = true;
	String new_name;
	if ((it != object_map_.end()) || (new_id.size() < 2)) {
		// it does, so create a unique name from the requested name
		int n = 1;
		String base_name;
		if (new_id.size() >= 2) {
			Maybe<int> parsed = parse<int>(new_id.substr(new_id.size()-2, 2));
			maybe_if(parsed, [&](int parsed_number) {
				base_name = new_id.substr(0, new_id.size()-2);
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

const String& BasicUniverse::get_id(ObjectPtr<const Object> object) const {
	auto it = reverse_object_map_.find(object);
	if (it != reverse_object_map_.end()) {
		return it->second;
	}
	return empty_id_;
}
	
void BasicUniverse::initialize_all() {
	for (auto pair: object_map_) {
		pair.second->initialize();
	}
}

void BasicUniverse::clear() {
	for (auto object: memory_map_) {
		const DerivedType* type = object->object_type();
		type->destruct(reinterpret_cast<byte*>(object), *this);
		allocator_.free(object);
	}
	// TODO: Test for references?
	object_map_.clear();
	reverse_object_map_.clear();
	memory_map_.clear();
}

}