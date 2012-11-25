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

#include "base/auto_link_queue.hpp"
#include "base/bare_link_list.hpp"
#include "base/iterators.hpp"

namespace falling {
	template <typename T> class LinkList;

	template <typename T>
	struct LinkListItem {
	private:
		T* next = nullptr;
		T* previous = nullptr;
		friend class LinkList<T>;
	};
	
	template <typename T>
	struct LinkListItemHolder {
	public:
		T value;
	private:
		template <typename... Args>
		LinkListItemHolder(Args&&... args) : value(std::forward<Args>(args)...) {}
		LinkListItemHolder<T>* next = nullptr;
		LinkListItemHolder<T>* previous = nullptr;
		friend class LinkList<T>;
		friend struct GetNextNode<LinkListItemHolder<T>>;
	};
	
	template <typename T>
	struct GetNodeValueType<LinkListItemHolder<T>> {
		using Type = T;
	};

	template <typename T> struct GetLinkListItemType {
		using Type = typename std::conditional<std::is_base_of<LinkListItem<T>, T>::value, T, LinkListItemHolder<T>>::type;
	};
	
	template <typename T>
	struct GetValueForNode<LinkListItemHolder<T>> {
		static T* get(LinkListItemHolder<T>* x) {
			return &x->value;
		}
	};
	
	template <typename T>
	struct GetNextNode<LinkListItemHolder<T>> {
		static LinkListItemHolder<T>* get(LinkListItemHolder<T>* x) {
			return x->next;
		}
	};
	
	template <typename T>
	typename GetLinkListItemType<T>::Type* get_next_from_link_list_node(LinkListItem<T>* item) {
		return item->next;
	}

	template <typename T>
	class LinkList {
	public:
		using ItemType = typename GetLinkListItemType<T>::Type;
		
		LinkList(IAllocator& alloc = default_allocator()) : allocator_(alloc) {}
		LinkList(IAllocator& alloc, const LinkList<T>& other);
		LinkList(const LinkList<T>& other);
		LinkList(LinkList<T>&& other);
		LinkList<T>& operator=(const LinkList<T>& other);
		
		void push_back(T&& x);
		void push_back(const T& x);
		void push_front(T x);
		template <typename... Args>
		T& emplace_back(Args&&... args);
		template <typename... Args>
		T& emplace_front(Args&&... args);
		
		T& front();
		const T& front() const;
		T& back();
		const T& back() const;
		
		using iterator = ForwardLinkListIterator<LinkList<T>, ItemType, false>;
		using const_iterator = ForwardLinkListIterator<LinkList<T>, ItemType, true>;
		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;
		
		iterator erase(iterator it);
		void clear();
	private:
		IAllocator& allocator_;
		ItemType* head_ = nullptr;
		ItemType* tail_ = nullptr;
		
		void link_after(ItemType* item, ItemType* after);
		void link_before(ItemType* item, ItemType* before);
		void unlink(ItemType* item);
	};
	
	template <typename T>
	void LinkList<T>::push_back(T&& x) {
		emplace_back(std::move(x));
	}
	
	template <typename T>
	void LinkList<T>::push_back(const T& x) {
		emplace_back(x);
	}
	
	template <typename T>
	template <typename... Args>
	T& LinkList<T>::emplace_back(Args&&... args) {
		ItemType* n = new(allocator_) ItemType(std::forward<Args>(args)...);
		link_after(n, tail_);
		return *GetValueForNode<ItemType>::get(n);
	}
	
	template <typename T>
	template <typename... Args>
	T& LinkList<T>::emplace_front(Args&&... args) {
		ItemType* n = new(allocator_) ItemType(std::forward<Args>(args)...);
		link_before(n, head_);
		return *GetValueForNode<ItemType>::get(n);
	}
	
	template <typename T>
	typename LinkList<T>::iterator
	LinkList<T>::begin() {
		return iterator(head_);
	}
	
	template <typename T>
	typename LinkList<T>::iterator
	LinkList<T>::end() {
		return iterator(nullptr);
	}
	
	template <typename T>
	typename LinkList<T>::const_iterator
	LinkList<T>::begin() const {
		return const_iterator(head_);
	}
	
	template <typename T>
	typename LinkList<T>::const_iterator
	LinkList<T>::end() const {
		return const_iterator(nullptr);
	}
	
	template <typename T>
	void LinkList<T>::link_after(ItemType* it, ItemType* after) {
		it->previous = after;
		if (after) {
			it->next = after->next;
			after->next = it;
			if (it->next) {
				it->next->previous = it;
			} else {
				tail_ = it;
			}
		} else {
			ASSERT(tail_ == nullptr && head_ == nullptr);
			tail_ = head_ = it;
		}
	}
	
	template <typename T>
	void LinkList<T>::link_before(ItemType* it, ItemType* before) {
		it->next = before;
		if (before) {
			it->previous = before->previous;
			before->previous = it;
			if (it->previous) {
				it->previous->next = it;
			} else {
				head_ = it;
			}
		} else {
			ASSERT(tail_ == nullptr && head_ == nullptr);
			tail_ = head_ = it;
		}
	}
	
	template <typename T>
	void LinkList<T>::unlink(ItemType* it) {
		if (it->previous) {
			it->previous->next = it->next;
		}
		if (it->next) {
			it->next->previous = it->previous;
		}
		it->next = nullptr;
		it->previous = nullptr;
	}
}

#endif
