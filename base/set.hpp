//
//  set.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 10/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef grace_set_hpp
#define grace_set_hpp

#include "base/basic.hpp"
#include "memory/allocator.hpp"
#include "io/formatters.hpp" // TODO: Get rid of dependency
#include <algorithm>

namespace grace {
	template <typename T> class Array;
	
	template <typename T, typename Container = Array<T>, typename Compare = Less>
	class Set : private Compare {
	public:
		using Self = Set<T,Container,Compare>;
		using iterator = typename Container::const_iterator;
		using const_iterator = typename Container::const_iterator;
		
		explicit Set(IAllocator& alloc = default_allocator());
		Set(Compare cmp, IAllocator& alloc = default_allocator());
		Set(const Set<T,Container,Compare>& other);
		Set(const Set<T,Container,Compare>& other, IAllocator& alloc);
		Set(Set<T,Container,Compare>&& other);
		Set(std::initializer_list<T> list, IAllocator& alloc = default_allocator());
		Set(std::initializer_list<T> list, Compare cmp, IAllocator& alloc = default_allocator());
		explicit Set(ArrayRef<T> list, IAllocator& alloc = default_allocator());
		Set(ArrayRef<T> list, Compare cmp, IAllocator& alloc = default_allocator());

		Self& operator=(const Self& other);
		Self& operator=(Self&& other);
		
		bool operator==(const Self& other) const;
		bool operator!=(const Self& other) const;
		bool operator==(ArrayRef<T> range) const;
		bool operator!=(ArrayRef<T> range) const;
		
		iterator insert(T element);
		
		const Container& container() const { return container_; }
		const Compare& compare() const { return *this; }
		
		const_iterator begin() const { return container_.begin(); }
		const_iterator end() const { return container_.end(); }
		
		template <typename ComparableKey>
		iterator find(const ComparableKey& key);
		template <typename ComparableKey>
		const_iterator find(const ComparableKey& key) const;
		
		iterator erase(iterator it);
		
		FORWARD_TO_MEMBER_CONST(size, container_, Container)
		FORWARD_TO_MEMBER(clear, container_, Container)
		FORWARD_TO_MEMBER(reserve, container_, Container)
		FORWARD_TO_MEMBER(allocator, container_, Container);
	private:
		Container container_;

		const Compare& cmp() const {
			return *this;
		}
	};
	
	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>::Set(IAllocator& alloc) : container_(alloc) {}
	
	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>::Set(const Set<T,C,Cmp>& other, IAllocator& alloc) : Cmp(other.cmp_), container_(other.container_, alloc) {}

	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>::Set(const Set<T,C,Cmp>& other) : Cmp(other.cmp_), container_(other.container_) {}
	
	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>::Set(Set<T,C,Cmp>&& other) : Cmp(move(other)), container_(std::move(other.container_)) {}

	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>::Set(std::initializer_list<T> list, IAllocator& alloc) : container_(alloc) {
		reserve(list.size());
		for (auto& x: list) {
			insert(move(x));
		}
	}

	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>::Set(std::initializer_list<T> list, Cmp cmp, IAllocator& alloc) : container_(alloc), Cmp(move(cmp)) {
		reserve(list.size());
		for (auto& x: list) {
			insert(move(x));
		}
	}

	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>::Set(ArrayRef<T> list, IAllocator& alloc) : container_(alloc) {
		reserve(list.size());
		for (auto& x: list) {
			insert(x);
		}
	}

	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>::Set(ArrayRef<T> list, Cmp cmp, IAllocator& alloc) : container_(alloc), Cmp(move(cmp)) {
		reserve(list.size());
		for (auto& x: list) {
			insert(x);
		}
	}

	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>& Set<T,C,Cmp>::operator=(const Set<T,C,Cmp>& other) {
		container_ = other.container_;
		return *this;
	}

	template <typename T, typename C, typename Cmp>
	Set<T,C,Cmp>& Set<T,C,Cmp>::operator=(Set<T,C,Cmp>&& other) {
		container_ = move(other.container_);
		return *this;
	}
	
	template <typename T, typename C, typename Cmp>
	typename Set<T,C,Cmp>::iterator Set<T,C,Cmp>::insert(T element) {
		auto insertion_place = std::lower_bound(container_.begin(), container_.end(), element, cmp());
		return container_.insert(std::move(element), insertion_place);
	}

	template <typename T, typename C, typename Cmp>
	bool Set<T,C,Cmp>::operator==(const Self& other) const {
		return (this == &other) || (container_ == other.container_);
	}

	template <typename T, typename C, typename Cmp>
	bool Set<T,C,Cmp>::operator!=(const Self& other) const {
		return (this != &other) && (container_ != other.container_);
	}
	
	template <typename T, typename C, typename Cmp>
	bool Set<T,C,Cmp>::operator==(ArrayRef<T> range) const {
		if (range.size() != size()) return false;
		auto i0 = begin();
		auto i1 = range.begin();
		for (; i0 != end(); ++i0, ++i1) {
			if (*i0 != *i1) return false;
		}
		return true;
	}
	
	template <typename T, typename C, typename Cmp>
	bool Set<T,C,Cmp>::operator!=(ArrayRef<T> range) const {
		return !(*this == range);
	}
	
	template <typename T, typename C, typename Cmp>
	template <typename ComparableKey>
	typename Set<T,C,Cmp>::iterator Set<T,C,Cmp>::find(const ComparableKey &key) {
		auto it = std::lower_bound(container_.begin(), container_.end(), key, cmp());
		if (*it == key) return it;
		return end();
	}
	
	template <typename T, typename C, typename Cmp>
	template <typename ComparableKey>
	typename Set<T,C,Cmp>::const_iterator Set<T,C,Cmp>::find(const ComparableKey &key) const {
		auto it = std::lower_bound(container_.begin(), container_.end(), key, cmp());
		if (*it == key) return it;
		return end();
	}
	
	template <typename T, typename C, typename Cmp>
	typename Set<T,C,Cmp>::iterator Set<T,C,Cmp>::erase(iterator it) {
		// TODO: Is there a better way to convert a const_iterator to an iterator?
		auto nit = container_.begin() + (it - container_.begin());
		return container_.erase(nit);
	}
	
	class FormattedStream;
	template <typename T, typename C, typename Cmp>
	FormattedStream& operator<<(FormattedStream& stream, const Set<T, C, Cmp>& q) {
		stream << 'S';
		stream << q.container();
		return stream;
	}
}

#endif
