#pragma once
#ifndef OBJECT_HPP_P40DARL9
#define OBJECT_HPP_P40DARL9

#include "base/basic.hpp"
#include "base/time.hpp"

namespace falling {

struct UniverseBase;
struct Type;
struct DerivedType;
struct ObjectTypeBase;
template <typename T> struct ObjectType;

#define REFLECT \
	public: \
		static const bool has_reflection__ = true; \
		typedef ObjectTypeBase TypeInfoType; \
		static const TypeInfoType* build_type_info__()

template <typename T> const Type* build_type_info(); // Only used for non-reflected types.

struct Object {
	REFLECT;
	
	Object() : type_(nullptr), offset_(0), universe_(nullptr) {}
	virtual ~Object() {}
	
	virtual void initialize() {} // Called after all other objects have been instantiated and deserialized.
	
	Object* find_parent();
	const Object* find_parent() const;
	Object* find_topmost_object();
	const Object* find_topmost_object() const;
	
	UniverseBase* universe() const { return universe_; }
	void set_universe__(UniverseBase* universe) { universe_ = universe; }
	uint32 universe_data__() const { return universe_data_; }
	void set_universe_data__(uint32 data) { universe_data_ = data; }
	
	const String& object_id() const;
	bool set_object_id(String new_id);
	
	const DerivedType* object_type() const { return type_; }
	void set_object_type__(const DerivedType* t) { type_ = t; }
	size_t object_offset() const { return offset_; }
	void set_object_offset__(uint32 o) { offset_ = o; }

private:
	const DerivedType* type_;
	UniverseBase* universe_;
	uint32 offset_; // offset within composite
	uint32 universe_data_;
};

template <typename T>
struct IsDerivedFromObject {
	static const bool Value = std::is_convertible<typename std::remove_const<T>::type*, Object*>::value;
};

inline const DerivedType* get_type(Object* object) {
	return object->object_type();
}

inline const DerivedType* get_type(const Object* object) {
	return object->object_type();
}

}

#endif /* end of include guard: OBJECT_HPP_P40DARL9 */
