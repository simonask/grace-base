//
//  map.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 10/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_map_hpp
#define grace_map_hpp

#include "memory/allocator.hpp"
#include "base/basic.hpp"
#include "base/iterators.hpp"
#include "base/array_ref.hpp"
#include "base/maybe.hpp"
#include "base/pair.hpp"
#include "base/array_utils.hpp"
#include "base/string.hpp" // for definition of Dictionary<T>

#include <algorithm>

namespace grace {
	template <typename Key, typename Value, bool IsConst>
	struct MapIteratorImpl;

	template <typename Key, typename Value, typename Cmp = Less>
	class Map {
	public:
		using Self = Map<Key,Value,Cmp>;
		using mapped_type = Value;
		using key_type = const Key;
		Map(IAllocator& alloc = default_allocator());
		Map(const Self&, IAllocator& alloc = default_allocator());
		Map(Self&& other);
		Map(ArrayRef<Key> keys, ArrayRef<Value> values, IAllocator& alloc = default_allocator());
		Map(std::initializer_list<Pair<Key, Value>> list, IAllocator& alloc = default_allocator());
		~Map() { clear(true); }
		
		IAllocator& allocator() const;
		size_t size() const;
		size_t capacity() const;
		void clear(bool free_memory = true);
		void swap(Self& other);
		void reserve(size_t new_size); // May do nothing in future versions
		
		ArrayRef<const Key> keys() const;
		ArrayRef<Value> values();
		ArrayRef<const Value> values() const;
		
		template <typename Cmp_ = Cmp>
		Self& operator=(const Map<Key,Value,Cmp_>& other);
		Self& operator=(const Map<Key,Value,Cmp>& other);
		Self& operator=(Self&& other);
		
		bool operator==(const Self& other) const;
		bool operator!=(const Self& other) const;
		
		template <typename ComparableAndConvertibleKey = Key>
		Value& operator[](const ComparableAndConvertibleKey& key);
		template <typename ComparableKey = Key>
		Maybe<const Value&> operator[](const ComparableKey& key) const;
		
		template <bool IsConst>
		using IteratorImpl = MapIteratorImpl<Key,Value,IsConst>;
		using iterator = IteratorImpl<false>;
		using const_iterator = IteratorImpl<true>;
		
		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;
		
		iterator erase(iterator it);
		template <typename ComparableKey>
		iterator erase(const ComparableKey& key);
		
		template <typename InputIterator>
		void insert(InputIterator a, InputIterator b);
		template <typename InputIterator>
		iterator insert_move(InputIterator a, InputIterator b);
		template <typename ComparableAndConvertibleKey = Key>
		iterator set(ComparableAndConvertibleKey key, Value value);
		
		template <typename ComparableKey = Key>
		iterator find(const ComparableKey& key);
		template <typename ComparableKey = Key>
		const_iterator find(const ComparableKey& key) const;
		template <typename ComparableKey = Key>
		size_t count(const ComparableKey& key) const;
		
		template <typename ComparableKey = Key>
		iterator at(const ComparableKey& key) { return find(key); }
		template <typename ComparableKey = Key>
		const_iterator at(const ComparableKey& key) const { return find(key); }
	private:
		IAllocator& allocator_;
		Cmp cmp_;
		uint32 alloc_size_ = 0;
		uint32 size_ = 0;
		Key* keys_ = nullptr;
		Value* values_ = nullptr;
		
		template <typename ComparableAndConvertibleKey>
		iterator insert_one(ComparableAndConvertibleKey key, Value v);
	};
	
	template <typename Key, typename Value, bool IsConst>
	struct MapIteratorImpl {
		using Self = MapIteratorImpl<Key,Value,IsConst>;
		using KeyType = const Key;
		using KeyRefType = KeyType&;
		using ValueType = typename std::conditional<IsConst, const Value, Value>::type;
		using ValueRefType = ValueType&;
		using PairType = Pair<KeyRefType, ValueRefType>;
		using KeyIterator = LinearMemoryIterator<Key, true>;
		using ValueIterator = LinearMemoryIterator<Value, IsConst>;
		
