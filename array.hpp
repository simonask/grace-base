#pragma once
#ifndef ARRAY_HPP_6MM1YKSV
#define ARRAY_HPP_6MM1YKSV

#include "base/basic.hpp"
#include "base/array_ref.hpp"
#include "memory/allocator.hpp"
#include "base/iterators.hpp"

#if defined(USE_STD_VECTOR)
#include <vector>
namespace falling {
template <typename T> using Array = std::vector<T>;
}
#else

namespace falling {

template <typename T>
class Array {
public:
	typedef T value_type;
	using iterator = LinearMemoryIterator<Array<T>, T, false>;
	using const_iterator = LinearMemoryIterator<Array<T>, T, true>;
	
	Array() : allocator_(default_allocator()) {}
	explicit Array(IAllocator& alloc) : allocator_(alloc) {}
	Array(std::initializer_list<T> list, IAllocator& alloc = default_allocator());
	Array(const Array<T>& other, IAllocator& alloc = default_allocator());
	Array(Array<T>&& other);
	explicit Array(ArrayRef<T> array, IAllocator& alloc = default_allocator()); // TODO!
	~Array();
	Array<T>& operator=(std::initializer_list<T> list);
	Array<T>& operator=(const Array<T>& other);
	Array<T>& operator=(Array<T>&& other);
	bool operator==(const Array<T>& other) const;
	bool operator!=(const Array<T>& other) const { return !(*this == other); }
	
	IAllocator& allocator() const { return allocator_; }
	
	ArrayRef<T> ref() const {
		return ArrayRef<T>(data_, data_ + size_);
	}
	
	operator ArrayRef<T>() const {
		return ref();
	}
	
	T& operator[](size_t idx);
	const T& operator[](size_t idx) const;
	
	uint32 size() const { return size_; }
	void push_back(T element);
	T pop_back();
	T& back();
	const T& back() const;
	T& front();
	const T& front() const;
	void reserve(size_t);
	void resize(size_t, T fill = T());
	void clear(bool deallocate = true);
	
	template <typename InputIterator>
	void insert(InputIterator begin, InputIterator end);
	
	template <typename InputIterator>
	void insert(InputIterator begin, InputIterator end, iterator before);
	
	void insert(T element, iterator before);
	
	template <typename... Args>
	void emplace_back(Args... args);
	
	iterator begin() { return data_; }
	iterator end() { return data_ + size_; }
	const_iterator begin() const { return data_; }
	const_iterator end() const { return data_ + size_; }
	T* data() { return data_; }
	const T* data() const { return data_; }
	
	size_t erase(size_t idx);
	iterator erase(iterator);
private:
	IAllocator& allocator_;
	T* data_ = nullptr;
	uint32 size_ = 0;
	uint32 alloc_size_ = 0;
	
	void check_index_valid(size_t idx) const;
};

template <typename T>
Array<T>::Array(std::initializer_list<T> list, IAllocator& alloc) : allocator_(alloc) {
	insert(list.begin(), list.end());
}

template <typename T>
Array<T>::Array(const Array<T>& other, IAllocator& alloc) : allocator_(alloc) {
	reserve(other.size());
	insert(other.begin(), other.end());
}

template <typename T>
Array<T>::Array(Array<T>&& other) : allocator_(other.allocator_), data_(other.data_), size_(other.size_), alloc_size_(other.alloc_size_) {
	other.data_ = nullptr;
	other.size_ = 0;
	other.alloc_size_ = 0;
}

template <typename T>
Array<T>::~Array() {
	clear(true);
}

template <typename T>
Array<T>& Array<T>::operator=(std::initializer_list<T> list) {
	clear(list.size() < alloc_size_); // deallocate if we have more memory than we need for the list
	insert(list.begin(), list.end());
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
bool Array<T>::operator==(const Array<T>& other) const {
	if (size() == other.size()) {
		for (size_t i = 0; i < size(); ++i) {
			if (data_[i] != other[i]) return false;
		}
		return true;
	}
	return false;
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
const T& Array<T>::back() const {
	check_index_valid(size_-1);
	return data_[size_-1];
}
	
template <typename T>
T& Array<T>::back() {
	check_index_valid(size_-1);
	return data_[size_-1];
}
	
template <typename T>
const T& Array<T>::front() const {
	check_index_valid(0);
	return data_[0];
}

template <typename T>
T& Array<T>::front() {
	check_index_valid(0);
	return data_[0];
}
	

template <typename T>
void Array<T>::push_back(T element) {
	reserve(size_+1);
	new(data_ + size_) T(std::move(element));
	size_++;
}
	
template <typename T>
T Array<T>::pop_back() {
	check_index_valid(size_-1);
	T element = std::move(data_[size_-1]);
	erase(size_-1);
	return element;
}

template <typename T>
void Array<T>::reserve(size_t new_size) {
	if (new_size > alloc_size_) {
		size_t req_size = alloc_size_ ? alloc_size_ : 1;
		if (new_size*sizeof(T) > 0x2000) { // Allocate precisely as much as needed when above 8K
			req_size = new_size;
		} else {
			while (req_size < new_size) req_size *= 2;
		}
		T* new_data = (T*)allocator_.allocate(sizeof(T)*req_size, alignof(T));
		for (size_t i = 0; i < size_; ++i) {
			new(new_data+i) T(std::move(data_[i]));
			data_[i].~T();
		}
		allocator_.free(data_);
		data_ = new_data;
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
void Array<T>::insert(InputIterator b, InputIterator e) {
	insert(b, e, end());
}

template <typename T>
template <typename InputIterator>
void Array<T>::insert(InputIterator b, InputIterator e, iterator before) {
	size_t add_len = e - b;
	size_t num_move = end() - before;
	size_t before_idx = before - begin();
	reserve(size_ + add_len);
	// reserve invalidates iterators, so recalculate it:
	before = begin() + before_idx;
	iterator move_end = end();
	iterator move_begin = before;
	iterator move_target_end = end() + add_len;
	iterator move_target_begin = before + add_len;
	for (size_t i = 0; i < num_move; ++i) {
		iterator src = move_end - i - 1;
		iterator dst = move_target_end - i - 1;
		if (dst >= end()) {
			// moving to uninitialized memory
			new(dst.get()) T(std::move(*src));
		} else {
			// moving to previously initialized memory
			*dst = std::move(*src);
		}
	}
	size_t i = 0;
	for (auto it = b; it != e; ++it, ++i) {
		iterator dst = before + i;
		if (dst < end()) {
			// moving to previously initialized memory
			*dst = *it;
		} else {
			// moving to uninitialized memory
			new(dst.get()) T(*it);
		}
	}
	size_ += add_len;
}
	
template <typename T>
void Array<T>::insert(T element, iterator before) {
	// TODO: Move semantics
	insert(&element, &element + 1, before);
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
		allocator_.free(data_);
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
			data_[i] = std::move(data_[i+1]);
		}
		--size_;
		// destroy last element
		data_[size_].~T();
	}
	return idx;
}

template <typename T>
typename Array<T>::iterator Array<T>::erase(Array<T>::iterator it) {
	size_t idx = it - begin();
	check_index_valid(idx);
	idx = erase(idx);
	return begin() + idx;
}

}

#endif // if defined(USE_STD_VECTOR)

#endif /* end of include guard: ARRAY_HPP_6MM1YKSV */
