#pragma once
#ifndef TYPE_HPP_ZXLGBWRF
#define TYPE_HPP_ZXLGBWRF

#include "base/basic.hpp"
#include "base/array.hpp"
#include "base/vector.hpp"
#include <string>
#include <map>
#include "io/string_stream.hpp"
#include <algorithm>
#include <limits.h>


namespace falling {
	
struct ArchiveNode;
struct IUniverse;
struct SlotBase;

struct Type {
	virtual void deserialize(byte* place, const ArchiveNode&, IUniverse&) const = 0;
	virtual void serialize(const byte* place, ArchiveNode&, IUniverse&) const = 0;
	virtual void construct(byte* place, IUniverse&) const = 0;
	virtual void destruct(byte* place, IUniverse&) const = 0;
	
	virtual std::string name() const = 0;
	virtual size_t size() const = 0;
	virtual bool is_abstract() const { return false; }
protected:
	Type() {}
};

template <typename ObjectType, typename TypeType = Type>
struct TypeFor : TypeType {
	// Forwarding constructor.
	template <typename... Args>
	TypeFor(Args&&... args) : TypeType(std::forward<Args>(args)...) {}
	
	// Override interface.
	virtual void deserialize(ObjectType& place, const ArchiveNode&, IUniverse&) const = 0;
	virtual void serialize(const ObjectType& place, ArchiveNode&, IUniverse&) const = 0;


	// Do not override.
	void deserialize(byte* place, const ArchiveNode& node, IUniverse& universe) const {
		this->deserialize(*reinterpret_cast<ObjectType*>(place), node, universe);
	}
	void serialize(const byte* place, ArchiveNode& node, IUniverse& universe) const {
		this->serialize(*reinterpret_cast<const ObjectType*>(place), node, universe);
	}
	void construct(byte* place, IUniverse&) const {
		::new(place) ObjectType;
	}
	void destruct(byte* place, IUniverse&) const {
		reinterpret_cast<ObjectType*>(place)->~ObjectType();
	}
	size_t size() const { return sizeof(ObjectType); }
};

struct VoidType : Type {
	static const VoidType* get();
	
	void deserialize(byte* place, const ArchiveNode&, IUniverse&) const override {}
	void serialize(const byte*, ArchiveNode&, IUniverse&) const override {}
	virtual void construct(byte*, IUniverse&) const override {}
	virtual void destruct(byte*, IUniverse&) const override {}
	static const char Name[];
	std::string name() const override { return Name; }
	size_t size() const override { return 0; }
	bool is_abstract() const override { return true; }
private:
	VoidType() {}
};

struct SimpleType : Type {
	SimpleType(std::string name, size_t width, size_t component_width, bool is_float, bool is_signed) : name_(std::move(name)), width_(width), component_width_(component_width), is_float_(is_float), is_signed_(is_signed) {}
	std::string name() const override { return name_; }
	void construct(byte* place, IUniverse&) const { std::fill(place, place + size(), 0); }
	void destruct(byte*, IUniverse&) const {}
	
	size_t size() const override { return width_; }
	size_t num_components() const { return width_ / component_width_; }
	bool is_signed() const { return is_signed_; }
	virtual void* cast(const SimpleType* to, void* o) const = 0;
protected:
	std::string name_;
	size_t width_;
	size_t component_width_;
	bool is_float_;
	bool is_signed_;
};

struct EnumType : SimpleType {
	EnumType(std::string name, size_t width, bool is_signed = true) : SimpleType(name, width, width, false, is_signed), max_(1LL-SSIZE_MAX), min_(SSIZE_MAX) {}
	void add_entry(std::string name, ssize_t value, std::string description) {
		entries_.emplace_back(std::make_tuple(std::move(name), value, std::move(description)));
	}
	bool contains(ssize_t value) const;
	ssize_t max() const { return max_; }
	ssize_t min() const { return min_; }
	bool name_for_value(ssize_t value, std::string& out_name) const;
	bool value_for_name(const std::string& name, ssize_t& out_value) const;
	
	void deserialize(byte*, const ArchiveNode&, IUniverse&) const override;
	void serialize(const byte*, ArchiveNode&, IUniverse&) const override;
	void* cast(const SimpleType* to, void* o) const;
private:
	Array<std::tuple<std::string, ssize_t, std::string>> entries_;
	ssize_t max_;
	ssize_t min_;
};

struct IntegerType : SimpleType {
	IntegerType(std::string name, size_t width, bool is_signed = true) : SimpleType(name, width, width, false, is_signed) {}
	void deserialize(byte*, const ArchiveNode&, IUniverse&) const override;
	void serialize(const byte*, ArchiveNode&, IUniverse&) const override;
	void* cast(const SimpleType* to, void* o) const;
	size_t max() const;
	ssize_t min() const;
};

struct FloatType : SimpleType {
	FloatType(std::string name, size_t width) : SimpleType(name, width, width, true, true) {}
	void deserialize(byte*, const ArchiveNode& node, IUniverse&) const override;
	void serialize(const byte*, ArchiveNode&, IUniverse&) const override;
	void* cast(const SimpleType* to, void* o) const;
};

struct StringType : TypeFor<std::string> {
	static const StringType* get();
	
	void deserialize(std::string& place, const ArchiveNode&, IUniverse&) const override;
	void serialize(const std::string& place, ArchiveNode&, IUniverse&) const override;
	
	std::string name() const override;
	size_t size() const override { return sizeof(std::string); }
};

struct DerivedType : Type {
	virtual const SlotBase* find_slot_by_name(const std::string& name) const { return nullptr; }
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

template <> struct BuildTypeInfo<std::string> {
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
typename std::enable_if<!HasReflection<T>::Value, decltype(build_type_info<T>())>::type
get_type() {
	return build_type_info<T>();
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
std::string get_signature_description() {
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
