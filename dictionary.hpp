//
//  dictionary.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 05/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_dictionary_hpp
#define falling_dictionary_hpp

#include "base/map.hpp"

namespace falling {
	template <typename Value, typename Cmp = Less>
	class Dictionary {
	public:
		using Self = Dictionary<Value,Cmp>;
		using mapped_type = Value;
		using key_type = StringRef;
		Dictionary(IAllocator& alloc = default_allocator());
		Dictionary(const Self&, IAllocator& alloc);
		Dictionary(const Self&);
		Dictionary(Self&& other);
		Dictionary(ArrayRef<StringRef> keys, ArrayRef<Value> values, IAllocator& alloc = default_allocator());
		Dictionary(std::initializer_list<Pair<StringRef, Value>> list, IAllocator& alloc = default_allocator());
		~Dictionary() { clear(true); }
		
		IAllocator& allocator() const { return map_.allocator(); }
		size_t size() const { return map_.size(); }
		size_t capacity() const { return map_.capacity(); }
		void clear(bool free_memory = true);
		void swap(Self& other);
		void reserve(size_t n) { map_.reserve(n); }
		
		ArrayRef<const StringRef> keys() const { return map_.keys(); }
		ArrayRef<Value> values() { return map_.values(); }
		ArrayRef<const Value> values() const { return map_.values(); }
		
		template <typename Cmp_ = Cmp>
		Self& operator=(const Dictionary<Value,Cmp_>& other) { map_ = other.map_; return *this; }
		Self& operator=(const Self& other) { map_ = other.map_; return *this; }
		Self& operator=(Self&& other) { map_ = move(other.map_); return *this; }
		
		bool operator==(const Self& other) const { return map_ == other.map_; }
		bool operator!=(const Self& other) const { return map_ != other.map_; }
		
		template <typename ComparableAndConvertibleKey = StringRef>
		Value& operator[](const ComparableAndConvertibleKey& key);
		template <typename ComparableKey = StringRef>
		Maybe<const Value&> operator[](const ComparableKey& key) const { return map_[key]; }
		
		using InternalMapType = Map<StringRef, Value, Cmp>;
		using iterator = typename InternalMapType::iterator;
		using const_iterator = typename InternalMapType::const_iterator;
		iterator begin() { return map_.begin(); }
		iterator end() { return map_.end(); }
		const_iterator begin() const { return map_.begin(); }
		const_iterator end() const { return map_.end(); }
		
		iterator erase(iterator it);
		template <typename ComparableKey>
		iterator erase(const ComparableKey& key);
		
		template <typename InputIterator>
		void insert(InputIterator a, InputIterator b);
		template <typename InputIterator>
		iterator insert_move(InputIterator a, InputIterator b);
		template <typename ComparableAndConvertibleKey = StringRef>
		iterator set(const ComparableAndConvertibleKey& key, Value value);
		
		template <typename ComparableKey = StringRef>
		iterator find(const ComparableKey& key) { return map_.find(key); }
		template <typename ComparableKey = StringRef>
		const_iterator find(const ComparableKey& key) const { return map_.find(key); }
		template <typename ComparableKey = StringRef>
		size_t count(const ComparableKey& key) const { return map_.count(key); }
		template <typename ComparableKey = StringRef>
		iterator at(const ComparableKey& key) { return find(key); }
		template <typename ComparableKey = StringRef>
		const_iterator at(const ComparableKey& key) { return find(key); }
	private:
		Map<StringRef, Value, Cmp> map_;
		
		template <typename ComparableAndConvertibleKey>
		iterator insert_one(const ComparableAndConvertibleKey& key, Value v);
	};
	
	template <typename V, typename C>
	Dictionary<V,C>::Dictionary(IAllocator& alloc) : map_(alloc) {}
	
	template <typename V, typename C>
	Dictionary<V,C>::Dictionary(const Self& other, IAllocator& alloc) : map_(alloc) {
		insert(other.begin(), other.end());
	}
	
	template <typename V, typename C>
	Dictionary<V,C>::Dictionary(const Self& other) : map_(other.allocator()) {
		insert(other.begin(), other.end());
	}
	
	template <typename V, typename C>
	Dictionary<V,C>::Dictionary(Self&& other) : map_(move(other.map_)) {}
	
	template <typename V, typename C>
	Dictionary<V,C>::Dictionary(ArrayRef<StringRef> keys, ArrayRef<V> values, IAllocator& alloc) : map_(alloc) {
		size_t n = std::min(keys.size(), values.size());
		map_.reserve(n);
		for (size_t i = 0; i < n; ++i) {
			insert_one(keys[i], values[i]);
		}
	}
	
	template <typename V, typename C>
	Dictionary<V,C>::Dictionary(std::initializer_list<Pair<StringRef,V>> list, IAllocator& alloc) : map_(alloc) {
		insert(list.begin(), list.end());
	}
	
	template <typename V, typename C>
	void Dictionary<V,C>::clear(bool free_memory) {
		for (auto& k: keys()) {
			allocator().free((void*)k.data(), k.size());
		}
		map_.clear(free_memory);
	}
	
	template <typename V, typename C>
	void Dictionary<V,C>::swap(Self& other) {
		map_.swap(other.map_);
	}
	
	template <typename V, typename C>
	typename Dictionary<V,C>::iterator Dictionary<V,C>::erase(iterator it) {
		allocator().free(it->first.data(), it->first.size());
		return map_.erase(it);
	}
	
	template <typename V, typename C>
	template <typename ComparableKey>
	typename Dictionary<V,C>::iterator Dictionary<V,C>::erase(const ComparableKey& key) {
		auto it = find(key);
		if (it != end()) {
			return erase(it);
		}
		return it;
	}
	
	template <typename V, typename C>
	template <typename InputIterator>
	void Dictionary<V,C>::insert(InputIterator a, InputIterator b) {
		auto n = iterator_distance_if_supported(a, b);
		if (n != SIZE_MAX) {
			reserve(size() + n);
		}
		for (auto it = a; it != b; ++it) {
			insert_one(it->first, it->second);
		}
	}
	
	template <typename V, typename C>
	template <typename InputIterator>
	typename Dictionary<V,C>::iterator Dictionary<V,C>::insert_move(InputIterator a, InputIterator b) {
		auto n = iterator_distance_if_supported(a, b);
		if (n != SIZE_MAX) {
			reserve(size() + n);
		}
		for (auto it = a; it != b; ++it) {
			insert_one(it->first, move(it->second));
		}
	}
	
	template <typename V, typename C>
	template <typename ComparableAndConvertibleKey>
	typename Dictionary<V,C>::iterator Dictionary<V,C>::set(const ComparableAndConvertibleKey& key, V value) {
		reserve(size() + 1);
		return insert_one(key, move(value));
	}
	
	template <typename V, typename C>
	template <typename ComparableAndConvertibleKey>
	typename Dictionary<V,C>::iterator Dictionary<V,C>::insert_one(const ComparableAndConvertibleKey& key, V v) {
		StringRef k0 = key;
		char* str = (char*)allocator().allocate(k0.size(), 1);
		std::copy(k0.begin(), k0.end(), str);
		StringRef k1(str, k0.size());
		return map_.set(k1, move(v));
	}
	
	template <typename V, typename C>
	template <typename ComparableAndConvertibleKey>
	V& Dictionary<V,C>::operator[](const ComparableAndConvertibleKey& key) {
		auto it = find(key);
		if (it == end()) {
			it = set(key, V());
		}
		return it->second;
	}
}

#endif
