//
//  maxarray.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 15/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_maxarray_hpp
#define grace_maxarray_hpp

#include "base/array_ref.hpp"
#include "base/iterators.hpp"
#include "base/raise.hpp"
#include "base/exceptions.hpp"
#include <type_traits>

namespace grace {
	struct MaxArrayTooSmallError : ErrorBase<MaxArrayTooSmallError> {};

	template <typename T, uint32 Max>
	class MaxArray {
	public:
		using value_type = T;
		using iterator = LinearMemoryIterator<T, false>;
		using const_iterator = LinearMemoryIterator<T, true>;
		
		MaxArray() {}
		MaxArray(std::initializer_list<T> list);
		explicit MaxArray(ArrayRef<T> array);
		~MaxArray();
		MaxArray<T,Max>& operator=(std::initializer_list<T> list);
		MaxArray<T,Max>& operator=(ArrayRef<T> array);
		bool operator==(ArrayRef<T> other) const;
		bool operator==(ArrayRef<const T> other) const;
		bool operator!=(ArrayRef<T> other) const;
		bool operator!=(ArrayRef<const T> other) const;
		
		ArrayRef<T> ref() { return ArrayRef<T>(ptr(), size_); }
		ArrayRef<const T> cref() const { return ArrayRef<const T>(ptr(), ptr() + size_); }
		ArrayRef<const T> ref() const { return cref(); }
		
		operator ArrayRef<T>() const {
			return cref();
		}
		
		T& operator[](size_t idx);
		const T& operator[](size_t idx) const;
		
		uint32 size() const { return size_; };
		uint32 capacity() const { return Max; }
		void push_back(T element);
		T pop_back();
		T& back();
		T& front();
		void resize(size_t, T fill = T());
		void clear();
		template <typename InputIterator>
		iterator insert(InputIterator begin, InputIterator end);
		template <typename InputIterator>
		iterator insert(InputIterator begin, InputIterator end, iterator before);
		template <typename InputIterator>
		iterator insert_move(InputIterator begin, InputIterator end);
		template <typename InputIterator>
		iterator insert_move(InputIterator begin, InputIterator end, iterator before);
		iterator insert(T element, iterator before);
		
		template <typename... Args>
		void emplace_back(Args&&... args);
		
		iterator begin() { return ptr(); }
		iterator end() { return ptr() + size_; }
		const_iterator begin() const { return ptr(); }
		const_iterator end() const { return ptr() + size_; }
		T* data() { return ptr(); }
		const T* data() const { return ptr(); }
		
		size_t erase(size_t idx);
		iterator erase(iterator);
	private:
		using Storage = typename std::aligned_storage<sizeof(T) * Max, alignof(T)>::type;
		Storage memory_;
		uint32 size_ = 0;
		T* ptr() { return reinterpret_cast<T*>(&memory_); }
		const T* ptr() const { return reinterpret_cast<const T*>(&memory_); }
		void check_index_valid(size_t idx) const;
		template <typename InputIterator, bool Move>
		iterator insert_impl(InputIterator b, InputIterator e, iterator before);
	};
	
	template <typename T, uint32 C>
	MaxArray<T,C>::MaxArray(std::initializer_list<T> list) {
		if (list.size() > C) {
			raise<MaxArrayTooSmallError>("Tried to initialize a MaxArray<T,{1}> with a list of length {0}.", list.size(), C);
		}
		insert(list.begin(), list.end());
	}
	
	template <typename T, uint32 C>
	MaxArray<T,C>::MaxArray(ArrayRef<T> list) {
		if (list.size() > C) {
			raise<MaxArrayTooSmallError>("Tried to initialize a MaxArray<T,{1}> with a list of length {0}.", list.size(), C);
		}
		insert(list.begin(), list.end());
	}
	
	template <typename T, uint32 C>
	MaxArray<T,C>::~MaxArray() {
		clear();
	}
	
	template <typename T, uint32 C>
	MaxArray<T,C>& MaxArray<T,C>::operator=(std::initializer_list<T> list) {
		clear();
		insert_move(list.begin(), list.end());
	}
	
	template <typename T, uint32 C>
	T& MaxArray<T,C>::operator[](size_t idx) {
		check_index_valid(idx);
		return *(ptr()+idx);
	}
	
	template <typename T, uint32 C>
	const T& MaxArray<T,C>::operator[](size_t idx) const {
		check_index_valid(idx);
		return *(ptr()+idx);
	}
	
	template <typename T, uint32 C>
	bool MaxArray<T,C>::operator==(ArrayRef<const T> other) const {
		return other == ref();
	}
	
	template <typename T, uint32 C>
	bool MaxArray<T,C>::operator==(ArrayRef<T> other) const {
		return other == ref();
	}
	
	template <typename T, uint32 C>
	bool MaxArray<T,C>::operator!=(ArrayRef<const T> other) const {
		return other != ref();
	}
	
	template <typename T, uint32 C>
	bool MaxArray<T,C>::operator!=(ArrayRef<T> other) const {
		return other != ref();
	}
	
