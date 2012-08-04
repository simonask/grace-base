#pragma once
#ifndef ARRAY_HPP_6MM1YKSV
#define ARRAY_HPP_6MM1YKSV

#include "base/basic.hpp"
#include "base/array_ref.hpp"

#if defined(USE_STD_VECTOR)
#include <vector>
namespace falling {
template <typename T> using Array = std::vector<T>;
}
#else

#include <string>

namespace falling {
	
	struct IndexOutOfBoundsException {
		std::string what() { return "Index out of bounds."; }
	};

template <typename T>
class Array {
public:
	Array() : data_(nullptr), size_(0), alloc_size_(0) {}
	Array(const Array<T>& other);
	Array(Array<T>&& other);
	explicit Array(ArrayRef<T> array); // TODO!
	~Array();
	Array<T>& operator=(const Array<T>& other);
	Array<T>& operator=(Array<T>&& other);
	
	operator ArrayRef<T>() const {
		return ArrayRef<T>(data_, data_ + size_);
	}
	
	T& operator[](size_t idx);
	const T& operator[](size_t idx) const;
	
	uint32 size() const { return size_; }
	void push_back(T element);
	void reserve(size_t);
	void resize(size_t, T fill = T());
	void clear(bool deallocate = true);
	
	template <typename InputIterator>
	void insert(InputIterator begin, InputIterator end);
	
	template <typename... Args>
	void emplace_back(Args... args);
	
	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator;
	iterator begin() { return data_; }
	iterator end() { return data_ + size_; }
	const_iterator begin() const { return data_; }
	const_iterator end() const { return data_ + size_; }
	
	size_t erase(size_t idx);
	iterator erase(iterator);
private:
	T* data_;
	uint32 size_;
	uint32 alloc_size_;
	
	void check_index_valid(size_t idx) const;
};

template <typename T>
Array<T>::Array(const Array<T>& other) : data_(nullptr), size_(0), alloc_size_(0) {
	reserve(other.size());
	insert(other.begin(), other.end());
}

template <typename T>
Array<T>::Array(Array<T>&& other) : data_(other.data_), size_(other.size_), alloc_size_(other.alloc_size_) {
	other.data_ = nullptr;
	other.size_ = 0;
	other.alloc_size_ = 0;
}

template <typename T>
Array<T>::~Array() {
	clear(true);
}

template <typename T>
Array<T>& Array<T>::operator=(const Array<T>& other) {
	clear();
	reserve(other.size());
	insert(other.begin(), other.end());
	return *this;
}

template <typename T>
Array<T>& Array<T>::operator=(Array<T>&& other) {
	data_ = other.data_;
	size_ = other.size_;
	alloc_size_ = other.alloc_size_;
	other.data_ = nullptr;
	other.size_ = 0;
	other.alloc_size_ = 0;
	return *this;
}

template <typename T>
T& Array<T>::operator[](size_t idx) {
	check_index_valid(idx);
	return data_[idx];
}

template <typename T>
const T& Array<T>::operator[](size_t idx) const {
	check_index_valid(idx);
	return data_[idx];
}

template <typename T>
void Array<T>::push_back(T element) {
	reserve(size_+1);
	new(data_ + size_) T(std::move(element));
	size_++;
}

template <typename T>
void Array<T>::reserve(size_t new_size) {
	if (new_size > alloc_size_) {
		size_t req_size = alloc_size_ ? alloc_size_ : 1;
		if (new_size > 0x2000) { // Allocate precisely as much as needed when above 8K
			req_size = new_size;
		} else {
			while (req_size < new_size) req_size *= 2;
		}
		byte* p = new byte[sizeof(T)*req_size];
		for (uint32 i = 0; i < size_; ++i) {
			new(p+sizeof(T)*i) T(std::move(data_[i]));
			data_[i].~T();
		}
		delete[] reinterpret_cast<byte*>(data_);
		data_ = reinterpret_cast<T*>(p);
		alloc_size_ = (uint32)req_size;
	}
}

template <typename T>
void Array<T>::resize(size_t new_size, T x) {
	reserve(new_size);
	while (size_ < new_size) push_back(x);
}

template <typename T>
template <typename InputIterator>
void Array<T>::insert(InputIterator begin, InputIterator end) {
	reserve(end - begin);
	for (auto p = begin; p < end; ++p) {
		push_back(*p);
	}
}

template <typename T>
template <typename... Args>
void Array<T>::emplace_back(Args... args) {
	reserve(size_+1);
	new(data_ + size_) T(std::forward<Args>(args)...);
	size_++;
}

template <typename T>
void Array<T>::clear(bool deallocate) {
	for (uint32 i = 0; i < size_; ++i) {
		data_[i].~T();
	}
	size_ = 0;
	if (deallocate) {
		delete[] reinterpret_cast<byte*>(data_);
		data_ = nullptr;
		alloc_size_ = 0;
	}
}
	
template <typename T>
void Array<T>::check_index_valid(size_t idx) const {
	if (idx >= size_) {
		throw IndexOutOfBoundsException();
	}
}
	
template <typename T>
size_t Array<T>::erase(size_t idx) {
	check_index_valid(idx);
	if (idx == size_-1) {
		data_[idx].~T();
		--size_;
	} else {
		for (size_t i = idx; i < size_-1; ++i) {
			data_[idx] = std::move(data_[idx+1]);
		}
		--size_;
		// destroy last element
		data_[size_].~T();
	}
	return idx;
}

/*template <typename T>
Array<T>::iterator Array<T>::erase(Array<T>::iterator it) {
	ASSERT(it >= begin() && it < end());
	
}*/

}

#endif // if defined(USE_STD_VECTOR)

#endif /* end of include guard: ARRAY_HPP_6MM1YKSV */
