#include "type/type_registry.hpp"
#include "object/object_type.hpp"
#include "base/basic.hpp"
#include "base/map.hpp"
#include "render/vertex_type.hpp"

namespace grace {

struct TypeRegistry::Impl {
	Map<String, const ObjectTypeBase*> type_map;
	Map<String, const VertexType*> vertex_type_map;
	Map<String, const EnumType*> enum_type_map;
};

TypeRegistry::Impl* TypeRegistry::impl() {
	static Impl* i = new_static Impl;
	return i;
}

void TypeRegistry::clear() {
	Impl* i = impl();
	i->type_map.clear();
	i->vertex_type_map.clear();
	i->enum_type_map.clear();
}

void TypeRegistry::add(const ObjectTypeBase* type) {
	impl()->type_map[type->name()] = type;
}

void TypeRegistry::add(const VertexType* type) {
	impl()->vertex_type_map[type->name()] = type;
}

void TypeRegistry::add(const EnumType* type) {
	impl()->enum_type_map[type->name()] = type;
}

const ObjectTypeBase* TypeRegistry::get(StringRef name) {
	return find_or(impl()->type_map, name, nullptr);
}

const VertexType* TypeRegistry::get_vertex_type(StringRef name) {
	return find_or(impl()->vertex_type_map, name, nullptr);
}

ArrayRef<const ObjectTypeBase*> TypeRegistry::object_types() {
	return impl()->type_map.values();
}

ArrayRef<const EnumType*> TypeRegistry::enum_types() {
	return impl()->enum_type_map.values();
}

ArrayRef<const VertexType*> TypeRegistry::vertex_types() {
	return impl()->vertex_type_map.values();
}

void TypeRegistry::add_missing_types() {
	ScratchAllocator scratch;
	Array<const ObjectTypeBase*> registered_types(impl()->type_map.values(), scratch);
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