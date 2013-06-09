#pragma once
#ifndef ARRAY_TYPE_HPP_JIO2A6YN
#define ARRAY_TYPE_HPP_JIO2A6YN

#include "type/type.hpp"

namespace grace {

struct IArrayReader;
struct IArrayWriter;

struct ArrayType : DerivedType {
public:
	virtual bool is_variable_length() const = 0;
	virtual const Type* element_type() const = 0;
	const Type* type_of_element(size_t idx) const { return element_type(); }
protected:
	ArrayType(const TypeInfo& ti) : DerivedType(ti) {}
	
	void deserialize_array(IArrayWriter&, const ArchiveNode&, IUniverse&) const;
	void serialize_array(IArrayReader&, ArchiveNode&, IUniverse&) const;
};

String build_variable_length_array_type_name(IAllocator& alloc, StringRef base_container_name, const Type* element_type);

template <typename Container>
struct VariableLengthArrayType : TypeFor<Container, ArrayType> {
public:
	typedef typename Container::value_type ElementType;
	VariableLengthArrayType(IAllocator& alloc, StringRef base_container_name) : name_(build_variable_length_array_type_name(alloc, base_container_name, get_type<ElementType>())) {}
	size_t num_elements() const { return SIZE_T_MAX; }
	size_t offset_of_element(size_t idx) const { return idx * this->element_type_->size(); }
	
	void deserialize(Container& place, const ArchiveNode& node, IUniverse&) const;
	void serialize(const Container& place, ArchiveNode& node, IUniverse&) const;
	
	StringRef name() const { return name_; }
	bool is_variable_length() const { return true; }
	const Type* element_type() const { return get_type<ElementType>(); }
private:
	String name_;
};

template <typename T>
struct BuildTypeInfo<Array<T>> {
	static const ArrayType* build() {
		static const auto type = new_static VariableLengthArrayType<Array<T>>(static_allocator(), "Array");
		return type;
	}
};

struct IArrayReader {
	virtual void* get_current() = 0;
	virtual bool next() = 0;
};

struct IArrayWriter {
	virtual void reserve(size_t sz) = 0;
	virtual void push_back_move(void* data) = 0;
};

template <typename T>
struct ArrayReader : IArrayReader {
	ArrayReader(const T& a) : array_(a) {}
	const T& array_;
	typename T::const_iterator p_;
	bool init_ = false;
	
	bool next() final {
		if (!init_) {
			init_ = true;
			p_ = array_.begin();
		} else {
			++p_;
		}
		return p_ != array_.end();
	}
	void* get_current() final {
		return (void*)p_.get();
	}
};

template <typename T>
struct ArrayWriter : IArrayWriter {
	ArrayWriter(T& a) : array_(a) {}
	T& array_;
	
	void reserve(size_t sz) final { array_.reserve(sz); }
	void push_back_move(void* data) {
		using V = typename T::value_type;
		array_.push_back(move(*reinterpret_cast<V*>(data)));
	}
};

template <typename T>
void VariableLengthArrayType<T>::deserialize(T& obj, const ArchiveNode& node, IUniverse& universe) const {
	ArrayWriter<T> w(obj);
	this->deserialize_array(w, node, universe);
}

template <typename T>
void VariableLengthArrayType<T>::serialize(const T& obj, ArchiveNode& node, IUniverse& universe) const {
	ArrayReader<T> r(obj);
	this->serialize_array(r, node, universe);
}

}

#endif /* end of include guard: ARRAY_TYPE_HPP_JIO2A6YN */
