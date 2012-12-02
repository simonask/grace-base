#pragma once
#ifndef TYPE_HPP_ZXLGBWRF
#define TYPE_HPP_ZXLGBWRF

#include "base/basic.hpp"
#include "base/array.hpp"
#include "base/vector.hpp"
#include "memory/static_allocator.hpp"
#include "base/string.hpp"
#include <map>
#include "io/string_stream.hpp"
#include <algorithm>
#include <limits.h>


namespace falling {
	
struct ArchiveNode;
struct UniverseBase;
struct SlotBase;

struct Type {
	virtual void deserialize_raw(byte* place, const ArchiveNode&, UniverseBase&) const = 0;
	virtual void serialize_raw(const byte* place, ArchiveNode&, UniverseBase&) const = 0;
	virtual void construct(byte* place, UniverseBase&) const = 0;
	virtual void copy_construct(byte* to, const byte* from) const = 0;
	virtual void move_construct(byte* to, byte* from) const = 0;
	virtual void destruct(byte* place, UniverseBase&) const = 0;
	
	virtual String name() const = 0;
	virtual size_t size() const = 0;
	virtual bool is_abstract() const { return false; }
	virtual bool is_copy_constructible() const { return true; }
	virtual bool is_move_constructible() const { return true; }
protected:
	Type() {}
};

template <typename ObjectType, typename TypeType = Type>
struct TypeFor : TypeType {
	// Forwarding constructor.
	template <typename... Args>
	TypeFor(Args&&... args) : TypeType(std::forward<Args>(args)...) {}
	
	// Override interface.
	virtual void deserialize(ObjectType& place, const ArchiveNode&, UniverseBase&) const = 0;
	virtual void serialize(const ObjectType& place, ArchiveNode&, UniverseBase&) const = 0;


	// Do not override.
	void deserialize_raw(byte* place, const ArchiveNode& node, UniverseBase& universe) const {
		this->deserialize(*reinterpret_cast<ObjectType*>(place), node, universe);
	}
	void serialize_raw(const byte* place, ArchiveNode& node, UniverseBase& universe) const {
		this->serialize(*reinterpret_cast<const ObjectType*>(place), node, universe);
	}
	
	void construct(byte* place, UniverseBase&) const {
		this->construct_unless_abstract(place);
	}
	void destruct(byte* place, UniverseBase&) const {
		reinterpret_cast<ObjectType*>(place)->~ObjectType();
	}
	void copy_construct(byte* to, const byte* from) const {
		const ObjectType* original = reinterpret_cast<const ObjectType*>(from);
		this->copy_construct_unless_abstract(to, *original);
	}
	void move_construct(byte* to, byte* from) const {
		const ObjectType* original = reinterpret_cast<ObjectType*>(from);
		this->move_construct_unless_abstract(to, std::move(*original));
	}
	size_t size() const { return sizeof(ObjectType); }
	bool is_copy_constructible() const { return IsCopyConstructibleNonRef<ObjectType>::Value; }
	bool is_move_constructible() const { return IsMoveConstructibleNonRef<ObjectType>::Value; }
	
private:
	template <typename T = ObjectType, typename... Args>
	typename std::enable_if<std::is_abstract<T>::value, void>::type
	construct_unless_abstract(byte* place, Args&&...) const {
		// Abstract, do nothing.
		ASSERT(false); // Should never be called!
	}
	
	template <typename T = ObjectType, typename... Args>
	typename std::enable_if<!std::is_abstract<T>::value, void>::type
	construct_unless_abstract(byte* place, Args&&... args) const {
		::new(place) ObjectType(std::forward<Args>(args)...);
	}
	
	template <typename T = ObjectType>
	typename std::enable_if<std::is_abstract<T>::value || !IsCopyConstructibleNonRef<T>::Value, void>::type
	copy_construct_unless_abstract(byte* place, const T& original) const {
		ASSERT(false); // Type is abstract or not copy-constructible.
	}
	
