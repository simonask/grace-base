//
//  multiset.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_multiset_hpp
#define grace_multiset_hpp

#include "base/basic.hpp"

namespace grace {
	template <typename T> class Array;
	
	template <typename T, typename C = Array<T>>
	class MultiSet {
	public:
		explicit MultiSet(IAllocator& alloc = default_allocator()) : container_(alloc) {}
		
		using iterator = typename C::iterator;
		using const_iterator = typename C::const_iterator;
		
		template <typename InputIterator>
		void insert(InputIterator b, InputIterator e) {
			for (auto it = b; it != e; ++it) {
				insert(*it);
			}
		}
		
		template <typename InputIterator>
		void insert_move(InputIterator b, InputIterator e) {
			for (auto it = b; it != e; ++it) {
				insert(move(*it));
			}
		}
		
		iterator insert(T object) {
			auto it = find_insertion_point(object);
			return container_.insert(move(object), it);
		}
		
		FORWARD_TO_MEMBER_CONST(size, container_, C)
		FORWARD_TO_MEMBER_CONST(capacity, container_, C)
		FORWARD_TO_MEMBER(reserve, container_, C)
		FORWARD_TO_MEMBER(begin, container_, C)
		FORWARD_TO_MEMBER_CONST(begin, container_, C)
		FORWARD_TO_MEMBER(end, container_, C)
		FORWARD_TO_MEMBER_CONST(end, container_, C)
		FORWARD_TO_MEMBER(erase, container_, C)
	private:
		C container_;
		
		iterator find_insertion_point(const T& value) {
			return std::lower_bound(begin(), end(), value);
		}
	};
}

#endif
