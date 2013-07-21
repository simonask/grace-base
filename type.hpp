#pragma once
#ifndef TYPE_HPP_ZXLGBWRF
#define TYPE_HPP_ZXLGBWRF

#include "base/basic.hpp"
#include "base/array.hpp"
#include "base/vector.hpp"
#include "memory/static_allocator.hpp"
#include "base/string.hpp"
#include "io/string_stream.hpp"
#include "base/type_info.hpp"
#include <algorithm>
#include <limits.h>


namespace grace {
	
struct DocumentNode;
struct IUniverse;

struct IType {
	virtual void deserialize_raw(byte* place, const DocumentNode&, IUniverse&) const = 0;
	virtual void serialize_raw(const byte* place, DocumentNode&, IUniverse&) const = 0;
	virtual void construct(byte* place, IUniverse&) const = 0;
	virtual void copy_construct(byte* to, const byte* from) const = 0;
	virtual void move_construct(byte* to, byte* from) const = 0;
	virtual void destruct(byte* place, IUniverse&) const = 0;
	
	virtual StringRef name() const = 0;
	virtual size_t size() const = 0;
	virtual size_t alignment() const = 0;
	virtual bool is_abstract() const = 0;
	virtual bool is_copy_constructible() const = 0;
	virtual bool is_move_constructible() const = 0;
	virtual bool deferred_instantiation() const = 0;
};

struct Type : public IType {
public:
	size_t size() const override { return type_info_.size; }
	size_t alignment() const override { return type_info_.alignment; }
	void construct(byte* place, IUniverse&) const override { type_info_.construct(place); }
	void destruct(byte* place, IUniverse&) const override { type_info_.destruct(place); }
	void copy_construct(byte* to, const byte* from) const override { type_info_.copy_construct(to, from); }
	void move_construct(byte* to, byte* from) const override { type_info_.move_construct(to, from); }
	bool is_abstract() const override { return type_info_.is_abstract(); }
	bool is_copy_constructible() const override { return type_info_.is_copy_constructible(); }
	bool is_move_constructible() const override { return type_info_.is_move_constructible(); }
	bool deferred_instantiation() const override { return false; }
protected:
	const TypeInfo& type_info_;
	Type(const TypeInfo& type_info) : type_info_(type_info) {}
};

template <typename ObjectType, typename TypeType = Type>
struct TypeFor : TypeType {
	static constexpr const TypeInfo& Info = GetTypeInfo<ObjectType>::Value;

	template <typename... Args>
	TypeFor(Args&&... args) : TypeType(Info, std::forward<Args>(args)...) {}
	
	// Override interface.
	virtual void deserialize(ObjectType& place, const DocumentNode&, IUniverse&) const = 0;
	virtual void serialize(const ObjectType& place, DocumentNode&, IUniverse&) const = 0;

	// Do not override.
	void deserialize_raw(byte* place, const DocumentNode& node, IUniverse& universe) const {
		this->deserialize(*reinterpret_cast<ObjectType*>(place), node, universe);
	}
	void serialize_raw(const byte* place, DocumentNode& node, IUniverse& universe) const {
		this->serialize(*reinterpret_cast<const ObjectType*>(place), node, universe);
	}
};

template <typename ObjectType, typename TypeType>
constexpr const TypeInfo& TypeFor<ObjectType,TypeType>::Info;

struct VoidType : Type {
	static const VoidType* get();
	
	void deserialize_raw(byte* place, const DocumentNode&, IUniverse&) const override {}
	void serialize_raw(const byte*, DocumentNode&, IUniverse&) const override {}
	virtual void construct(byte*, IUniverse&) const override {}
	virtual void destruct(byte*, IUniverse&) const override {}
	virtual void copy_construct(byte*, const byte*) const override {}
	virtual void move_construct(byte*, byte*) const override {}
	static const char Name[];
	StringRef name() const override { return Name; }
	size_t size() const override { return 0; }
	size_t alignment() const override { return 0; }
	bool is_abstract() const override { return true; }
private:
	VoidType() : Type(GetTypeInfo<void>::Value) {}
};

struct SimpleType : Type {
	SimpleType(IAllocator& alloc, const TypeInfo& type_info, StringRef name, size_t width, size_t component_width, bool is_float, bool is_signed) : Type(type_info), name_(name, alloc), width_(width), component_width_(component_width), is_float_(is_float), is_signed_(is_signed) {}
	StringRef name() const override { return name_; }
	void construct(byte* place, IUniverse&) const { std::fill(place, place + size(), 0); }
	void destruct(byte*, IUniverse&) const {}
	void copy_construct(byte* place, const byte* from) const { std::copy(from, from + size(), place); }
	void move_construct(byte* place, byte* from) const { copy_construct(place, from); }
	
