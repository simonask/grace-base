//
//  link_list.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 12/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_auto_list_hpp
#define falling_auto_list_hpp

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
	struct AutoListLink {
		AutoListLink<T>* next = nullptr;
		AutoListLink<T>** prev_next_ptr = nullptr;
		
		~AutoListLink() {
			unlink();
		}
		
		void unlink() {
			if (next) {
				next->prev_next_ptr = prev_next_ptr;
			}
			*prev_next_ptr = next;
		}
	};
	
	template <typename T, size_t MemberOffset>
	struct AutoList {
		typedef T ValueType;
		static const size_t LinkOffset = MemberOffset;
		
		AutoList() {}
		AutoList(const AutoList<T, MemberOffset>&) = delete;
		AutoList<T,MemberOffset>& operator=(const AutoList<T, MemberOffset>&) = delete;
		
		T* convert_link_to_object(AutoListLink<T>* link) const {
			ASSERT(link != nullptr);
			byte* pl = reinterpret_cast<byte*>(link);
			byte* po = pl - MemberOffset;
			return reinterpret_cast<T*>(po);
		}
		
		T* head() const {
			if (head_) {
				return convert_link_to_object(head_);
			} else {
				return nullptr;
			}
		}
		
		void link_head(AutoListLink<T>* l) {
			if (head_) {
				head_->prev_next_ptr = &l->next;
			}
			l->next = head_;
			l->prev_next_ptr = &head_;
			head_ = l;
		}
		
		void unlink(AutoListLink<T>* l) {
			l->unlink();
		}
		
		bool empty() const {
			return head_ == nullptr;
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
			template <typename, size_t> friend class AutoList;
			iterator(AutoList<T,MemberOffset>* self, AutoListLink<T>* current) : self(self), current(current) {}
			AutoList<T,MemberOffset>* self = nullptr;
			AutoListLink<T>* current = nullptr;
		};
		
		iterator begin() { return iterator(this, head_); }
		iterator end() { return iterator(this, nullptr); }
	private:
		AutoListLink<T>* head_ = nullptr;
	};
	
#define AUTO_LIST_TYPE(T, MEMBER) AutoList<T, offsetof(T, MEMBER)>

	struct VirtualAutoListBase {
		virtual ~VirtualAutoListBase() {}
	};
	
	template <typename T, size_t MemberOffset>
	struct VirtualAutoList : VirtualAutoListBase, AutoList<T, MemberOffset> {
		
	};
}

#endif
