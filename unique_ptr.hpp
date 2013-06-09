//
//  unique_ptr.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 24/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_unique_ptr_hpp
#define grace_unique_ptr_hpp

#include "base/basic.hpp"
#include "memory/allocator.hpp"

namespace grace {
	template <typename T>
	class UniquePtr {
	public:
		template <typename U> friend class UniquePtr;
		
		UniquePtr() { check_invariant(); }
		UniquePtr(NullPtr* null) : ptr_(nullptr), alloc_(nullptr) { check_invariant(); }
		UniquePtr(UniquePtr<T>&& other) { this->swap(other); check_invariant(); }
		template <typename U>
		UniquePtr(UniquePtr<U>&& other) {
			alloc_ = other.alloc_;
			ptr_ = other.release();
			check_invariant();
		}
		~UniquePtr() { reset(nullptr, nullptr); }
		
		IAllocator& allocator() const { ASSERT(alloc_); return *alloc_; }
		T* get() const { return ptr_; }
		T* operator->() const { return get(); }
		const T& operator*() const { return *get(); }
		T& operator*() { return *get(); }
		
		explicit operator bool() const { return ptr_ != nullptr; }
		
		template <typename U>
		typename std::enable_if<!std::is_same<T, U>::value && std::is_assignable<T*, U*>::value, UniquePtr<T>>::type&
		operator=(UniquePtr<U>&& other) {
			reset(*other.alloc_, other.release());
			return *this;
		}
		UniquePtr<T>& operator=(UniquePtr<T>&& other) {
			reset(*other.alloc_, other.release());
			return *this;
		}
		bool operator==(T* b) const { return get() == b; }
		bool operator!=(T* b) const { return get() != b; }
		
		T* release() {
			T* p = ptr_;
			ptr_ = nullptr;
			alloc_ = nullptr;
			check_invariant();
			return p;
		}
		
		void reset(IAllocator& alloc, T* ptr) {
			reset(&alloc, ptr);
		}
		
		void swap(UniquePtr<T>& other) {
			std::swap(ptr_, other.ptr_);
			std::swap(alloc_, other.alloc_);
			check_invariant();
			other.check_invariant();
		}
	private:
		template <typename U, typename... Args> friend UniquePtr<U> make_unique(IAllocator&, Args&&...);
		
		UniquePtr(IAllocator* alloc, T* ptr) : alloc_(alloc), ptr_(ptr) {
			check_invariant();
		}
		
		IAllocator* alloc_ = nullptr;
		T* ptr_ = nullptr;
		
		void reset(IAllocator* alloc, T* ptr) {
			if (ptr_ && ptr != ptr_) {
				destroy(ptr_, *alloc_);
			}
			ASSERT(ptr != ptr_ || (ptr == ptr_ && alloc_ == alloc));
			ptr_ = ptr;
			alloc_ = alloc;
			check_invariant();
		}
		
		void check_invariant() {
			ASSERT(ptr_ == nullptr || alloc_ != nullptr);
		}
	};
	
	template <typename T, typename... Args>
	UniquePtr<T> make_unique(IAllocator& alloc, Args&&... args) {
		return UniquePtr<T>(&alloc, new(alloc) T(std::forward<Args>(args)...));
	}
	
	template <typename T>
	UniquePtr<T> copy_unique(const UniquePtr<T>& ptr) {
		return ptr ? make_unique<T>(ptr.allocator(), *ptr) : nullptr;
	}
	
	template <typename T>
	inline bool operator==(const UniquePtr<T>& a, const UniquePtr<T>& b) {
		return &a == &b;
	}
	
	template <typename T>
	inline bool operator==(T* a, const UniquePtr<T>& b) {
		return a == b.get();
	}
	
	template <typename T>
	inline bool operator!=(T* a, const UniquePtr<T>& b) {
		return a != b.get();
	}
}

#endif