	template <typename T = ObjectType>
	typename std::enable_if<!std::is_abstract<T>::value && IsCopyConstructibleNonRef<T>::Value, void>::type
	copy_construct_unless_abstract(byte* place, const T& original) const {
		::new(place) ObjectType(original);
	}
	
	template <typename T = ObjectType>
	typename std::enable_if<std::is_abstract<T>::value || !IsMoveConstructibleNonRef<T>::Value, void>::type
	move_construct_unless_abstract(byte* place, T&& original) const {
		ASSERT(false); // Type is abstract or not move-constructible.
	}
	
	template <typename T = ObjectType>
	typename std::enable_if<!std::is_abstract<T>::value && IsMoveConstructibleNonRef<T>::Value, void>::type
	move_construct_unless_abstract(byte* place, T&& original) const {
		::new(place) ObjectType(std::move(original));
	}
};

struct VoidType : Type {
	static const VoidType* get();
	
	void deserialize_raw(byte* place, const ArchiveNode&, UniverseBase&) const override {}
	void serialize_raw(const byte*, ArchiveNode&, UniverseBase&) const override {}
	virtual void construct(byte*, UniverseBase&) const override {}
	virtual void destruct(byte*, UniverseBase&) const override {}
	virtual void copy_construct(byte*, const byte*) const override {}
	virtual void move_construct(byte*, byte*) const override {}
	static const char Name[];
	String name() const override { return Name; }
	size_t size() const override { return 0; }
	bool is_abstract() const override { return true; }
private:
	VoidType() {}
};

struct SimpleType : Type {
	SimpleType(String name, size_t width, size_t component_width, bool is_float, bool is_signed) : name_(std::move(name)), width_(width), component_width_(component_width), is_float_(is_float), is_signed_(is_signed) {}
	String name() const override { return name_; }
	void construct(byte* place, UniverseBase&) const { std::fill(place, place + size(), 0); }
	void destruct(byte*, UniverseBase&) const {}
	void copy_construct(byte* place, const byte* from) const { std::copy(from, from + size(), place); }
	void move_construct(byte* place, byte* from) const { copy_construct(place, from); }
	
	size_t size() const override { return width_; }
	size_t num_components() const { return width_ / component_width_; }
	bool is_signed() const { return is_signed_; }
	virtual void* cast(const SimpleType* to, void* o) const = 0;
protected:
	String name_;
	size_t width_;
	size_t component_width_;
	bool is_float_;
	bool is_signed_;
};

struct EnumType : SimpleType {
	EnumType(String name, size_t width, bool is_signed = true) : SimpleType(name, width, width, false, is_signed), max_(1LL-SSIZE_MAX), min_(SSIZE_MAX) {}
	void add_entry(String name, ssize_t value, String description) {
		entries_.emplace_back(std::make_tuple(std::move(name), value, std::move(description)));
	}
	bool contains(ssize_t value) const;
	ssize_t max() const { return max_; }
	ssize_t min() const { return min_; }
	bool name_for_value(ssize_t value, String& out_name) const;
	bool value_for_name(const String& name, ssize_t& out_value) const;
	
	void deserialize_raw(byte*, const ArchiveNode&, UniverseBase&) const override;
	void serialize_raw(const byte*, ArchiveNode&, UniverseBase&) const override;
	void* cast(const SimpleType* to, void* o) const;
private:
	Array<std::tuple<String, ssize_t, String>> entries_;
	ssize_t max_;
	ssize_t min_;
};

struct IntegerType : SimpleType {
	IntegerType(String name, size_t width, bool is_signed = true) : SimpleType(name, width, width, false, is_signed) {}
	void deserialize_raw(byte*, const ArchiveNode&, UniverseBase&) const override;
	void serialize_raw(const byte*, ArchiveNode&, UniverseBase&) const override;
	void* cast(const SimpleType* to, void* o) const;
	size_t max() const;
	ssize_t min() const;
};

struct FloatType : SimpleType {
	FloatType(String name, size_t width) : SimpleType(name, width, width, true, true) {}
	void deserialize_raw(byte*, const ArchiveNode& node, UniverseBase&) const override;
	void serialize_raw(const byte*, ArchiveNode&, UniverseBase&) const override;
	void* cast(const SimpleType* to, void* o) const;
};

struct StringType : TypeFor<String> {
	static const StringType* get();
	
