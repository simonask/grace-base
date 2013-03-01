#include "type/type_registry.hpp"
#include "object/object_type.hpp"
#include "base/basic.hpp"
#include "base/map.hpp"
#include "render/vertex_type.hpp"

namespace falling {

struct TypeRegistry::Impl {
	Array<const ObjectTypeBase*> types;
	Map<String, size_t> type_map;
	Array<const VertexType*> vertex_types;
	Map<String, size_t> vertex_type_map;
};

TypeRegistry::Impl* TypeRegistry::impl() {
	static Impl* i = new_static Impl;
	return i;
}

void TypeRegistry::add(const ObjectTypeBase* type) {
	impl()->type_map[type->name()] = impl()->types.size();
	impl()->types.push_back(type);
}

void TypeRegistry::add(const VertexType* type) {
	impl()->vertex_type_map[type->name()] = impl()->vertex_types.size();
	impl()->vertex_types.push_back(type);
}

const ObjectTypeBase* TypeRegistry::get(StringRef name) {
	size_t idx = find_or(impl()->type_map, name, SIZE_T_MAX);
	if (idx >= impl()->types.size()) {
		return nullptr;
	} else {
		return impl()->types[idx];
	}
}

const VertexType* TypeRegistry::get_vertex_type(StringRef name) {
	size_t idx = find_or(impl()->vertex_type_map, name, SIZE_T_MAX);
	if (idx >= impl()->vertex_types.size()) {
		return nullptr;
	} else {
		return impl()->vertex_types[idx];
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
		while (s && s != get_type<Object>()) {
			if (impl()->type_map.find(s->name()) == impl()->type_map.end()) {
				add(s);
			}
			s = dynamic_cast<const ObjectTypeBase*>(s->super());
		}
	}
	
	if (impl()->type_map.find("Object") == impl()->type_map.end()) {
		add(get_type<Object>());
	}
}

}