	size_t size() const override { return width_; }
	size_t alignment() const override { return width_; }
	size_t num_components() const { return width_ / component_width_; }
	bool is_signed() const { return is_signed_; }
protected:
	String name_;
	size_t width_;
	size_t component_width_;
	bool is_float_;
	bool is_signed_;
};

struct EnumType : SimpleType {
	EnumType(const TypeInfo& type_info, IAllocator& alloc, StringRef name, size_t width, bool is_signed = true) : SimpleType(alloc, type_info, name, width, width, false, is_signed), max_(1LL-SSIZE_MAX), min_(SSIZE_MAX), entries_(alloc) {}
	bool contains(ssize_t value) const;
	bool contains(StringRef name) const;
	ssize_t max() const { return max_; }
	ssize_t min() const { return min_; }
	bool name_for_value(ssize_t value, StringRef& out_name) const;
	bool value_for_name(StringRef name, ssize_t& out_value) const;
	bool allows_arbitrary_value() const { return allow_arbitrary_value_; }
	
	struct Entry {
		StringRef name;
		StringRef description;
		ssize_t value;
	};
	
	ArrayRef<Entry> entries() const { return entries_; }
private:
	template <typename T> friend struct EnumTypeBuilder;
	Array<Entry> entries_;
	ssize_t max_;
	ssize_t min_;
	bool allow_arbitrary_value_;
};

#define REFLECT_ENUM_TYPE(T) template <> struct BuildTypeInfo<T> { static const EnumType* build(); };


struct IntegerType : SimpleType {
	IntegerType(IAllocator& alloc, const TypeInfo& type_info, StringRef name, size_t width, bool is_signed = true) : SimpleType(alloc, type_info, name, width, width, false, is_signed) {}
	void deserialize_raw(byte*, const DocumentNode&, IUniverse&) const override;
	void serialize_raw(const byte*, DocumentNode&, IUniverse&) const override;
	void* cast(const SimpleType* to, void* o) const;
	size_t max() const;
	ssize_t min() const;
};

struct FloatType : SimpleType {
	FloatType(IAllocator& alloc, const TypeInfo& type_info, String name, size_t width) : SimpleType(alloc, type_info, name, width, width, true, true) {}
	void deserialize_raw(byte*, const DocumentNode& node, IUniverse&) const override;
	void serialize_raw(const byte*, DocumentNode&, IUniverse&) const override;
	void* cast(const SimpleType* to, void* o) const;
};

struct StringType : TypeFor<String> {
	static const StringType* get();
	
	void deserialize(String& place, const DocumentNode&, IUniverse&) const final;
	void serialize(const String& place, DocumentNode&, IUniverse&) const final;
	
	StringRef name() const final;
};
	
struct StringRefType : TypeFor<StringRef> {
	static const StringRefType* get();
	void deserialize(StringRef& place, const DocumentNode&, IUniverse&) const final;
	void serialize(const StringRef& place, DocumentNode&, IUniverse&) const final;
	StringRef name() const final;
};

struct DerivedType : Type {
	DerivedType(const TypeInfo& ti) : Type(ti) {}
};

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
	
template <> struct BuildTypeInfo<StringRef> {
	static const StringRefType* build() { return StringRefType::get(); }
};

template <typename T>
auto build_type_info()
-> decltype(BuildTypeInfo<T>::build())
{
	return BuildTypeInfo<T>::build();
}

template <typename T>
DLL_PUBLIC
typename std::enable_if<HasReflection<T>::Value, const typename T::TypeInfoType*>::type
get_type() {
	auto p = T::build_type_info__();
	ASSERT(p != nullptr);
	return p;
}
	
template <typename T>
DLL_PUBLIC
typename std::enable_if<!HasReflection<T>::Value, decltype(BuildTypeInfo<T>::build())>::type
get_type() {
	auto p = BuildTypeInfo<T>::build();
	ASSERT(p != nullptr);
	return p;
}
	
template <typename T>
DLL_PUBLIC
typename std::enable_if<HasReflection<T>::Value, const DerivedType*>::type
get_type(const T& object) {
	auto p = object.object_type();
	ASSERT(p != nullptr);
	return p;
}
	
template <typename T>
DLL_PUBLIC
typename std::enable_if<!HasReflection<T>::Value, const IType*>::type
get_type(const T& value) {
	return get_type<T>();
}

struct CheckHasGetType {
	template <typename T, typename TT = decltype(get_type<T>())>
	struct Check {};
};

template <typename T>
struct CanGetType : public HasMember<T, CheckHasGetType> {};



template <typename Last = void>
void append_type_names(FormattedStream& os) {
	const IType* t = get_type<Last>();
	os << t->name();
}

template <typename Head, typename Next, typename... Rest>
void append_type_names(FormattedStream& os) {
	const IType* t = get_type<Head>();
	os << t->name();
	os << ", ";
	append_type_names<Next, Rest...>(os);
}

template <typename... Args>
String get_signature_description(IAllocator& alloc) {
	StringStream ss(alloc);
	ss << '(';
	append_type_names<Args...>(ss);
	ss << ')';
	return ss.string(alloc);
}


template <typename Head = void>
void build_signature(Array<const IType*>& signature) {
	signature.push_back(get_type<Head>());
}
template <typename Head, typename Next, typename... Rest>
void build_signature(Array<const IType*>& signature) {
	signature.push_back(get_type<Head>());
	build_signature<Next, Rest...>(signature);
}

}

#endif /* end of include guard: TYPE_HPP_ZXLGBWRF */
