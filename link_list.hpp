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

namespace falling {
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
