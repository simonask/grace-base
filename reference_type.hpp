#pragma once
#ifndef REFERENCE_TYPE_HPP_EAHSMBCU
#define REFERENCE_TYPE_HPP_EAHSMBCU

#include "type/type.hpp"
#include "serialization/archive_node.hpp"

namespace falling {

struct ReferenceType : Type {
	ReferenceType(String name) : name_(std::move(name)) {}
	
	virtual const Type* pointee_type() const = 0;
	
	String name() const override { return name_; }
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
	void deserialize(T& ptr, const ArchiveNode& node, UniverseBase&) const;
	void serialize(const T& ptr, ArchiveNode& node, UniverseBase&) const;
};

template <typename T>
void ReferenceTypeImpl<T>::deserialize(T& ptr, const ArchiveNode& node, UniverseBase&) const {
	node.register_reference_for_deserialization(ptr);
}

template <typename T>
void ReferenceTypeImpl<T>::serialize(const T& ptr, ArchiveNode& node, UniverseBase&) const {
	node.register_reference_for_serialization(ptr);
}

}

#endif /* end of include guard: REFERENCE_TYPE_HPP_EAHSMBCU */
