#pragma once
#ifndef TYPE_REGISTRY_HPP_LPQGF8DT
#define TYPE_REGISTRY_HPP_LPQGF8DT

#include "object/object.hpp"
#include "type/type.hpp"
#include "object/objectptr.hpp"
#include "base/string.hpp"

namespace falling {

class TypeRegistry {
public:
	template <typename T>
	static void add();
	static void add(const ObjectTypeBase* type);
	static ArrayRef<const ObjectTypeBase*> object_types();
	
	static const ObjectTypeBase* get(StringRef name);
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