		MapIteratorImpl() {}
		MapIteratorImpl(KeyIterator key_it, ValueIterator value_it) : key_it_(key_it), value_it_(value_it) {}
		MapIteratorImpl(const MapIteratorImpl<Key,Value,false>& other) : key_it_(other.key_it_), value_it_(other.value_it_) {}
		template <bool IsConst_ = IsConst>
		MapIteratorImpl(const typename std::enable_if<IsConst_, Self>::type& other) : key_it_(other.key_it_), value_it_(other.value_it_) {}
		
		Self& operator=(const MapIteratorImpl<Key,Value,false>& other) {
			key_it_ = other.key_it_;
			value_it_ = other.value_it_;
			return *this;
		}
		
		template <bool IsConst_ = IsConst>
		Self& operator=(const typename std::enable_if<IsConst_, Self>::type& other) {
			key_it_ = other.key_it_;
			value_it_ = other.value_it_;
			return *this;
		}
		
		ValueType& value() const { return *value_it_; }
		
		PairType get() const        { return PairType{*key_it_, *value_it_}; }
		PairType operator->() const { return get(); }
		PairType operator*() const  { return get(); }
		
		Self& operator++()      { *this += 1; return *this; }
		Self operator++(int)    { Self copy = *this; (*this)++; return copy; }
		Self& operator--()      { *this -= 1; return *this; }
		Self operator--(int)    { Self copy = *this; (*this)--; return copy; }
		
		template <bool B>
		bool operator==(const MapIteratorImpl<Key,Value,B>& other) const {
			return key_it_ == other.key_it_ && value_it_ == other.value_it_;
		}
		template <bool B>
		bool operator!=(const MapIteratorImpl<Key,Value,B>& other) const {
			return !(*this == other);
		}
		
		// XXX: This is provided as an optimization for insertion, not for general purpose.
		ptrdiff_t operator-(const Self& other) const {
			return key_it_ - other.key_it_;
		}
	private:
		KeyIterator key_it_;
		ValueIterator value_it_;
		friend struct MapIteratorImpl<Key,Value,!IsConst>;
		
		Self& operator+=(int n) { key_it_ += n; value_it_ += n; return *this; }
		Self& operator-=(int n) { key_it_ -= n; value_it_ -= n; return *this; }
	};
	
	template <typename K, typename V, typename C>
	Map<K,V,C>::Map(IAllocator& alloc) : allocator_(alloc) {}
	
	template <typename K, typename V, typename C>
	Map<K,V,C>::Map(const Self& other, IAllocator& alloc) : allocator_(alloc) {
		reserve(other.size_);
		for (size_t i = 0; i < other.size_; ++i) {
			new(keys_+i) K(other.keys_[i]);
			new(values_+i) V(other.values_[i]);
		}
		size_ = other.size_;
	}
	
	template <typename K, typename V, typename C>
	Map<K,V,C>::Map(Self&& other) : allocator_(other.allocator_) {
		alloc_size_ = other.alloc_size_;
		size_ = other.size_;
		keys_ = other.keys_;
		values_ = other.values_;
		cmp_ = move(other.cmp_);
		other.size_ = 0;
		other.alloc_size_ = 0;
		other.keys_ = nullptr;
		other.values_ = nullptr;
	}
	
	template <typename K, typename V, typename C>
	Map<K,V,C>::Map(ArrayRef<K> keys, ArrayRef<V> values, IAllocator& alloc) : allocator_(alloc) {
		size_t n = std::min(keys.size(), values.size());
		reserve(n);
		for (size_t i = 0; i < n; ++i) {
			insert_one(keys[i], values[i]);
		}
	}
	
	template <typename K, typename V, typename C>
	Map<K,V,C>::Map(std::initializer_list<Pair<K,V>> list, IAllocator& alloc) : allocator_(alloc) {
		insert(list.begin(), list.end());
	}
	
