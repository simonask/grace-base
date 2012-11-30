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
		friend struct GetPreviousNode<ListLinkBase<T>>;
        T* next = nullptr;
        T* previous = nullptr;
    };
	
	template <typename T>
	struct GetNextNode<ListLinkBase<T>> {
		static T* get(ListLinkBase<T>* x) { return x->next; }
	};
	
	template <typename T>
	struct GetPreviousNode<ListLinkBase<T>> {
		static T* get(ListLinkBase<T>* x) { return x->previous; }
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
        BareLinkList() {
			sentinel.next = (T*)&sentinel;
			sentinel.previous = (T*)&sentinel;
		}
        
        T* head() const { ASSERT(!empty()); return sentinel.next; }
        T* tail() const { ASSERT(!empty()); return sentinel.previous; }
        T& front() { return *sentinel.next; }
        const T& front() const { return *sentinel.next; }
        T& back() { return *sentinel.previous; }
        const T& back() const { return *sentinel.previous; }
        
        void link_tail(T* element);
        void link_head(T* element);
        void unlink(T* element);
		
		bool empty() const { return sentinel.next == (T*)&sentinel; }
        
        template <bool IsConst> using iterator_impl = ForwardLinkListIterator<BareLinkList<T>, ListLinkBase<T>, IsConst>;
        using iterator = iterator_impl<false>;
        using const_iterator = iterator_impl<true>;
        iterator begin();
		iterator last();
        iterator end();
        const_iterator begin() const;
		const_iterator last() const;
        const_iterator end() const;
        
        iterator erase(iterator it);
    private:
		ListLinkBase<T> sentinel; // previous = tail, next = head
    };
    
    template <typename T>
    void BareLinkList<T>::link_tail(T* element) {
		element->next = (T*)&sentinel;
		element->previous = sentinel.previous;
		element->previous->next = element;
		element->next->previous = element;
    }
    
    template <typename T>
    void BareLinkList<T>::link_head(T* element) {
		element->previous = (T*)&sentinel;
		element->next = sentinel.next;
		element->next->previous = element;
		element->previous->next = element;
    }
    
    template <typename T>
    void BareLinkList<T>::unlink(T* element) {
		element->next->previous = element->previous;
		element->previous->next = element->next;
    }
        
    template <typename T>
    typename BareLinkList<T>::iterator BareLinkList<T>::begin() {
        return iterator(sentinel.next);
    }
	
	template <typename T>
	typename BareLinkList<T>::iterator BareLinkList<T>::last() {
		return iterator(sentinel.previous);
	}
    
    template <typename T>
    typename BareLinkList<T>::iterator BareLinkList<T>::end() {
        return iterator((T*)&sentinel);
    }
    
    template <typename T>
    typename BareLinkList<T>::const_iterator BareLinkList<T>::begin() const {
        return const_iterator(sentinel.next);
    }
	
	template <typename T>
	typename BareLinkList<T>::const_iterator BareLinkList<T>::last() const {
		return const_iterator(sentinel.previous);
	}
    
    template <typename T>
    typename BareLinkList<T>::const_iterator BareLinkList<T>::end() const {
        return const_iterator((T*)&sentinel);
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
