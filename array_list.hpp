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

namespace falling {
	namespace detail {
		struct ArrayListPage {
			void* operator new(size_t sz);
			void operator delete(void* ptr);
		private:
			void* operator new[](size_t);
			void operator delete[](void*);
		};
	}
	
	
	template <typename T>
	class ArrayList {
	public:
		ArrayList() : head_(nullptr), tail_(nullptr) {}
		ArrayList(const ArrayList<T>& other);
		ArrayList(ArrayList<T>&& other);
		~ArrayList() { clear(); }
		ArrayList<T>& operator=(const ArrayList<T>& other);
		ArrayList<T>& operator=(ArrayList<T>&& other);
		
		template <bool IsConst>
		struct Iterator;		
		typedef T value_type;
		typedef Iterator<false> iterator;
		typedef Iterator<true> const_iterator;
		
		T& operator[](uint32 idx);
		const T& operator[](uint32 idx) const;
		
		uint32 size() const;
		uint32 capacity() const;
		void push_back(T element);
		void resize(uint32, T fill = T());
		void reserve(uint32);
		void clear();
		
		template <typename InputIterator>
		void insert(InputIterator begin, InputIterator end);
		template <typename InputIterator>
		void insert(InputIterator begin, InputIterator end, iterator position);
		void insert(T element, iterator position);
		
		template <typename... Args>
		void emplace_back(Args... args);
		
		iterator begin();
		iterator end();
		const_iterator begin();
		const_iterator end();
		
		iterator erase(iterator);
		iterator at(uint32 idx);
		
	private:
		struct Page : detail::ArrayListPage {
			typedef std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type Element;
			
			Page* previous;
			Page* next;
			Element* begin;
			Element* current;
			Element* end;
			
			size_t available() const { return end - current; }
		};
		
		static const PageSize = 4096;
		static const PageHeaderSize = round_up(sizeof(Page), sizeof(T));
		static const DataPerPage = PageSize - PageHeaderSize;
		static const ElementsPerPage = DataPerPage / sizeof(T);
		
		Page* head_;
		Page* tail_;
		size_t size_;
		size_t num_pages_;
		
		Page* create_page();
	};
	
	template <typename T>
	ArrayList<T>::ArrayList(const ArrayList<T>& other) : head_(nullptr), tail_(nullptr), size_(0), num_pages_(0) {
		insert(other.begin(), other.end());
	}
	
	template <typename T>
	ArrayList<T>::ArrayList(ArrayList<T>&& other) {
		head_ = other.head_;
		tail_ = other.tail_;
		size_ = other.size_;
		num_pages_ = other.num_pages_;
		other.head_ = nullptr;
		other.tail_ = nullptr;
		other.size_ = 0;
		other.num_pages_ = 0;
	}
	
	template <typename T>
	ArrayList<T>& ArrayList<T>::operator=(const ArrayList<T>& other) {
		clear();
		insert(other.begin(), other.end());
		return *this;
	}
	
	template <typename T>
	ArrayList<T>& ArrayList<T>::operator=(ArrayList<T>&& other) {
		clear();
		head_ = other.head_;
		tail_ = other.tail_;
		size_ = other.size_;
		num_pages_ = other.num_pages_;
		other.head_ = nullptr;
		other.tail_ = nullptr;
		other.size_ = 0;
		other.num_pages_ = 0;
		return *this;
	}
	
	template <typename T>
	T& ArrayList<T>::operator[](uint32 idx) {
		for (Page* p = head_; p != nullptr; p = p->next) {
			if (idx < ElementsPerPage) {
				ASSERT(idx < (p->current - p->begin)); // out of bounds!
				return p->begin[idx];
			}
			idx -= ElementsPerPage;
		}
		ASSERT(false); // out of bounds!
	}
	
	template <typename T>
	const T& ArrayList<T>::operator[](uint32 idx) const {
		for (Page* p = head_; p != nullptr; p = p->next) {
			if (idx < ElementsPerPage) {
				ASSERT(idx < (p->current - p->begin)); // out of bounds!
				return p->begin[idx];
			}
			idx -= ElementsPerPage;
		}
		ASSERT(false); // out of bounds!
	}
	
	template <typename T>
	uint32 ArrayList<T>::size() const {
		return size_;
	}
	
	template <typename T>
	uint32 ArrayList<T>::capacity() const {
		return num_pages_ * ElementsPerPage;
	}
	
	template <typename T>
	void ArrayList<T>::push_back(T element) {
		insert(std::move(element), end());
	}
	
	template <typename T>
	void ArrayList<T>::resize(uint32 new_size, T element) {
		size_t sz = size();
		if (new_size < sz) {
			auto i0 = at(new_size);
			auto i1 = end() - 1;
			for (auto it = i1; it != i0-1; --it) {
				erase(it);
			}
		} else if (new_size > sz) {
			size_t diff = new_size - sz;
			reserve(new_size);
			for (size_t i = 0; i < diff; ++i) {
				push_back(element);
			}
		}
	}
	
	template <typename T>
	void ArrayList<T>::reserve(uint32 new_capacity) {
		while (capacity() < new_capacity) {
			Page* p = create_page();
			if (tail_ == nullptr) {
				head_ = tail_ = p;
			} else {
				tail_->next = p;
				p->previous = tail_;
				tail_ = p;
			}
		}
	}
	
	template <typename T>
	void ArrayList<T>::clear() {
		for (Page* p = head_; p != nullptr;) {
			for (Page::Element* e = p->begin; e != p->current; ++e) {
				T* x = reinterpret_cast<T*>(e);
				x->~T();
			}
			
			Page* next = p->next;
			delete p;
			p = next;
		}
		size_ = 0;
		num_pages_ = 0;
	}
	
	template <typename T>
	template <typename InputIterator>
	void ArrayList<T>::insert(InputIterator i0, InputIterator i1) {
		insert(std::forward<InputIterator>(i0), std::forward<InputIterator>(i1), end());
	}
	
	template <typename T>
	template <typename InputIterator>
	void ArrayList<T>::insert(InputIterator i0, InputIterator i1, iterator position) {
		size_t enlarge_by = i1 - i0;
		reserve(size() + enlarge_by);
		
		// shift tail to the right
		auto shr_i0 = position;
		auto shr_i1 = end();
		size_t shr_n = shr_i1 - shr_i0;
		size_t shifted = 0;
		Page* shr_p = tail_;
		while (shifted < shr_n) {
			
		}
	}
}

#endif
