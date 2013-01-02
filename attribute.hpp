#pragma once
#ifndef ATTRIBUTE_HPP_FFQKLYB6
#define ATTRIBUTE_HPP_FFQKLYB6

#include "object/object.hpp"
#include "type/type.hpp"
#include "serialization/archive.hpp"
#include "base/any.hpp"

namespace falling {
	struct IAttribute {
		virtual const Type* type() const = 0;
		virtual StringRef name() const = 0;
		virtual StringRef description() const = 0;
		virtual Any get_any(Object* object) const = 0;
		virtual Any get_any(const Object* object) const = 0;
		virtual bool set_any(Object* object, const Any& value) const = 0;
		virtual void deserialize_attribute(Object* object, const ArchiveNode&, UniverseBase&) const = 0;
		virtual void serialize_attribute(const Object* object, ArchiveNode&, UniverseBase&) const = 0;
	};

	template <typename T>
	struct AttributeOfType : IAttribute {
		virtual bool get_polymorphic(Object* object, T& out_value) const = 0;
		virtual bool get_polymorphic(const Object* object, T& out_value) const = 0;
		virtual bool set_polymorphic(Object* object, const T& in_value) const = 0;
		
		const Type* type() const final { return get_type<T>(); }
	};
	
	namespace detail {
		void warn_set_any_wrong_type(StringRef property_name, const Type* expected, const Type* got);
	}

	template <typename ObjectType, typename MemberType, typename GetterType = MemberType>
	struct AttributeForObjectOfType : AttributeOfType<MemberType> {
		String name_;
		String description_;
	
		AttributeForObjectOfType(IAllocator& alloc, StringRef name, StringRef description) : name_(name, alloc), description_(description, alloc) {}
		
		StringRef name() const { return name_; }
		StringRef description() const { return description_; }
	
		virtual GetterType get(const ObjectType&) const = 0;
		virtual void set(ObjectType&, MemberType value) const = 0;
	
		void deserialize_attribute(Object* object, const ArchiveNode& node, UniverseBase& universe) const {
			ObjectType* o = dynamic_cast<ObjectType*>(object);
			ASSERT(o != nullptr);
			if (!node.is_empty()) {
				MemberType value;
				this->type()->deserialize_raw(reinterpret_cast<byte*>(&value), node, universe);
				set(*o, std::move(value));
			}
		}
	
		void serialize_attribute(const Object* object, ArchiveNode& node, UniverseBase& universe) const {
			const ObjectType* o = dynamic_cast<const ObjectType*>(object);
			GetterType value = get(*o);
			this->type()->serialize_raw(reinterpret_cast<const byte*>(&value), node, universe);
		}
	
		Any get_any(Object* object) const {
			MemberType value;
			if (get_polymorphic(object, value)) {
				return move(value);
			}
			return Nothing;
		}
	
		Any get_any(const Object* object) const {
			MemberType value;
			if (get_polymorphic(object, value)) {
				return move(value);
			}
			return Nothing;
		}
	
		bool set_any(Object* object, const Any& value) const {
			if (value.is_a<MemberType>()) {
				bool result = false;
				value.get<MemberType>().map([&](const MemberType& v) {
					set_polymorphic(object, v);
					result = true;
				});
				return result;
			}
			detail::warn_set_any_wrong_type(name_, get_type<MemberType>(), value.type());
			return false;
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
	
		bool get_polymorphic(const Object* object, MemberType& out_value) const {
			const ObjectType* o = dynamic_cast<const ObjectType*>(object);
			if (o != nullptr) {
				out_value = get(*o);
				return true;
			}
			return false;
		}
	};

template <typename ObjectType, typename MemberType>
struct MemberAttribute : AttributeForObjectOfType<ObjectType, MemberType, const MemberType&> {
	typedef MemberType ObjectType::* MemberPointer;
	
	MemberAttribute(IAllocator& alloc, StringRef name, StringRef description, MemberPointer member) : AttributeForObjectOfType<ObjectType, MemberType, const MemberType&>(alloc, name, description), member_(member) {}
	
	const MemberType& get(const ObjectType& object) const {
		return object.*member_;
	}
	
	void set(ObjectType& object, MemberType value) const {
		object.*member_ = std::move(value);
	}
	
	// override deserialize_attribute so we can deserialize in-place
	void deserialize_attribute(Object* object, const ArchiveNode& node, UniverseBase& universe) const {
		ObjectType* o = dynamic_cast<ObjectType*>(object);
		ASSERT(o != nullptr);
		MemberType* ptr = &(o->*member_);
		this->type()->deserialize_raw(reinterpret_cast<byte*>(ptr), node, universe);
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
	
	MethodAttribute(IAllocator& alloc, StringRef name, StringRef description, GetterPointer getter, SetterPointer setter) : AttributeForObjectOfType<ObjectType, MemberType, GetterReturnType>(alloc, name, description), getter_(getter), setter_(setter) {}
	
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
