//
//  array_ref.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_array_ref_hpp
#define grace_array_ref_hpp

#include <initializer_list>
#include "base/iterators.hpp"
#include "base/basic.hpp"

namespace grace {
	struct Empty {};
	
	#define AREF(static_array) (ArrayRef<typename std::remove_reference<decltype(static_array[0])>::type>(static_array, static_array + sizeof(static_array)/sizeof(static_array[0])))

	template <typename T>
	struct ArrayRef {
	public:
		constexpr ArrayRef() : begin_(nullptr), end_(nullptr) {}
		constexpr ArrayRef(Empty e) : begin_(nullptr), end_(nullptr) {}
		template <size_t N, typename C = T>
		constexpr ArrayRef(const C(&data)[N]) : begin_(data), end_(data + N) { }
		constexpr ArrayRef(T* begin, T* end) : begin_(begin), end_(end) { /*ASSERT(begin_ <= end_);*/ }
		ArrayRef(const ArrayRef<T>& other) = default;
		ArrayRef(ArrayRef<T>&& other) = default;
		ArrayRef<T>& operator=(const ArrayRef<T>& other) = default;
		ArrayRef<T>& operator=(ArrayRef<T>&& other) = default;
		bool operator==(const ArrayRef<T>& other) const;
		bool operator!=(const ArrayRef<T>& other) const;
		
		size_t size() const { return end_ - begin_; }
		
		const T& operator[](size_t idx) const { ASSERT(idx < size()); return begin_[idx]; }
		T& operator[](size_t idx) { ASSERT(idx < size()); return begin_[idx]; }
		
		typedef const T value_type;
		using iterator = LinearMemoryIterator<T, false>;
		using const_iterator = LinearMemoryIterator<T, true>;
		iterator begin() { return begin_; }
		iterator end() { return end_; }
		const_iterator begin() const { return begin_; }
		const_iterator end() const { return end_; }
		
		T* data() { return begin_; }
		const T* data() const { return begin_; }
	private:
		T* begin_;
		T* end_;
	};
	
	template <typename T>
	bool ArrayRef<T>::operator==(const ArrayRef<T> &other) const {
		if (size() == other.size()) {
			for (size_t i = 0; i < size(); ++i) {
				if (begin_[i] != other.begin_[i]) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	
	template <typename T>
	bool ArrayRef<T>::operator!=(const ArrayRef<T>& other) const {
		return !(*this == other);
	}
	
	struct IAttribute;
	struct ISlot;
	class String;
	struct StringRef;
	struct Type;
	extern template struct ArrayRef<byte>;
	extern template struct ArrayRef<char>;
	extern template struct ArrayRef<float32>;
	extern template struct ArrayRef<String>;
	extern template struct ArrayRef<StringRef>;
	extern template struct ArrayRef<const Type*>;
	extern template struct ArrayRef<const IAttribute*>;
	extern template struct ArrayRef<const ISlot* const>;
}

#endif
