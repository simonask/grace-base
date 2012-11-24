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
#include "base/intrusive_list.hpp"

namespace falling {
	template <typename T>
    class ArrayList {
    public:
        explicit ArrayList(IAllocator& allocator = default_allocator());
        ArrayList(IAllocator& allocator, const ArrayList<T>& other);
        ArrayList(const ArrayList& other);
        ArrayList(ArrayList&& other);
        ~ArrayList();
        
        IAllocator& allocator() const;
        
        size_t size() const;
        
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
        void insert(InputIterator i0, InputIterator i1, iterator before);
        iterator erase(iterator it);
    private:
        template <bool> friend struct iterator_impl;
        
        IAllocator& allocator_;
        
        struct Block {
            Block* next;
            T* begin;
            T* end;
            T* current;
        };
        
        Block* head_ = nullptr;
        Block* tail_ = nullptr;
        size_t size_ = 0;
        
        Block* create_and_append_block();
    };
    
    template <typename T>
    ArrayList<T>::ArrayList(IAllocator& alloc) : allocator_(alloc) {}
    
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
    void ArrayList<T>::push_back(const T& object) {
        Block* b = tail_;
        if (b == nullptr || b->end - b->current == 0) {
            b = create_and_append_block();
        }
        
        ASSERT(b->end - b->current > 0);
        T* ptr = b->current++;
        ++size_;
        new(ptr) T(object);
    }
    
    template <typename T>
    void ArrayList<T>::clear() {
        Block* b = head_;
        while (b) {
            Block* tmp = b->next;
            for (T* p = b->begin; p < b->current; ++p) {
                p->~T();
            }
            size_t block_size = (byte*)b->end - (byte*)b;
            allocator_.free_large(b, block_size);
            b = tmp;
        }
        head_ = nullptr;
        tail_ = nullptr;
    }
    
    template <typename T>
    typename ArrayList<T>::Block* ArrayList<T>::create_and_append_block() {
        size_t actual_allocation_size;
        byte* memory = (byte*)allocator_.allocate_large(4096, alignof(Block), actual_allocation_size);
        Block* b = new(memory) Block;
        
        byte* begin = memory + sizeof(Block);
        intptr_t ibegin = reinterpret_cast<intptr_t>(begin);
        intptr_t adjust = (ibegin % alignof(T)) & (alignof(T)-1);
        begin += adjust;
        b->begin = reinterpret_cast<T*>(begin);
        b->end = reinterpret_cast<T*>(memory + actual_allocation_size);
        b->current = b->begin;
        b->next = nullptr;
        
        if (head_ == nullptr) {
            head_ = b;
        }
        if (tail_ != nullptr) {
            tail_->next = b;
        }
        tail_ = b;
        
        return b;
    }
    
    template <typename T>
    template <bool IsConst>
    struct ArrayList<T>::iterator_impl {
    public:
        using Self = iterator_impl<IsConst>;
        using Owner = typename std::conditional<IsConst, const ArrayList<T>, ArrayList<T>>::type;
        using ValueType = typename std::conditional<IsConst, const T, T>::type;
        using value_type = ValueType;
        
        ValueType& operator*() const {
            return *current_;
        }
        
        ValueType* operator->() const {
            return current_;
        }
        
        Self& operator++() {
            ++current_;
            if (current_ >= block_->current) {
                block_ = block_->next;
                if (block_) {
                    current_ = block_->begin;
                } else {
                    current_ = nullptr;
                }
            }
            return *this;
        }
        
        Self operator++(int) {
            Self s = *this;
            ++s;
            return s;
        }
        
        // TODO: Reverse iterators?
        
        bool operator==(const iterator_impl<true>& other) const {
            return owner_ == other.owner_ && block_ == other.block_ && current_ == other.current_;
        }
        bool operator!=(const iterator_impl<true>& other) const {
            return !(*this == other);
        }
        
        iterator_impl(const iterator_impl<false>& other) : owner_(other.owner_), block_(other.block_), current_(other.current_) {}
        template <bool IsConst_ = IsConst>
        iterator_impl(const typename std::enable_if<IsConst_, iterator_impl<true>>::type& other) : owner_(other.owner_), block_(other.block_), current_(other.current_) {}
        Self& operator=(const iterator_impl<false>& other) {
            owner_ = other.owner_;
            block_ = other.block_;
            current_ = other.current_;
            return *this;
        }
        template <bool IsConst_ = IsConst>
        Self& operator=(const typename std::enable_if<IsConst_, iterator_impl<true>>::type& other) {
            owner_ = other.owner_;
            block_ = other.block_;
            current_ = other.current_;
            return *this;
        }
    private:
        iterator_impl(Owner& owner, Block* b, ValueType* c) : owner_(&owner), block_(b), current_(c) {}
        
        friend class ArrayList<T>;
        friend struct iterator_impl<!IsConst>;
        using Block = typename Owner::Block;
        Owner* owner_;
        Block* block_;
        ValueType* current_;
    };
    
    template <typename T>
    typename ArrayList<T>::iterator ArrayList<T>::begin() {
        return iterator(*this, head_, head_ ? head_->begin : nullptr);
    }
    
    template <typename T>
    typename ArrayList<T>::iterator ArrayList<T>::end() {
        return iterator(*this, nullptr, nullptr);
    }
    
    template <typename T>
    typename ArrayList<T>::const_iterator ArrayList<T>::begin() const {
        return const_iterator(*this, head_, head_ ? head_->begin : nullptr);
    }
    
    template <typename T>
    typename ArrayList<T>::const_iterator ArrayList<T>::end() const {
        return const_iterator(*this, nullptr, nullptr);
    }
}

#endif
