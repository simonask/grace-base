#pragma once
#ifndef TYPE_REGISTRY_HPP_LPQGF8DT
#define TYPE_REGISTRY_HPP_LPQGF8DT

#include "object/object.hpp"
#include "type/type.hpp"
#include "object/objectptr.hpp"
#include "base/string.hpp"

namespace falling {

	struct VertexType;

class TypeRegistry {
public:
	template <typename T>
	static void add();
	static void add(const ObjectTypeBase* type);
	static void add(const VertexType* type);
	static ArrayRef<const ObjectTypeBase*> object_types();
	static void add_missing_types(); // Search registered types for references to non-registered types and register them.
	
	static const ObjectTypeBase* get(StringRef name);
	static const VertexType* get_vertex_type(StringRef name);
	
	static void clear();
private:
	TypeRegistry();
	struct Impl;
	static Impl* impl();
};

template <typename T>
void TypeRegistry::add() {
	add(get_type<T>());
}

}

#endif /* end of include guard: TYPE_REGISTRY_HPP_LPQGF8DT */