	template <typename T, uint32 C>
	void MaxArray<T,C>::push_back(T value) {
		if (size_+1 > C) {
			raise<MaxArrayTooSmallError>("Tried to insert element at position {0} in a MaxArray<T,{1}>.", size_, C);
		}
		new(ptr() + size_) T(move(value));
		++size_;
	}
	
	template <typename T, uint32 C>
	template <typename... Args>
	void MaxArray<T,C>::emplace_back(Args&&... args) {
		if (size_+1 > C) {
			raise<MaxArrayTooSmallError>("Tried to insert element at position {0} in a MaxArray<T,{1}>.", size_, C);
		}
		new(ptr() + size_) T(std::forward<Args>(args)...);
		++size_;
	}
	
	template <typename T, uint32 C>
	T MaxArray<T,C>::pop_back() {
		if (size_ == 0) {
			raise<IndexOutOfBoundsException>("Requested index {0} from MaxArray<T,{1}>.", size_, size_);
		}
		size_t idx = size_ - 1;
		T tmp = move(*(ptr() + idx));
		(ptr() + idx)->~T();
		return move(tmp);
	}
	
	template <typename T, uint32 C>
	void MaxArray<T,C>::resize(size_t new_size, T fill) {
		if (new_size > C) {
			raise<MaxArrayTooSmallError>("Attempted to resize MaxArray<T,{1}> beyond its memory to size {0}.", new_size, C);
		}
		if (size_ < new_size) {
			while (size_ < new_size) push_back(fill);
		} else {
			while (size_ > new_size) pop_back();
		}
	}
	
	template <typename T, uint32 C>
	void MaxArray<T,C>::clear() {
		for (uint32 i = 0; i < size_; ++i) {
			(ptr() + i)->~T();
		}
		size_ = 0;
	}
	
	template <typename T, uint32 C>
	template <typename InputIterator>
	typename MaxArray<T,C>::iterator MaxArray<T,C>::insert(InputIterator b, InputIterator e) {
		return insert(b, e, end());
	}
	
	template <typename T, uint32 C>
	template <typename InputIterator, bool Move>
	typename MaxArray<T,C>::iterator MaxArray<T,C>::insert_impl(InputIterator b, InputIterator e, iterator before) {
		size_t add_len = e - b;
		size_t num_move = end() - before;
		if (size_ + add_len > C) {
			raise<MaxArrayTooSmallError>("Attempted to insert {0} elements in MaxArray<T,{1}> already containing {2} elements.", add_len, C, size_);
		}
		iterator move_end = end();
		iterator move_begin = before;
		iterator move_target_end = end() + add_len;
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
				move_or_copy<Move>(*dst, *it);
			} else {
				// moving to uninitialized memory
				move_or_copy_construct<Move>(dst.get(), *it);
			}
		}
		size_ += add_len;
		return before;
	}
	
	template <typename T, uint32 C>
	template <typename InputIterator>
	typename MaxArray<T,C>::iterator MaxArray<T,C>::insert(InputIterator b, InputIterator e, iterator before) {
		return insert_impl<InputIterator, false>(b, e, before);
	}
	
	template <typename T, uint32 C>
	template <typename InputIterator>
	typename MaxArray<T,C>::iterator MaxArray<T,C>::insert_move(InputIterator b, InputIterator e) {
		return insert_move(b, e, end());
	}
	
	template <typename T, uint32 C>
	template <typename InputIterator>
	typename MaxArray<T,C>::iterator MaxArray<T,C>::insert_move(InputIterator b, InputIterator e, iterator before) {
		return insert_impl<InputIterator, true>(b, e, before);
	}
	
	template <typename T, uint32 C>
	typename MaxArray<T,C>::iterator MaxArray<T,C>::insert(T element, iterator before) {
		return insert_move(&element, &element + 1, before);
	}
	
	template <typename T, uint32 C>
	size_t MaxArray<T,C>::erase(size_t idx) {
		check_index_valid(idx);
		if (idx == size_-1) {
			ptr()[idx].~T();
			--size_;
		} else {
			for (size_t i = idx; i < size_-1; ++i) {
				ptr()[i] = std::move(ptr()[i+1]);
			}
			--size_;
			// destroy last element
			ptr()[size_].~T();
		}
		return idx;
	}
	
	template <typename T, uint32 C>
	typename MaxArray<T, C>::iterator MaxArray<T,C>::erase(iterator it) {
		size_t idx = it - begin();
		check_index_valid(idx);
		idx = erase(idx);
		return begin() + idx;
	}
	
	template <typename T, uint32 C>
	void MaxArray<T,C>::check_index_valid(size_t idx) const {
		if (idx >= size_ || idx >= C) {
			raise<IndexOutOfBoundsException>("Requested index {0} of MaxArray<T,{1}>.", idx, C);
		}
	}

	template <typename OS, typename T, uint32 C>
	OS& operator<<(OS& stream, const MaxArray<T, C>& array) {
		stream << '[';
		stream << join(array, ", ");
		stream << ']';
		return stream;
	}
}

#endif
