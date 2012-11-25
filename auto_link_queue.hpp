//
//  auto_link_stack.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 25/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_auto_link_stack_hpp
#define falling_auto_link_stack_hpp

namespace falling {
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
