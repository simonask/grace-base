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
	
	template <typename T, typename Container = Array<T>, typename Compare = Less>
	class PriorityQueue {
	public:
		using iterator = typename Container::iterator;
		using const_iterator = typename Container::const_iterator;
	
		explicit PriorityQueue(IAllocator& alloc = default_allocator());
		PriorityQueue(Compare cmp, IAllocator& alloc = default_allocator());
		PriorityQueue(const PriorityQueue<T,Container,Compare>& other, IAllocator& alloc = default_allocator());
		PriorityQueue(PriorityQueue<T,Container,Compare>&& other);
		
		bool operator==(ArrayRef<T> range) const;
		bool operator!=(ArrayRef<T> range) const;
		
		iterator insert(T element);
		
		T& top();
		const T& top() const;
		T& back();
		const T& back() const;
		T pop();
		
		const Container& container() const { return container_; }
		const Compare& compare() const { return cmp_; }
		IAllocator& allocator() const;
		size_t size() const;
		
		iterator begin() { return container_.begin(); }
		iterator end() { return container_.end(); }
		const_iterator begin() const { return container_.begin(); }
		const_iterator end() const { return container_.end(); }
		
		template <typename ComparableKey>
		iterator find(const ComparableKey& key);
		template <typename ComparableKey>
		const_iterator find(const ComparableKey& key) const;
		
		FORWARD_TO_MEMBER(clear, container_, Container)
		FORWARD_TO_MEMBER(erase, container_, Container)
		FORWARD_TO_MEMBER(reserve, container_, Container)
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
	typename PriorityQueue<T,C,Cmp>::iterator PriorityQueue<T,C,Cmp>::insert(T element) {
		auto insertion_place = std::lower_bound(begin(), end(), element, cmp_);
		return container_.insert(std::move(element), insertion_place);
	}
	
	template <typename T, typename C, typename Cmp>
	const T& PriorityQueue<T,C,Cmp>::top() const {
		return container_.back();
	}
	
	template <typename T, typename C, typename Cmp>
	T& PriorityQueue<T,C,Cmp>::top() {
		return container_.back();
	}
	
	template <typename T, typename C, typename Cmp>
	const T& PriorityQueue<T,C,Cmp>::back() const {
		return container_.front();
	}
	
	template <typename T, typename C, typename Cmp>
	T& PriorityQueue<T,C,Cmp>::back() {
		return container_.front();
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
	
	template <typename T, typename C, typename Cmp>
	template <typename ComparableKey>
	typename PriorityQueue<T,C,Cmp>::iterator PriorityQueue<T,C,Cmp>::find(const ComparableKey &key) {
		auto it = std::lower_bound(container_.begin(), container_.end(), key, cmp_);
		if (*it == key) return it;
		return end();
	}
	
	template <typename T, typename C, typename Cmp>
	template <typename ComparableKey>
	typename PriorityQueue<T,C,Cmp>::const_iterator PriorityQueue<T,C,Cmp>::find(const ComparableKey &key) const {
		auto it = std::lower_bound(container_.begin(), container_.end(), key, cmp_);
		if (*it == key) return it;
		return end();
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
