//
//  priority_queue.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 17/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_priority_queue_hpp
#define falling_priority_queue_hpp

#include "base/basic.hpp"
#include "memory/allocator.hpp"
#include <algorithm>

namespace falling {
	template <typename T> class Array;
	
	template <typename T, typename Container = Array<T>, typename Compare = std::less<T>>
	class PriorityQueue {
	public:
		explicit PriorityQueue(IAllocator& alloc = default_allocator());
		PriorityQueue(Compare cmp, IAllocator& alloc = default_allocator());
		PriorityQueue(const PriorityQueue<T,Container,Compare>& other, IAllocator& alloc = default_allocator());
		PriorityQueue(PriorityQueue<T,Container,Compare>&& other);
		
		bool operator==(ArrayRef<T> range) const;
		bool operator!=(ArrayRef<T> range) const;
		
		void insert(const T& element);
		
		T& top();
		const T& top() const;
		T pop();
		
		const Container& container() const { return container_; }
		const Compare& compare() const { return cmp_; }
		IAllocator& allocator() const;
		size_t size() const;
		
		using iterator = typename Container::iterator;
		using const_iterator = typename Container::const_iterator;
		iterator begin() { return container_.begin(); }
		iterator end() { return container_.end(); }
		const_iterator begin() const { return container_.begin(); }
		const_iterator end() const { return container_.end(); }
		
		FORWARD_TO_MEMBER(erase, container_, Container)
		FORWARD_TO_MEMBER(reserve, container_, Container);
	private:
		Compare cmp_;
		Container container_;
	};
	
	template <typename T, typename C, typename Cmp>
	PriorityQueue<T,C,Cmp>::PriorityQueue(IAllocator& alloc) : container_(alloc) {}
	
	template <typename T, typename C, typename Cmp>
	PriorityQueue<T,C,Cmp>::PriorityQueue(const PriorityQueue<T,C,Cmp>& other, IAllocator& alloc) : cmp_(other.cmp_), container_(other.container_, alloc) {}
	
	template <typename T, typename C, typename Cmp>
	PriorityQueue<T,C,Cmp>::PriorityQueue(PriorityQueue<T,C,Cmp>&& other) : cmp_(std::move(other.cmp_)), container_(std::move(other.container_)) {}
	
	template <typename T, typename C, typename Cmp>
	void PriorityQueue<T,C,Cmp>::insert(const T& element) {
		auto insertion_place = std::lower_bound(begin(), end(), element, cmp_);
		container_.insert(&element, &element + 1, insertion_place);
	}
	
	template <typename T, typename C, typename Cmp>
	const T& PriorityQueue<T,C,Cmp>::top() const {
		if (size() == 0) {
			throw IndexOutOfBoundsException();
		}
		return container_.back();
	}
	
	template <typename T, typename C, typename Cmp>
	T& PriorityQueue<T,C,Cmp>::top() {
		if (size() == 0) {
			throw IndexOutOfBoundsException();
		}
		return container_.back();
	}
	
	template <typename T, typename C, typename Cmp>
	T PriorityQueue<T,C,Cmp>::pop() {
		if (size() == 0) {
			throw IndexOutOfBoundsException();
		}
		iterator l = end() - 1;
		T x = std::move(*l);
		container_.erase(l);
		return x;
	}
	
	template <typename T, typename C, typename Cmp>
	IAllocator& PriorityQueue<T,C,Cmp>::allocator() const {
		return container_.allocator();
	}
	
	template <typename T, typename C, typename Cmp>
	size_t PriorityQueue<T,C,Cmp>::size() const {
		return container_.size();
	}
	
	template <typename T, typename C, typename Cmp>
	bool PriorityQueue<T,C,Cmp>::operator==(ArrayRef<T> range) const {
		if (range.size() != size()) return false;
		auto i0 = begin();
		auto i1 = range.begin();
		for (; i0 != end(); ++i0, ++i1) {
			if (*i0 != *i1) return false;
		}
		return true;
	}
	
	template <typename T, typename C, typename Cmp>
	bool PriorityQueue<T,C,Cmp>::operator!=(ArrayRef<T> range) const {
		return !(*this == range);
		
	}
	
	class FormattedStream;
	template <typename T, typename C, typename Cmp>
	FormattedStream& operator<<(FormattedStream& stream, const PriorityQueue<T, C, Cmp>& q) {
		stream << 'Q';
		stream << q.container();
		return stream;
	}
}

#endif
