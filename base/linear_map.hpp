//
//  linear_map.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 06/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_linear_map_hpp
#define grace_linear_map_hpp

#include "base/basic.hpp"
#include "base/array.hpp"

#include <algorithm>

namespace grace {
	template <typename K, typename V>
	class LinearMap {
	public:
		struct Pair { K key; V value; }
		
		LinearMap() {}
		LinearMap(const LinearMap& other) : data_(other.data_) {}
		LinearMap(LinearMap&& other) : data_(std::move(other.data_)) {}
		
		V& operator[](const K& key);
		typedef typename Array<Pair>::iterator iterator;
		typedef typename Array<Pair>::const_iterator const_iterator;
		FORWARD_TO_MEMBER(begin, data_, Array<Pair>)
		FORWARD_TO_MEMBER(end,   data_, Array<Pair>)
		FORWARD_TO_CONST_MEMBER(begin, data_, Array<Pair>)
		FORWARD_TO_CONST_MEMBER(end,   data_, Array<Pair>)
		FORWARD_TO_CONST_MEMBER(size,  data_, Array<Pair>)
		FORWARD_TO_MEMBER(clear, data_, Array<Pair>)
		
		void erase(iterator);
		iterator find(const Key& k);
		const_iterator find(const Key& k) const;
		iterator lower_bound(const Key& k);
		const_iterator lower_bound(const Key& k) const;
		iterator upper_bound(const Key& k);
		const_iterator upper_bound(const Key& k);
		
		template <typename InputIterator, typename >
		void insert(InputIterator begin, InputIterator end);
		
		iterator insert(K key, V value);
	private:
		Array<Pair> data_;
	};
}

#endif
