#pragma once
#ifndef OBJECT_HPP_P40DARL9
#define OBJECT_HPP_P40DARL9

#include "base/basic.hpp"
#include "base/time.hpp"
#include "base/string.hpp"

namespace grace {

struct IUniverse;
struct Type;
struct StructuredType;
struct ObjectTypeBase;
template <typename T> struct ObjectType;

#define REFLECT \
	public: \
		static const bool has_reflection__ = true; \
		using TypeInfoType = grace::ObjectTypeBase; \
		static const TypeInfoType* build_type_info__()

template <typename T> const IType* build_type_info(); // Only used for non-reflected types.

class Renderer;

struct Object {
	REFLECT;
	
	Object() : type_(nullptr), offset_(0), universe_(nullptr) {}
	Object(const Object&) = delete;
	virtual ~Object() {}
	
	virtual void initialize() {} // Called after all other objects have been instantiated and deserialized.
	virtual void update(GameTimeDelta delta) {}
	virtual void debug_render(Renderer&) {}
	
	Object* find_parent();
	const Object* find_parent() const;
	Object* find_topmost_object();
	const Object* find_topmost_object() const;
	
	bool is_aspect_in_composite() const { return offset_ != 0; }
	
	IUniverse* universe() const { return universe_; }
	void set_universe__(IUniverse* universe) { universe_ = universe; }
	uint32 universe_data__() const { return universe_data_; }
	void set_universe_data__(uint32 data) { universe_data_ = data; }
	
	StringRef object_id() const;
	bool set_object_id(StringRef new_id);
	
	const StructuredType* object_type() const { return type_; }
	void set_object_type__(const StructuredType* t) { type_ = t; }
	size_t object_offset() const { return offset_; }
	void set_object_offset__(uint32 o) { offset_ = o; }

	void enable_updates();
	void disable_updates();
private:
	StringRef object_type_name() const;

	const StructuredType* type_;
	IUniverse* universe_;
	uint32 offset_; // offset within composite
	uint32 universe_data_;
};

template <typename T>
struct IsDerivedFromObject {
	static const bool Value = std::is_convertible<typename std::remove_const<T>::type*, Object*>::value;
};

inline const StructuredType* get_type(Object* object) {
	return object->object_type();
}

inline const StructuredType* get_type(const Object* object) {
	return object->object_type();
}

}

#endif /* end of include guard: OBJECT_HPP_P40DARL9 */
