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
		friend struct GetNextNode<T>;
		friend struct GetPreviousNode<T>;
	};
	
	template <typename T>
	struct LinkListItemHolder {
	private:
		template <typename... Args>
		LinkListItemHolder(Args&&... args) : value(std::forward<Args>(args)...) {}
		LinkListItemHolder<T>* next = nullptr;
		LinkListItemHolder<T>* previous = nullptr;
		friend class LinkList<T>;
		friend struct GetNextNode<LinkListItemHolder<T>>;
		friend struct GetPreviousNode<LinkListItemHolder<T>>;
	public:
		T value;
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
	struct GetPreviousNode<LinkListItemHolder<T>> {
		static LinkListItemHolder<T>* get(LinkListItemHolder<T>* x) {
			return x->previous;
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
		
		LinkList(IAllocator& alloc = default_allocator()) : allocator_(alloc) { setup_sentinel(); }
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
		bool empty() const;
		
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
		ItemType sentinel_;
		
		void setup_sentinel();
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
		link_after(n, GetPreviousNode<ItemType>::get(&sentinel_));
		return *GetValueForNode<ItemType>::get(n);
	}
	
	template <typename T>
	template <typename... Args>
	T& LinkList<T>::emplace_front(Args&&... args) {
		ItemType* n = new(allocator_) ItemType(std::forward<Args>(args)...);
		link_before(n, GetNextNode<ItemType>::get(&sentinel_));
		return *GetValueForNode<ItemType>::get(n);
	}
	
	template <typename T>
	typename LinkList<T>::iterator
	LinkList<T>::begin() {
		return iterator(GetNextNode<ItemType>::get(&sentinel_));
	}
	
	template <typename T>
	typename LinkList<T>::iterator
	LinkList<T>::end() {
		return iterator(&sentinel_);
	}
	
	template <typename T>
	typename LinkList<T>::const_iterator
	LinkList<T>::begin() const {
		return const_iterator(GetNextNode<ItemType>::get(&sentinel_));
	}
	
	template <typename T>
	typename LinkList<T>::const_iterator
	LinkList<T>::end() const {
		return const_iterator(&sentinel_);
	}
	
	template <typename T>
	void LinkList<T>::link_after(ItemType* it, ItemType* after) {
		it->previous = after;
		it->next = after->next;
		it->previous->next = it;
		it->next->previous = it;
	}
	
	template <typename T>
	void LinkList<T>::link_before(ItemType* it, ItemType* before) {
		it->next = before;
		it->previous = before->previous;
		it->previous->next = it;
		it->next->previous = it;
	}
	
	template <typename T>
	void LinkList<T>::unlink(ItemType* it) {
		it->next->previous = it->previous;
		it->previous->next = it->next;
	}
	
	template <typename T>
	void LinkList<T>::setup_sentinel() {
		sentinel_.next = &sentinel_;
		sentinel_.previous = &sentinel_;
	}
}

#endif
