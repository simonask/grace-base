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
	struct ArrayRefBase {
	public:
		bool operator==(const ArrayRefBase<T>& other) const;
		bool operator!=(const ArrayRefBase<T>& other) const;
		
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
		constexpr const T* data() const { return begin_; }
	protected:
		constexpr ArrayRefBase() {}
		constexpr ArrayRefBase(T* begin, T* end) : begin_(begin), end_(end) {}
		template <size_t N, typename C = T>
		constexpr ArrayRefBase(const C(&data)[N]) : begin_(data), end_(data + N) {}
		ArrayRefBase(const ArrayRefBase<T>& other) = default;
		ArrayRefBase(ArrayRefBase<T>&& other) = default;
		T* begin_;
		T* end_;
	};
	
	template <typename T> struct ArrayRef;
	
	template <typename T>
	struct ArrayRef<const T> : ArrayRefBase<const T> {
	public:
		constexpr ArrayRef() {}
		constexpr ArrayRef(Empty empty) {}
		ArrayRef(NothingType nothing) {}
		constexpr ArrayRef(const T* begin, const T* end) : ArrayRefBase<const T>(begin, end) {}
		template <size_t N, typename C = T>
		constexpr ArrayRef(const C(&data)[N]) : ArrayRefBase<const T>(data) {}
		ArrayRef(const ArrayRef<const T>& other) = default;
		ArrayRef(ArrayRef<const T>&& other) = default;
		
		// Create ArrayRef<const T> from ArrayRef<T>
		ArrayRef(const ArrayRef<T>& other) : ArrayRefBase<const T>(other.data(), other.data() + other.size()) {}
		
		// Create ArrayRef<T*> from ArrayRef<U*>
		template <typename U>
		ArrayRef(const ArrayRef<U>& other, typename std::enable_if<std::is_pointer<U>::value && std::is_pointer<T>::value && std::is_convertible<U, T>::value, void>::type* dummy = nullptr)
		: ArrayRefBase<const T>((const T*)other.data(), (const T*)other.data() + other.size())
		{}
		
		ArrayRef<const T>& operator=(const ArrayRef<const T>& other) {
			this->begin_ = other.begin_;
			this->end_ = other.end_;
			return *this;
		}
		ArrayRef<const T>& operator=(ArrayRef<const T>&& other) {
			this->begin_ = other.begin_;
			this->end_ = other.end_;
			return *this;
		}
	};
	
	template <typename T>
	struct ArrayRef : ArrayRefBase<T> {
		constexpr ArrayRef() {}
		constexpr ArrayRef(Empty empty) {}
		ArrayRef(NothingType nothing) {}
		constexpr ArrayRef(T* begin, T* end) : ArrayRefBase<T>(begin, end) {}
		template <size_t N, typename C = T>
		constexpr ArrayRef(const C(&data)[N]) : ArrayRefBase<T>(data) {}
		ArrayRef(const ArrayRef<T>& other) = default;
		ArrayRef(ArrayRef<T>&& other) = default;
		
		// Create ArrayRef<T*> from ArrayRef<U*>
		template <typename U>
		ArrayRef(const ArrayRef<U>& other, typename std::enable_if<std::is_pointer<U>::value && std::is_pointer<T>::value && std::is_convertible<U, T>::value, void>::type* dummy = nullptr)
		: ArrayRefBase<T>((T*)other.data(), (T*)other.data() + other.size())
		{}
		
		ArrayRef<T>& operator=(const ArrayRef<T>& other) {
			this->begin_ = other.begin_;
			this->end_ = other.end_;
			return *this;
		}
		ArrayRef<T>& operator=(ArrayRef<T>&& other) {
			this->begin_ = other.begin_;
			this->end_ = other.end_;
			return *this;
		}
	};

	
	template <typename T>
	bool ArrayRefBase<T>::operator==(const ArrayRefBase<T>& other) const {
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
	bool ArrayRefBase<T>::operator!=(const ArrayRefBase<T>& other) const {
		return !(*this == other);
	}
	
	struct IAttribute;
	struct ISlot;
	class String;
	struct StringRef;
	struct IType;
	extern template struct ArrayRef<byte>;
	extern template struct ArrayRef<char>;
	extern template struct ArrayRef<float32>;
	extern template struct ArrayRef<String>;
	extern template struct ArrayRef<StringRef>;
	extern template struct ArrayRef<const IType*>;
	extern template struct ArrayRef<const IAttribute*>;
	extern template struct ArrayRef<const ISlot* const>;
}

#endif