	template <typename K, typename V, typename C>
	IAllocator& Map<K,V,C>::allocator() const {
		return allocator_;
	}
	
	template <typename K, typename V, typename C>
	size_t Map<K,V,C>::size() const {
		return size_;
	}
	
	template <typename K, typename V, typename C>
	size_t Map<K,V,C>::capacity() const {
		return alloc_size_;
	}
	
	template <typename K, typename V, typename C>
	void Map<K,V,C>::clear(bool free_memory) {
		destruct_range(keys_, keys_+size_);
		destruct_range(values_, values_+size_);
		size_ = 0;
		if (free_memory) {
			allocator_.free(keys_, sizeof(K) * alloc_size_);
			allocator_.free(values_, sizeof(V) * alloc_size_);
			keys_ = nullptr;
			values_ = nullptr;
			alloc_size_ = 0;
		}
	}
	
	template <typename K, typename V, typename C>
	void Map<K,V,C>::swap(Self& other) {
		if (&allocator_ == &other.allocator_) {
			// Fast swap
			std::swap(keys_, other.keys_);
			std::swap(values_, other.values_);
			std::swap(size_, other.size_);
			std::swap(alloc_size_, other.alloc_size_);
			std::swap(cmp_, other.cmp_);
		} else {
			// Slower swap
			Self tmp = move(other);
			other = move(*this);
			*this = move(tmp);
		}
	}
	
	template <typename K, typename V, typename C>
	ArrayRef<const K> Map<K,V,C>::keys() const {
		return ArrayRef<const K>(keys_, keys_ + size_);
	}
	
	template <typename K, typename V, typename C>
	ArrayRef<const V> Map<K,V,C>::values() const {
		return ArrayRef<const V>(values_, values_ + size_);
	}
	
	template <typename K, typename V, typename C>
	ArrayRef<V> Map<K,V,C>::values() {
		return ArrayRef<V>(values_, values_ + size_);
	}
	
	template <typename K, typename V, typename C>
	template <typename Cmp_>
	Map<K,V,C>& Map<K,V,C>::operator=(const Map<K,V,Cmp_>& other) {
		clear(false);
		insert(other.begin(), other.end());
		return *this;
	}
	
	template <typename K, typename V, typename C>
	Map<K,V,C>& Map<K,V,C>::operator=(const Map<K,V,C>& other) {
		clear(false);
		insert(other.begin(), other.end());
		return *this;
	}
	
	template <typename K, typename V, typename C>
	Map<K,V,C>& Map<K,V,C>::operator=(Self&& other) {
		clear(true);
		if (&allocator_ == &other.allocator_) {
			alloc_size_ = other.alloc_size_;
			size_ = other.size_;
			keys_ = other.keys_;
			values_ = other.values_;
			cmp_ = move(other.cmp_);
			other.alloc_size_ = 0;
			other.size_ = 0;
			other.keys_ = nullptr;
			other.values_ = nullptr;
		} else {
			insert(other.begin(), other.end());
			other.clear();
		}
		return *this;
	}
	
