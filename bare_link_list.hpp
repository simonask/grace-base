//
//  bare_link_list.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 25/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_bare_link_list_hpp
#define falling_bare_link_list_hpp

#include "base/iterators.hpp"

namespace falling {
	template <typename T> class BareLinkList;
    
    template <typename T>
    struct ListLinkBase {
    private:
        friend class BareLinkList<T>;
		friend struct GetNextNode<ListLinkBase<T>>;
        T* next = nullptr;
        T* previous = nullptr;
    };
	
	template <typename T>
	struct GetNextNode<ListLinkBase<T>> {
		static T* get(ListLinkBase<T>* x) { return x->next; }
	};
	
	template <typename T>
	struct GetValueForNode<ListLinkBase<T>> {
		static T* get(ListLinkBase<T>* x) { return static_cast<T*>(x); }
	};
	
	template <typename T>
	struct GetNodeValueType<ListLinkBase<T>> {
		using Type = T;
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
        
        template <bool IsConst> using iterator_impl = ForwardLinkListIterator<BareLinkList<T>, ListLinkBase<T>, IsConst>;
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

}

#endif
