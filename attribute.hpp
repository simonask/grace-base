#pragma once
#ifndef ATTRIBUTE_HPP_FFQKLYB6
#define ATTRIBUTE_HPP_FFQKLYB6

#include "object/object.hpp"
#include "type/type.hpp"
#include "serialization/archive.hpp"

namespace falling {

struct AttributeBase {
	AttributeBase(std::string name, std::string description) : name_(std::move(name)), description_(std::move(description)) {}
	virtual ~AttributeBase() {}
	
	virtual const Type* type() const = 0;
	const std::string& name() const { return name_; }
	const std::string& description() const { return description_; }
protected:
	std::string name_;
	std::string description_;
};

template <typename T>
struct Attribute {
	virtual ~Attribute() {}
	virtual bool get_polymorphic(Object* object, T& out_value) const = 0;
	virtual bool const_get_polymorphic(const Object* object, T const*& out_pointer) const = 0;
	virtual bool set_polymorphic(Object* object, const T& in_value) const = 0;
};

template <typename T>
struct AttributeForObject : AttributeBase {
	AttributeForObject(std::string name, std::string description) : AttributeBase(std::move(name), std::move(description)) {}
	virtual ~AttributeForObject() {}
	virtual bool deserialize_attribute(T* object, const ArchiveNode&, IUniverse&) const = 0;
	virtual bool serialize_attribute(const T* object, ArchiveNode&, IUniverse&) const = 0;
};

template <typename ObjectType, typename MemberType, typename GetterType = MemberType>
struct AttributeForObjectOfType : AttributeForObject<ObjectType>, Attribute<MemberType> {
	AttributeForObjectOfType(std::string name, std::string description) : AttributeForObject<ObjectType>(name, description) {}
	
	virtual GetterType get(const ObjectType&) const = 0;
	virtual void set(ObjectType&, MemberType value) const = 0;
	
	const Type* type() const { return get_type<MemberType>(); }
	
	bool deserialize_attribute(ObjectType* object, const ArchiveNode& node, IUniverse& universe) const {
		MemberType value;
		this->type()->deserialize(reinterpret_cast<byte*>(&value), node, universe);
		set(*object, std::move(value));
		return true; // eh...
	}
	
	bool serialize_attribute(const ObjectType* object, ArchiveNode& node, IUniverse& universe) const {
		GetterType value = get(*object);
		this->type()->serialize(reinterpret_cast<const byte*>(&value), node, universe);
		return true; // eh...
	}
	
	bool get_polymorphic(Object* object, MemberType& out_value) const {
		const ObjectType* o = dynamic_cast<const ObjectType*>(object);
		if (o != nullptr) {
			out_value = get(*o);
			return true;
		}
		return false;
	}
	
	bool set_polymorphic(Object* object, const MemberType& in_value) const {
		ObjectType* o = dynamic_cast<ObjectType*>(object);
		if (o != nullptr) {
			set(*o, in_value);
			return true;
		}
		return false;
	}
	
	bool const_get_polymorphic(const Object* object, const MemberType*& out_pointer) const {
		const ObjectType* o = dynamic_cast<const ObjectType*>(object);
		if (o != nullptr) {
			out_pointer = &get(*o);
			return true;
		}
		return false;
	}
};

template <typename ObjectType, typename MemberType>
struct MemberAttribute : AttributeForObjectOfType<ObjectType, MemberType, const MemberType&> {
	typedef MemberType ObjectType::* MemberPointer;
	
	MemberAttribute(std::string name, std::string description, MemberPointer member) : AttributeForObjectOfType<ObjectType, MemberType, const MemberType&>(name, description), member_(member) {}
	
	const MemberType& get(const ObjectType& object) const {
		return object.*member_;
	}
	
	void set(ObjectType& object, MemberType value) const {
		object.*member_ = std::move(value);
	}
	
	// override deserialize_attribute so we can deserialize in-place
	bool deserialize_attribute(ObjectType* object, const ArchiveNode& node, IUniverse& universe) const {
		MemberType* ptr = &(object->*member_);
		this->type()->deserialize(reinterpret_cast<byte*>(ptr), node, universe);
		return true; // eh...
	}
	
	MemberPointer member_;
};

template <typename ObjectType,
          typename MemberType,
          typename GetterReturnType,
          typename SetterArgumentType,
          typename SetterReturnTypeUnused>
struct MethodAttribute : AttributeForObjectOfType<ObjectType, MemberType, GetterReturnType> {
	typedef GetterReturnType(ObjectType::*GetterPointer)() const;
	typedef SetterReturnTypeUnused(ObjectType::*SetterPointer)(SetterArgumentType);
	
	MethodAttribute(std::string name, std::string description, GetterPointer getter, SetterPointer setter) : AttributeForObjectOfType<ObjectType, MemberType, GetterReturnType>(name, description), getter_(getter), setter_(setter) {}
	
	GetterReturnType get(const ObjectType& object) const {
		return (object.*getter_)();
	}
	
 	void set(ObjectType& object, MemberType value) const {
		(object.*setter_)(std::move(value));
	}
	
	GetterPointer getter_;
	SetterPointer setter_;
};

}

#endif /* end of include guard: ATTRIBUTE_HPP_FFQKLYB6 */
