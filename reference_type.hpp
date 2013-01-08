#pragma once
#ifndef REFERENCE_TYPE_HPP_EAHSMBCU
#define REFERENCE_TYPE_HPP_EAHSMBCU

#include "type/type.hpp"
#include "object/universe.hpp"
#include "serialization/archive_node.hpp"

namespace falling {

struct ReferenceType : Type {
	ReferenceType(String name) : name_(std::move(name)) {}
	
	virtual const Type* pointee_type() const = 0;
	
	String name() const override { return name_; }
	bool deferred_instantiation() const final { return true; }
protected:
	static String build_reference_type_name(String base_name, const Type* pointee);
private:
	String name_;
};

template <typename T>
struct ReferenceTypeImpl : TypeFor<T, ReferenceType> {
	typedef typename T::PointeeType PointeeType;
	
	ReferenceTypeImpl(String base_name) : TypeFor<T, ReferenceType>(ReferenceType::build_reference_type_name(base_name, get_type<PointeeType>())) {}
	
	// ReferenceType interface
	const Type* pointee_type() const { return get_type<PointeeType>(); }
	
	// Type interface
	void deserialize(T& ptr, const ArchiveNode& node, IUniverse&) const;
	void serialize(const T& ptr, ArchiveNode& node, IUniverse&) const;
};

template <typename T>
void ReferenceTypeImpl<T>::deserialize(T& ptr, const ArchiveNode& node, IUniverse& universe) const {
	ptr = aspect_cast<typename T::PointeeType>(universe.get_object(node.string_value));
}

template <typename T>
void ReferenceTypeImpl<T>::serialize(const T& ptr, ArchiveNode& node, IUniverse& universe) const {
	if (ptr) {
		node = ptr->object_id();
	} else {
		node.clear();
	}
}

	template <typename T>
	struct BuildTypeInfo<ObjectPtr<T>> {
		static const ReferenceTypeImpl<ObjectPtr<T>>* build() {
			static auto type = new_static ReferenceTypeImpl<ObjectPtr<T>>("ObjectPtr");
			return type;
		}
	};

}

#endif /* end of include guard: REFERENCE_TYPE_HPP_EAHSMBCU */
