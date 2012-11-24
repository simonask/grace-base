//
//  link_list.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 12/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_intrusive_list_hpp
#define falling_intrusive_list_hpp

#include "base/basic.hpp"
#include "memory/allocator.hpp"

namespace falling {
    template <typename T> class BareLinkList;
    
    template <typename T>
    struct ListLinkBase {
    private:
        friend class BareLinkList<T>;
        T* next = nullptr;
        T* previous = nullptr;
    };
    
    /*
        A BareLinkList is a linked list that doesn't manage its own memory.
     */
    template <typename T>
    struct BareLinkList {
    public:
        BareLinkList() {}
        
        T* head() const { return head_; }
        T* tail() const { return tail_; }
        T& front() { return *head_; }
        const T& front() const { return *head_; }
        T& back() { return *tail_; }
        const T& back() const { return *tail_; }
        
        void link_tail(T* element);
        void link_head(T* element);
        void unlink(T* element);
        
        template <bool IsConst> struct iterator_impl;
        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        
        iterator erase(iterator it);
    private:
        T* head_ = 0;
        T* tail_ = 0;
    };
    
    template <typename T>
    void BareLinkList<T>::link_tail(T* element) {
        element->previous = tail_;
        if (tail_) {
            tail_->previous = element;
        }
        tail_ = element;
        if (head_ == nullptr) {
            head_ = element;
        }
    }
    
    template <typename T>
    void BareLinkList<T>::link_head(T* element) {
        element->next = head_;
        if (head_) {
            head_->next = element;
        }
        head_ = element;
        if (tail_ == nullptr) {
            tail_ = element;
        }
    }
    
    template <typename T>
    void BareLinkList<T>::unlink(T* element) {
        if (element->next) {
            element->next->previous = element->previous;
        }
        if (element->previous) {
            element->previous->next = element->next;
        }
        if (element == head_) {
            head_ = element->previous;
        }
        if (element == tail_) {
            tail_ = element->next;
        }
    }
    
    template <typename T>
    template <bool IsConst>
    struct BareLinkList<T>::iterator_impl {
    public:
        using Owner = typename std::conditional<IsConst, const BareLinkList<T>, BareLinkList<T>>::type;
        using ValueType = typename std::conditional<IsConst, const T, T>::type;
        using value_type = ValueType;
        using Self = iterator_impl<IsConst>;
        
        iterator_impl(const iterator_impl<false>& other) : current_(other.current_) {}
        template <bool IsConst_>
        iterator_impl(const typename std::enable_if<IsConst_, iterator_impl<true>>::type& other) : current_(other.current_) {}
        
        Self& operator++() {
            current_ = current_->next;
            return *this;
        }
        
        Self operator++(int) {
            Self s = *this;
            ++s;
            return s;
        }
        
        Self& operator--() {
            current_ = current_->previous;
            return *this;
        }
        
        Self operator--(int) {
            Self s = *this;
            --s;
            return s;
        }
        
        ValueType& operator*() const {
            return *current_;
        }
        
        ValueType* operator->() const {
            return current_;
        }
        
        ValueType* get() const {
            return current_;
        }
        
        template <bool B>
        bool operator==(const iterator_impl<B>& other) const { return current_ == other.current_; }
        template <bool B>
        bool operator!=(const iterator_impl<B>& other) const { return !(*this == other); }
    private:
        friend struct iterator_impl<!IsConst>;
        friend class BareLinkList<T>;
        ValueType* current_;
        
        iterator_impl(ValueType* current) : current_(current) {}
    };
    
    template <typename T>
    typename BareLinkList<T>::iterator BareLinkList<T>::begin() {
        return iterator(head_);
    }
    
    template <typename T>
    typename BareLinkList<T>::iterator BareLinkList<T>::end() {
        return iterator(nullptr);
    }
    
    template <typename T>
    typename BareLinkList<T>::const_iterator BareLinkList<T>::begin() const {
        return const_iterator(head_);
    }
    
    template <typename T>
    typename BareLinkList<T>::const_iterator BareLinkList<T>::end() const {
        return const_iterator(nullptr);
    }
    
    template <typename T>
    typename BareLinkList<T>::iterator BareLinkList<T>::erase(iterator it) {
        auto next = it;
        ++next;
        unlink(it.get());
        return next;
    }
    
	template <typename T>
	struct IntrusiveListLink {
		IntrusiveListLink<T>* next = nullptr;
		IntrusiveListLink<T>* previous = nullptr;
		
		~IntrusiveListLink() {
			unlink();
		}
		
		void unlink() {
			if (previous)
				previous->next = next;
			if (next)
				next->previous = previous;
		}
	};
	
	template <typename T, size_t MemberOffset>
	struct IntrusiveList {
		typedef T ValueType;
		static const size_t LinkOffset = MemberOffset;
		IntrusiveListLink<T> begin_sentinel;
		IntrusiveListLink<T> end_sentinel;
		
		IntrusiveList() {
			begin_sentinel.next = &end_sentinel;
			end_sentinel.previous = &begin_sentinel;
		}
		
		void force_reset() {
			// WARNING: This may cause circular references! Use with caution.
			begin_sentinel.next = &end_sentinel;
			end_sentinel.previous = &begin_sentinel;
		}
		
		IntrusiveList(const IntrusiveList<T, MemberOffset>&) = delete;
		IntrusiveList<T,MemberOffset>& operator=(const IntrusiveList<T, MemberOffset>&) = delete;
		
		T* convert_link_to_object(IntrusiveListLink<T>* link) const {
			ASSERT(link != nullptr);
			byte* pl = reinterpret_cast<byte*>(link);
			byte* po = pl - MemberOffset;
			return reinterpret_cast<T*>(po);
		}
		
		void link_tail(IntrusiveListLink<T>* l) {
			ASSERT(l->next == nullptr && l->previous == nullptr); // Already in a list
			end_sentinel.previous->next = l;
			l->previous = end_sentinel.previous;
			end_sentinel.previous = l;
			l->next = &end_sentinel;
		}
		
		bool empty() const {
			return begin_sentinel.next == &end_sentinel;
		}
		
		struct iterator {
		public:
			iterator() = default;
			iterator(const iterator& other) = default;
			iterator& operator=(const iterator& other) = default;
			bool operator==(const iterator& other) const { return self == other.self && current == other.current; }
			bool operator!=(const iterator& other) const { return !(*this == other); }
			iterator& operator++() { current = current->next; return *this; }
			iterator operator++(int) { iterator copy = *this; current = current->next; return copy; }
			T* operator*() const { return self->convert_link_to_object(current); }
		private:
			template <typename, size_t> friend class IntrusiveList;
			iterator(IntrusiveList<T,MemberOffset>* self, IntrusiveListLink<T>* current) : self(self), current(current) {}
			IntrusiveList<T,MemberOffset>* self = nullptr;
			IntrusiveListLink<T>* current = nullptr;
		};
		
		iterator begin() { return iterator(this, begin_sentinel.next); }
		iterator end() { return iterator(this, &end_sentinel); }
	};
	
#define INTRUSIVE_LIST_TYPE(T, MEMBER) IntrusiveList<T, offsetof(T, MEMBER)>

	struct VirtualIntrusiveListBase {
		virtual ~VirtualIntrusiveListBase() {}
	};
	
	template <typename T, size_t MemberOffset>
	struct VirtualIntrusiveList : VirtualIntrusiveListBase, IntrusiveList<T, MemberOffset> {
		
	};
}

#endif
