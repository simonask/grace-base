#pragma once
#ifndef REFERENCE_TYPE_HPP_EAHSMBCU
#define REFERENCE_TYPE_HPP_EAHSMBCU

#include "type/type.hpp"
#include "type/attribute.hpp"
#include "object/universe.hpp"
#include "serialization/archive_node.hpp"

namespace falling {

struct ReferenceType : Type {
	ReferenceType(String name) : name_(std::move(name)) {}
	
	virtual const Type* pointee_type() const = 0;
	
	StringRef name() const override { return name_; }
	bool deferred_instantiation() const final { return true; }
	
	// Used by EditorUniverse:
	virtual ObjectPtr<> get_attribute_as_plain(const IAttribute* attr, Object* owner) const = 0;
	virtual void set_attribute_as_plain(const IAttribute* attr, Object* owner, ObjectPtr<> new_value) const = 0;
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
	ObjectPtr<> get_attribute_as_plain(const IAttribute* attr, Object* owner) const;
	void set_attribute_as_plain(const IAttribute* attr, Object* owner, ObjectPtr<> new_value) const;
	
	// Type interface
	void deserialize(T& ptr, const ArchiveNode& node, IUniverse&) const;
	void serialize(const T& ptr, ArchiveNode& node, IUniverse&) const;
};

template <typename T>
void ReferenceTypeImpl<T>::deserialize(T& ptr, const ArchiveNode& node, IUniverse& universe) const {
	if (node.is_scalar()) {
		StringRef object_name;
		if (node >> object_name) {
			ptr = aspect_cast<typename T::PointeeType>(universe.get_object(object_name));
		}
	}
}

template <typename T>
void ReferenceTypeImpl<T>::serialize(const T& ptr, ArchiveNode& node, IUniverse& universe) const {
	if (ptr) {
		node << ptr->object_id();
	} else {
		node.clear();
	}
}

template <typename T>
ObjectPtr<> ReferenceTypeImpl<T>::get_attribute_as_plain(const IAttribute *attr, Object *owner) const {
	auto rattr = dynamic_cast<const AttributeOfType<T>*>(attr);
	if (rattr != nullptr) {
		T value;
		rattr->get_polymorphic(owner, value);
		return value;
	}
	// TODO: Warn?
	return nullptr;
}

template <typename T>
void ReferenceTypeImpl<T>::set_attribute_as_plain(const IAttribute *attr, Object *owner, ObjectPtr<> new_value) const {
	auto rattr = dynamic_cast<const AttributeOfType<T>*>(attr);
	if (rattr != nullptr) {
		T value = aspect_cast<PointeeType>(new_value);
		rattr->set_polymorphic(owner, value);
		return;
	}
	// TODO: Warn?
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
