#include "type/type_registry.hpp"
#include "object/object_type.hpp"
#include "base/basic.hpp"
#include "base/map.hpp"

namespace falling {

struct TypeRegistry::Impl {
	Array<const ObjectTypeBase*> types;
	Map<String, size_t> type_map;
};

TypeRegistry::Impl* TypeRegistry::impl() {
	static Impl* i = new_static Impl;
	return i;
}

void TypeRegistry::add(const ObjectTypeBase* type) {
	impl()->type_map[type->name()] = impl()->types.size();
	impl()->types.push_back(type);
}

const ObjectTypeBase* TypeRegistry::get(StringRef name) {
	size_t idx = find_or(impl()->type_map, name, SIZE_T_MAX);
	if (idx >= impl()->types.size()) {
		return nullptr;
	} else {
		return impl()->types[idx];
	}
}

ArrayRef<const ObjectTypeBase*> TypeRegistry::object_types() {
	return impl()->types;
}

void TypeRegistry::add_missing_types() {
	ScratchAllocator scratch;
	Array<const ObjectTypeBase*> registered_types = impl()->types;
	for (auto type: registered_types) {
		const ObjectTypeBase* s = type;
		while (s != get_type<Object>()) {
			if (impl()->type_map.find(s->name()) == impl()->type_map.end()) {
				add(s);
			}
			s = s->super();
		}
	}
	
	if (impl()->type_map.find("Object") == impl()->type_map.end()) {
		add(get_type<Object>());
	}
}

}