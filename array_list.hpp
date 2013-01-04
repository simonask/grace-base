//
//  array_list.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 21/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_array_list_hpp
#define falling_array_list_hpp

#include "base/basic.hpp"
#include "memory/allocator.hpp"
#include "base/link_list.hpp"
#include "base/iterators.hpp"
#include <initializer_list>
#include <algorithm>

namespace falling {
	struct FormattedStream;

	template <typename T>
    class ArrayList {
    public:
        explicit ArrayList(IAllocator& allocator = default_allocator());
        ArrayList(const ArrayList<T>& other, IAllocator& allocator = default_allocator());
		ArrayList(std::initializer_list<T> init, IAllocator& allocator = default_allocator());
        ArrayList(ArrayList&& other);
        ~ArrayList();
        
        IAllocator& allocator() const;
        
        size_t size() const;
		bool operator==(const ArrayList<T>& other) const;
		bool operator!=(const ArrayList<T>& other) const;
        
        void push_back(const T& x);
        void push_back(T&& x);
        void clear();
        T& front();
        const T& front() const;
        T& back();
        const T& back() const;
        
        T& operator[](size_t idx);
        const T& operator[](size_t idx) const;
        
        template <bool IsConst> struct iterator_impl;
        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;
        
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        
        template <typename InputIterator>
        void insert(InputIterator i0, InputIterator i1);
        template <typename InputIterator>
        void insert(InputIterator i0, InputIterator i1, const iterator& before);
		template <typename InputIterator>
        void insert_move(InputIterator i0, InputIterator i1);
        template <typename InputIterator>
        void insert_move(InputIterator i0, InputIterator i1, const iterator& before);
        void erase(iterator it);
		void resize(size_t new_size, T filler = T());
    private:
        template <bool> friend struct iterator_impl;
        
        IAllocator& allocator_;
        
        struct Block : ListLinkBase<Block> {
            T* begin;
            T* end;
            T* current;
			
			void extend_current_by(size_t n) {
				ASSERT(current + n <= end);
				construct_range(current, current + n);
				current += n;
			}
			
			void extend_current_until_end() {
				construct_range(current, end);
				current = end;
			}
        };
        
        BareLinkList<Block> blocks_;
        size_t size_ = 0;
		
		using BlockIterator = typename BareLinkList<Block>::iterator;
        
        Block* create_and_append_block();
		Block* create_block_of_size(size_t n_elements);
		void delete_and_remove_block(BlockIterator b);
		iterator make_room_at(const iterator& at, size_t n);
		iterator fill_and_spill(BlockIterator fill, size_t n, const iterator& before);
    };
    
    template <typename T>
    ArrayList<T>::ArrayList(IAllocator& alloc) : allocator_(alloc) {}
	
	template <typename T>
	ArrayList<T>::ArrayList(const ArrayList<T>& other, IAllocator& alloc) : allocator_(alloc) {
		for (auto& it: other) {
			push_back(it);
		}
	}
	
	template <typename T>
	ArrayList<T>::ArrayList(ArrayList<T>&& other) : allocator_(other.allocator_), blocks_(std::move(other.blocks_)), size_(other.size_) {}
	
	template <typename T>
	ArrayList<T>::ArrayList(std::initializer_list<T> init, IAllocator& alloc) : allocator_(alloc) {
		for (auto& it: init) {
			push_back(it);
		}
	}
    
    template <typename T>
    ArrayList<T>::~ArrayList() {
        clear();
    }
    
    template <typename T>
    IAllocator& ArrayList<T>::allocator() const {
        return allocator_;
    }
    
    template <typename T>
    size_t ArrayList<T>::size() const {
        return size_;
    }
	
