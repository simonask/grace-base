//
//  array_ref.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_array_ref_hpp
#define falling_array_ref_hpp

#include <initializer_list>

namespace falling {
	struct Empty {};

	template <typename T>
	struct ArrayRef {
	public:
		ArrayRef() : begin_(nullptr), end_(nullptr) {}
		ArrayRef(Empty e) : begin_(nullptr), end_(nullptr) {}
		template <size_t N>
		ArrayRef(const T(&data)[N]) : begin_(data), end_(data + N) { }
		ArrayRef(T* begin, T* end) : begin_(begin), end_(end) { ASSERT(begin_ <= end_); }
		ArrayRef(const ArrayRef<T>& other) = default;
		ArrayRef(ArrayRef<T>&& other) = default;
		ArrayRef<T>& operator=(const ArrayRef<T>& other) = default;
		ArrayRef<T>& operator=(ArrayRef<T>&& other) = default;
		bool operator==(const ArrayRef<T>& other) const { return begin_ == other.begin_ && end_ == other.end_; }
		bool operator!=(const ArrayRef<T>& other) const { return begin_ != other.begin_ && end_ != other.end_; }
		
		size_t size() const { return end_ - begin_; }
		
		T& operator[](size_t idx) { ASSERT(idx < size()); return begin_[idx]; }
		const T& operator[](size_t idx) const { ASSERT(idx < size()); return begin_[idx]; }
		
		typedef T value_type;
		typedef T* iterator;
		typedef const T* const_iterator;
		iterator begin() { return begin_; }
		iterator end() { return end_; }
		const_iterator begin() const { return begin_; }
		const_iterator end() const { return end_; }
	private:
		T* begin_;
		T* end_;
	};
}

#endif