	void deserialize(String& place, const ArchiveNode&, UniverseBase&) const override;
	void serialize(const String& place, ArchiveNode&, UniverseBase&) const override;
	
	String name() const override;
	size_t size() const override { return sizeof(String); }
};

struct DerivedType : Type {
	virtual const SlotBase* find_slot_by_name(const String& name) const { return nullptr; }
};

// static downcast
template <typename To, typename From>
typename std::enable_if<std::is_convertible<From*, To*>::value, To*>::type
aspect_cast(From* ptr) {
	return ptr;
}

// dynamic upcast
template <typename To, typename From>
typename std::enable_if<
	!std::is_same<To, From>::value
	&& (!HasReflection<To>::Value || !HasReflection<From>::Value)
	&& std::is_convertible<typename std::remove_const<To>::type*, typename std::remove_const<From>::type*>::value
	, To*>::type
aspect_cast(From* ptr) {
	return dynamic_cast<To*>(ptr);
}

template <typename T> struct BuildTypeInfo {};

#define DECLARE_TYPE(T) template<> struct BuildTypeInfo<T> { static const SimpleType* build(); };
DECLARE_TYPE(int8)
DECLARE_TYPE(int16)
DECLARE_TYPE(int32)
DECLARE_TYPE(int64)
DECLARE_TYPE(uint8)
DECLARE_TYPE(uint16)
DECLARE_TYPE(uint32)
DECLARE_TYPE(uint64)
DECLARE_TYPE(float32)
DECLARE_TYPE(float64)

template <> struct BuildTypeInfo<void> {
	static const VoidType* build() { return VoidType::get(); }
};

template <> struct BuildTypeInfo<String> {
	static const StringType* build() { return StringType::get(); }
};

template <typename T>
auto build_type_info()
-> decltype(BuildTypeInfo<T>::build())
{
	return BuildTypeInfo<T>::build();
}

template <typename T>
typename std::enable_if<HasReflection<T>::Value, const typename T::TypeInfoType*>::type
get_type() {
	return T::build_type_info__();
}
	
template <typename T>
typename std::enable_if<!HasReflection<T>::Value, decltype(BuildTypeInfo<T>::build())>::type
get_type() {
	return BuildTypeInfo<T>::build();
}
	
template <typename T>
typename std::enable_if<HasReflection<T>::Value, const DerivedType*>::type
get_type(const T& object) {
	return object.object_type();
}
	
template <typename T>
typename std::enable_if<!HasReflection<T>::Value, const Type*>::type
get_type(const T& value) {
	return get_type<T>();
}

template <typename Last = void>
void append_type_names(FormattedStream& os) {
	const Type* t = get_type<Last>();
	os << t->name();
}

template <typename Head, typename Next, typename... Rest>
void append_type_names(FormattedStream& os) {
	const Type* t = get_type<Head>();
	os << t->name();
	os << ", ";
	append_type_names<Next, Rest...>(os);
}

template <typename... Args>
String get_signature_description() {
	StringStream ss;
	ss << '(';
	append_type_names<Args...>(ss);
	ss << ')';
	return ss.str();
}


template <typename Head = void>
void build_signature(Array<const Type*>& signature) {
	signature.push_back(get_type<Head>());
}
template <typename Head, typename Next, typename... Rest>
void build_signature(Array<const Type*>& signature) {
	signature.push_back(get_type<Head>());
	build_signature<Next, Rest...>(signature);
}

}

#endif /* end of include guard: TYPE_HPP_ZXLGBWRF */