	template <typename T>
	bool ArrayList<T>::operator==(const ArrayList<T>& other) const {
		if (size_ == other.size_) {
			auto it0 = begin();
			auto it1 = other.begin();
			for (; it0 != end() && it1 != end(); ++it0, ++it1) {
				if (*it0 != *it1) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	
	template <typename T>
	bool ArrayList<T>::operator!=(const ArrayList<T>& other) const {
		return !(*this == other);
	}
    
    template <typename T>
    void ArrayList<T>::push_back(const T& object) {
		insert(&object, &object + 1);
    }
	
	template <typename T>
	void ArrayList<T>::push_back(T&& object) {
		insert_move(&object, &object + 1);
	}
	
	template <typename T>
	template <typename InputIterator>
	void ArrayList<T>::insert(InputIterator a, InputIterator b) {
		insert(a, b, end());
	}
	
	template <typename T>
	template <typename InputIterator>
	void ArrayList<T>::insert_move(InputIterator a, InputIterator b) {
		insert_move(a, b, end());
	}
	
	template <typename T>
	typename ArrayList<T>::iterator ArrayList<T>::fill_and_spill(BlockIterator fill, size_t n, const iterator& before) {
		if (fill != blocks_.end()) {
			size_t fill_space = fill->end - fill->current;
			if (fill_space > 0) {
				auto it = iterator(*this, fill, fill->current, before.position_);
				if (fill_space >= n) {
					fill->extend_current_by(n);
				} else {
					fill->extend_current_until_end();
					size_t remaining = n - fill_space;
					Block* spill = create_block_of_size(remaining);
					blocks_.link_after(spill, fill);
					spill->extend_current_by(remaining);
				}
				return move(it);
			}
		}
		
		Block* spill = create_block_of_size(n);
		spill->extend_current_by(n);
		BlockIterator spill_it;
		if (fill != blocks_.end()) {
			spill_it = blocks_.link_after(spill, fill);
		} else {
			spill_it = blocks_.link_tail(spill);
		}
		return iterator(*this, spill_it, spill_it->begin, before.position_);
	}
	
	template <typename T>
	typename ArrayList<T>::iterator ArrayList<T>::make_room_at(const iterator& before, size_t n) {
		size_ += n;
		if (before == end()) {
			return fill_and_spill(blocks_.last(), n, before);
		} else  if (before == begin()) {
			// prepend block of size n
			Block* b = create_block_of_size(n);
			b->extend_current_by(n);
			auto it = blocks_.link_head(b);
			return iterator(*this, it, it->begin, before.position_);
		} else if (before.current_ == before.block_->begin) {
			// at block boundary, insert block before
			auto prev_block = before.block_ - 1;
			if (prev_block != blocks_.end()) {
				return fill_and_spill(prev_block, n, before);
			}
			
			Block* b = create_block_of_size(n);
			b->extend_current_by(n);
			auto it = blocks_.link_before(b, before.block_);
			return iterator(*this, it, it->begin, before.position_);
		} else if (before.current_ == before.block_->current) {
			// at block boundary:
			// fill current block if it has room
			// create new block for remaining elements
			return fill_and_spill(before.block_, n, before);
		} else {
			// split block
			BlockIterator b = before.block_;
			size_t elements_to_move = b->current - before.current_;
			size_t block_space = b->end - b->current;
			if (block_space >= n) {
				b->extend_current_by(n);
				// no more space required
			} else {
				b->extend_current_until_end();
				size_t remaining = n - block_space;
				Block* nb = create_block_of_size(remaining);
				nb->extend_current_by(remaining);
				blocks_.link_after(nb, b);
			}
			
			auto old_elements_begin = before + elements_to_move;
			std::move(before, old_elements_begin, old_elements_begin);
			return before;
		}
	}
	
	template <typename T>
	template <typename InputIterator>
	void ArrayList<T>::insert(InputIterator a, InputIterator b, const iterator& before) {
		ssize_t n = b - a;
		if (n != 0) {
			iterator i0 = make_room_at(before, n);
			std::copy(a, b, i0);
		}
	}
	
	template <typename T>
	template <typename InputIterator>
	void ArrayList<T>::insert_move(InputIterator a, InputIterator b, const iterator& before) {
		ssize_t n = b - a;
		if (n != 0) {
			iterator i0 = make_room_at(before, n);
			std::move(a, b, i0);
		}
	}
    
    template <typename T>
    void ArrayList<T>::clear() {
        for (auto it = blocks_.begin(); it != blocks_.end();) {
            Block* b = it.get();
            it = blocks_.erase(it);
            size_t block_size = (byte*)b->end - (byte*)b;
            allocator_.free_large(b, block_size);
        }
        ASSERT(blocks_.empty());
    }
	
	template <typename T>
	void ArrayList<T>::resize(size_t new_size, T filler) {
		if (new_size > size_) {
			while (size_ < new_size) {
				push_back(filler);
			}
		} else {
			while (size_ > new_size) {
				auto it = end();
				--it;
				erase(it);
			}
		}
	}
	
	template <typename T>
	T& ArrayList<T>::operator[](size_t idx) {
		size_t i = idx;
		for (const auto& b: blocks_) {
			size_t sz = b.current - b.begin;
			if (i < sz) {
				return b.begin[i];
			} else {
				i -= sz;
			}
		}
		throw IndexOutOfBoundsException();
	}
	
	template <typename T>
	const T& ArrayList<T>::operator[](size_t idx) const {
		size_t i = idx;
		for (const auto& b: blocks_) {
			size_t sz = b.current - b.begin;
			if (i < sz) {
				return b.begin[i];
			} else {
				i -= sz;
			}
		}
		throw IndexOutOfBoundsException();
	}
	
	template <typename T>
	void ArrayList<T>::erase(iterator it) {
		if (it == end()) {
			throw IndexOutOfBoundsException();
		}
		auto output_begin = it;
		auto input_begin = it+1;
		auto input_end = end();
		std::copy(input_begin, input_end, output_begin);
		if (size_) {
			auto last = end()-1;
			last.get()->~T();
			BlockIterator b = blocks_.last();
			--b->current;
			if (b->current == b->begin) {
				delete_and_remove_block(b);
			}
		}
		--size_;
	}
	
	template <typename T>
	typename ArrayList<T>::Block* ArrayList<T>::create_block_of_size(size_t n_elements) {
		size_t actual_allocation_size;
		byte* memory = (byte*)allocator_.allocate_large(sizeof(Block) + sizeof(T) * n_elements, alignof(Block), actual_allocation_size);
		Block* b = new(memory) Block;
		
		byte* begin = memory + sizeof(Block);
        intptr_t ibegin = reinterpret_cast<intptr_t>(begin);
        intptr_t adjust = (ibegin % alignof(T)) & (alignof(T)-1);
        begin += adjust;
        b->begin = reinterpret_cast<T*>(begin);
        b->end = reinterpret_cast<T*>(memory + actual_allocation_size);
        b->current = b->begin;
		
		return b;
	}
    
    template <typename T>
    typename ArrayList<T>::Block* ArrayList<T>::create_and_append_block() {
		size_t n_elements = (4096 - sizeof(Block)) / sizeof(T);
		Block* b = create_block_of_size(n_elements);
		blocks_.link_tail(b);
        return b;
    }
	
	template <typename T>
	void ArrayList<T>::delete_and_remove_block(BlockIterator b) {
		ASSERT(b->current != b->begin); // Block has live objects.
		Block* ptr = b.get();
		blocks_.erase(b);
		allocator_.free_large(ptr, (byte*)ptr->end - (byte*)ptr);
	}
    
    template <typename T>
    template <bool IsConst>
    struct ArrayList<T>::iterator_impl {
    public:
        using Self = iterator_impl<IsConst>;
        using Owner = typename std::conditional<IsConst, const ArrayList<T>, ArrayList<T>>::type;
        using ValueType = typename std::conditional<IsConst, const T, T>::type;
        using value_type = ValueType;
        using Block = typename Owner::Block;
        using BlockList = BareLinkList<Block>;
        using BlockListIterator = typename std::conditional<IsConst, typename BlockList::const_iterator, typename BlockList::iterator>::type;
        
        ValueType& operator*() const {
            return *current_;
        }
        
        ValueType* operator->() const {
            return current_;
        }
		
		ValueType* get() const {
			return current_;
		}
        
        Self& operator++() {
			inc_by(1);
            return *this;
        }
		
		Self& operator--() {
			dec_by(1);
			return *this;
		}
        
        Self operator++(int) {
            Self s = *this;
            inc_by(1);
            return s;
        }
		
		Self operator+(ptrdiff_t n) const {
			Self s = *this;
			if (n >= 0)
				s.inc_by(n);
			else
				s.dec_by(-n);
			return s;
		}
		
		Self operator-(ptrdiff_t n) const {
			return this->operator+(-n);
		}
		
		Self& operator+=(ptrdiff_t n) {
			if (n >= 0)
				inc_by(n);
			else
				dec_by(-n);
			return *this;
		}
		
		Self& operator-=(ptrdiff_t n) {
			return this->operator+=(-n);
		}
        
		template <bool IsConst_>
		ssize_t operator-(const iterator_impl<IsConst_>& other) const {
			ASSERT(owner_ == other.owner_);
			return (ssize_t)position_ - (ssize_t)other.position_;
		}
        
        bool operator==(const iterator_impl<true>& other) const {
            return owner_ == other.owner_ && block_ == other.block_ && current_ == other.current_;
        }
        bool operator!=(const iterator_impl<true>& other) const {
            return !(*this == other);
        }
        
		iterator_impl() : owner_(nullptr), current_(nullptr) {}
        iterator_impl(const iterator_impl<false>& other) : owner_(other.owner_), block_(other.block_), current_(other.current_), position_(other.position_) {}
        template <bool IsConst_ = IsConst>
        iterator_impl(const typename std::enable_if<IsConst_, iterator_impl<true>>::type& other) : owner_(other.owner_), block_(other.block_), current_(other.current_), position_(other.position_) {}
        Self& operator=(const iterator_impl<false>& other) {
            owner_ = other.owner_;
            block_ = other.block_;
            current_ = other.current_;
			position_ = other.position_;
            return *this;
        }
        template <bool IsConst_ = IsConst>
        Self& operator=(const typename std::enable_if<IsConst_, iterator_impl<true>>::type& other) {
            owner_ = other.owner_;
            block_ = other.block_;
            current_ = other.current_;
			position_ = other.position_;
            return *this;
        }
    private:
        iterator_impl(Owner& owner, BlockListIterator b, ValueType* c, size_t position) : owner_(&owner), block_(b), current_(c), position_(position) {}
        friend class ArrayList<T>;
        friend struct iterator_impl<!IsConst>;
        Owner* owner_;
        BlockListIterator block_;
        ValueType* current_;
		size_t position_ = 0;
		
		void inc_by(size_t n) {
			size_t remaining = n;
			while (remaining > 0) {
				size_t current_to_end = block_->current - current_;
				if (remaining < current_to_end) {
					current_ += remaining;
					remaining = 0;
				} else {
					remaining -= current_to_end;
					++block_;
					if (block_ != owner_->blocks_.end()) {
						current_ = block_->begin;
					} else {
						current_ = nullptr;
						break;
					}
				}
			}
			if (remaining > 0) {
				throw IndexOutOfBoundsException();
			}
			position_ += n;
		}
		
		void dec_by(size_t n) {
			if (n && block_ == owner_->blocks_.end()) {
				--block_;
				if (block_ == owner_->blocks_.end()) {
					throw IndexOutOfBoundsException();
				}
				current_ = block_->current;
			}
			
			size_t remaining = n;
			while (remaining > 0) {
				size_t begin_to_current = current_ - block_->begin;
				if (remaining < begin_to_current) {
					current_ -= remaining;
					remaining = 0;
				} else {
					remaining -= begin_to_current;
					--block_;
					if (block_ != owner_->blocks_.end()) {
						current_ = block_->current;
					} else {
						current_ = nullptr;
						break;
					}
				}
			}
			if (remaining > 0) {
				throw IndexOutOfBoundsException();
			}
			position_ -= n;
		}
    };
    
    template <typename T>
    typename ArrayList<T>::iterator ArrayList<T>::begin() {
        return iterator(*this, blocks_.begin(), blocks_.empty() ? nullptr : blocks_.head()->begin, 0);
    }
    
    template <typename T>
    typename ArrayList<T>::iterator ArrayList<T>::end() {
        return iterator(*this, blocks_.end(), nullptr, size());
    }
    
    template <typename T>
    typename ArrayList<T>::const_iterator ArrayList<T>::begin() const {
        return const_iterator(*this, blocks_.begin(), blocks_.empty() ? nullptr : blocks_.head()->begin, 0);
    }
    
    template <typename T>
    typename ArrayList<T>::const_iterator ArrayList<T>::end() const {
        return const_iterator(*this, blocks_.end(), nullptr, size());
    }
}

#endif