	template <typename K, typename V, typename C>
	bool Map<K,V,C>::operator==(const Self& other) const {
		if (size() == other.size()) {
			auto it0 = begin();
			auto it1 = other.begin();
			for (; it0 != end(); ++it0, ++it1) {
				if (*it0 != *it1) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	
	template <typename K, typename V, typename C>
	bool Map<K,V,C>::operator!=(const Self& other) const {
		return !(*this == other);
	}
	
	template <typename K, typename V, typename C>
	template <typename ComparableAndConvertibleKey>
	V& Map<K,V,C>::operator[](const ComparableAndConvertibleKey& key) {
		auto found = find(key);
		if (found == end()) {
			found = set(key, V());
		}
		return found.value();
	}
	
	template <typename K, typename V, typename C>
	template <typename ComparableKey>
	Maybe<const V&> Map<K,V,C>::operator[](const ComparableKey& key) const {
		auto found = find(key);
		if (found == end()) {
			return Nothing;
		}
		return found->second;
	}
	
	template <typename K, typename V, typename C>
	typename Map<K,V,C>::iterator Map<K,V,C>::begin() {
		return iterator(keys_, values_);
	}
	
	template <typename K, typename V, typename C>
	typename Map<K,V,C>::const_iterator Map<K,V,C>::begin() const {
		return const_iterator(keys_, values_);
	}
	
	template <typename K, typename V, typename C>
	typename Map<K,V,C>::iterator Map<K,V,C>::end() {
		return iterator(keys_ + size_, values_ + size_);
	}
	
	template <typename K, typename V, typename C>
	typename Map<K,V,C>::const_iterator Map<K,V,C>::end() const {
		return const_iterator(keys_ + size_, values_ + size_);
	}
	
	template <typename InputIterator> struct GetIteratorDistanceTypeIfSupported;
	template <typename InputIterator> struct GetIteratorDistanceTypeIfSupported {
		using DiffType = decltype(InputIterator() - InputIterator());
	};
	
	
	template <typename IteratorCategory, typename Iterator> struct GetIteratorDistanceIfSupported;
	template <typename Iterator> struct GetIteratorDistanceIfSupported<std::random_access_iterator_tag, Iterator> {
		using DiffType = typename std::iterator_traits<Iterator>::difference_type;
		static DiffType distance(Iterator a, Iterator b) {
			return b - a;
		}
	};
	template <typename IteratorCategory, typename Iterator> struct GetIteratorDistanceIfSupported {
		using DiffType = size_t;
		static DiffType distance(Iterator a, Iterator b) {
			return SIZE_MAX;
		}
	};
	
	template <typename AnyIterator>
	auto iterator_distance_if_supported(AnyIterator a, AnyIterator b)
	-> typename GetIteratorDistanceIfSupported<typename std::iterator_traits<AnyIterator>::iterator_category, AnyIterator>::DiffType
	{
		return GetIteratorDistanceIfSupported<typename std::iterator_traits<AnyIterator>::iterator_category, AnyIterator>::distance(a, b);
	}
	
	template <typename K, typename V, typename C>
	template <typename InputIterator>
	void Map<K,V,C>::insert(InputIterator a, InputIterator b) {
		auto n = iterator_distance_if_supported(a, b);
		if (n != SIZE_MAX) {
			reserve(size_ + n); // May overallocate because keys in a...b may already exist in the map.
		}
		for (auto it = a; it != b; ++it) {
			insert_one(it->first, it->second);
		}
	}
	
	template <typename K, typename V, typename C>
	template <typename InputIterator>
	typename Map<K,V,C>::iterator Map<K,V,C>::insert_move(InputIterator a, InputIterator b) {
		size_t n = b - a;
		reserve(size_ + n); // May overallocate because keys in a...b may already exist in the map.
		for (auto it = a; it != b; ++it) {
			insert_one(move(it->first), move(it->second));
		}
	}

	
	template <typename K, typename V, typename C>
	template <typename ComparableAndConvertibleKey>
	typename Map<K,V,C>::iterator Map<K,V,C>::set(ComparableAndConvertibleKey key, V value) {
		reserve(size_ + 1);
		return insert_one(move(key), move(value));
	}
	
	template <typename K, typename V, typename C>
	template <typename ComparableAndConvertibleKey>
	typename Map<K,V,C>::iterator Map<K,V,C>::insert_one(ComparableAndConvertibleKey key, V value) {
		ASSERT(alloc_size_ >= size_+1);
		K* insert_at = std::lower_bound(keys_, keys_ + size_, key, cmp_);
		ptrdiff_t insert_idx = insert_at - keys_;
		if (insert_idx == size_) {
			// key is max, so construct it as a new element at the end
			auto kp = new(keys_ + insert_idx) K(move(key));
			auto vp = new(values_ + insert_idx) V(move(value));
			++size_;
			return iterator(kp, vp);
		} else {
			if (*insert_at == key) {
				// key already exists in map, just assign value
				values_[insert_idx] = move(value);
			} else {
				// construct an empty element at the end
				new(keys_   + size_) K;
				new(values_ + size_) V;
				
				// shift existing values by one
				std::move_backward(keys_ + insert_idx,   keys_   + size_, keys_   + size_ + 1);
				std::move_backward(values_ + insert_idx, values_ + size_, values_ + size_ + 1);
				++size_;
				
				// assign new key and value
				keys_[insert_idx] = move(key);
				values_[insert_idx] = move(value);
			}
			return iterator(keys_ + insert_idx, values_ + insert_idx);
		}
	}
	
	template <typename K, typename V, typename C>
	template <typename ComparableKey>
	typename Map<K,V,C>::iterator Map<K,V,C>::find(const ComparableKey& key) {
		const K* found = std::lower_bound(keys_, keys_ + size_, key, cmp_);
		if (found == keys_ + size_ || *found != key) {
			return end();
		} else {
			size_t idx = found - keys_;
			return iterator(found, values_ + idx);
		}
	}
	
	template <typename K, typename V, typename C>
	template <typename ComparableKey>
	typename Map<K,V,C>::const_iterator Map<K,V,C>::find(const ComparableKey& key) const {
		const K* found = std::lower_bound(keys_, keys_ + size_, key, cmp_);
		if (found == keys_ + size_ || *found != key) {
			return end();
		} else {
			size_t idx = found - keys_;
			return iterator(found, values_ + idx);
		}
	}
	
	template <typename K, typename V, typename C>
	template <typename ComparableKey>
	size_t Map<K,V,C>::count(const ComparableKey& key) const {
		auto it = find(key);
		size_t n = 0;
		while (it != end()) {
			++it;
			++n;
		}
		return n;
	}
	
	template <typename K, typename V, typename C>
	void Map<K,V,C>::reserve(size_t new_size) {
		size_t k_alloc_size = alloc_size_;
		size_t v_alloc_size = alloc_size_;
		keys_ = grace::resize_allocation<K>(allocator_, keys_, &k_alloc_size, size_, new_size, 3, 2);
		values_ = grace::resize_allocation(allocator_, values_, &v_alloc_size, size_, new_size, 3, 2);
		ASSERT(k_alloc_size == v_alloc_size);
		ASSERT(k_alloc_size < UINT32_MAX);
		alloc_size_ = (uint32)k_alloc_size;
	}
	
	template <typename K, typename V, typename C>
	template <typename ComparableKey>
	typename Map<K,V,C>::iterator Map<K,V,C>::erase(const ComparableKey& key) {
		auto it = find(key);
		if (it != end()) {
			return erase(it);
		}
		return end();
	}
	
	template <typename K, typename V, typename C>
	typename Map<K,V,C>::iterator Map<K,V,C>::erase(iterator it) {
		K* k = const_cast<K*>(&it->first);
		V* v = &it->second;
		ASSERT(k >= keys_ && k < keys_ + size_); // iterator from another map!
		ASSERT(v >= values_ && v < values_ + size_); // iterator from another map!
		size_t idx = k - keys_;
		std::move(k + 1, keys_ + size_, k);
		std::move(v + 1, values_ + size_, v);
		--size_;
		keys_[size_].~K();
		values_[size_].~V();
		return iterator(keys_ + idx, values_ + idx);
	}
}

namespace std {
	template <typename Key, typename Value, bool IsConst>
	struct iterator_traits<grace::MapIteratorImpl<Key, Value, IsConst>> {
		using difference_type = ptrdiff_t;
		using value_type = typename grace::MapIteratorImpl<Key, Value, IsConst>::PairType;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::random_access_iterator_tag;
	};
}

#endif
