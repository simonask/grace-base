//
//  allocator.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 17/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_allocator_hpp
#define grace_allocator_hpp

#include "base/basic.hpp"
#include "memory/memory_tracker.hpp"
#include <atomic>
#include <string.h>

namespace grace {
	class FormattedStream;
	
#if defined(DEBUG) && DEBUG
	static const bool POISON_MEMORY = true;
#else
	static const bool POISON_MEMORY = false;
#endif

	class IAllocator {
	public:
		IAllocator() {}
		virtual ~IAllocator() {}
		virtual void* allocate(size_t nbytes, size_t alignment) = 0;
		virtual void* reallocate(void* ptr, size_t old_size, size_t new_size, size_t alignment) = 0;
		virtual void free(void* ptr, size_t nbytes) = 0; // Should not call finalizer!
        virtual void* allocate_large(size_t nbytes, size_t alignment, size_t& out_actually_allocated) = 0;
        virtual void free_large(void* ptr, size_t actual_size) = 0;
		virtual size_t usage() const = 0;
		virtual size_t capacity() const = 0;
	private:
		IAllocator(const IAllocator&) = delete;
		IAllocator(IAllocator&&) = delete;
		IAllocator& operator=(const IAllocator&) = delete;
	};
	
	struct MemoryTracker;
	struct MemoryLeak;

	/*
	 SystemAllocator has same semantics as malloc/free.
	*/
	class SystemAllocator : public IAllocator {
	public:
		SystemAllocator();
		~SystemAllocator();
		void* allocate(size_t nbytes, size_t alignment) final;
		void* reallocate(void* ptr, size_t old_size, size_t new_size, size_t alignment) final;
		void free(void* ptr, size_t nbytes) final;
		void free(void* ptr); // simple version that doesn't poison memory.
        void* allocate_large(size_t nbytes, size_t alignment, size_t& out_actually_allocated) final;
        void free_large(void* ptr, size_t actual_size) final;
		
		size_t usage() const final { return usage_; }
		size_t capacity() const final { return SIZE_MAX; }
		
		void start_allocation_tracking();
		void pause_allocation_tracking();
		void unpause_allocation_tracking();
		Array<MemoryLeak> finish_allocation_tracking(IAllocator& leak_info_alloc);
		
		void display_backtrace_for_allocation(void* ptr);
	private:
		std::atomic<size_t> usage_;
		MemoryTracker tracker_;
	};
	
	SystemAllocator& default_allocator();
	
	void display_backtrace_for_system_allocation(void* ptr);
	
	/*
	 LinearAllocator initially allocates one huge block of memory, which other allocators can
	 use as a basis.
	*/
	class LinearAllocator : public IAllocator {
	public:
		explicit LinearAllocator(size_t size);
		~LinearAllocator();
		
		void* allocate(size_t nbytes, size_t alignment) final;
		void* reallocate(void* ptr, size_t old_size, size_t new_size, size_t alignment) final;
		void free(void* ptr, size_t nbytes) final;
        void* allocate_large(size_t nbytes, size_t alignment, size_t& out_actually_allocated) final;
        void free_large(void* ptr, size_t actual_size) final;
		
		size_t usage() const final { return current_ - begin_; }
		size_t capacity() const final { return end_ - begin_; }
		
		byte* current() const;
		byte* begin() const { return begin_; }
		byte* end() const { return end_; }
		void reset(byte* p);
	private:
		byte* begin_ = nullptr;
		byte* end_ = nullptr;
		byte* current_ = nullptr;
	};
	
	LinearAllocator& scratch_linear_allocator();
	
	/*
	 ScratchAllocator automatically destroys all objects when it gets destroyed.
	 This is in effect a crude form of GC.
	*/
	class ScratchAllocator : public IAllocator {
	public:
		ScratchAllocator() : base_(scratch_linear_allocator()), reset_(base_.current()), last_current_(reset_) {}
		explicit ScratchAllocator(LinearAllocator& base) : base_(base), reset_(base.current()), last_current_(reset_) {}
		~ScratchAllocator();
		void* allocate_with_finalizer(size_t nbytes, size_t alignment, void(*finalize)(void*));
		void* allocate(size_t nbytes, size_t alignment) final;
		void* reallocate(void* ptr, size_t old_size, size_t new_size, size_t alignment) final;
		void free(void* ptr, size_t nbytes) final { /* no-op */ }
        void* allocate_large(size_t nbytes, size_t alignment, size_t& out_actually_allocated) final;
        void free_large(void* ptr, size_t actual_size) final { /* no-op */ }
		
		size_t usage() const final { return base_.current() - reset_; }
		size_t capacity() const final { return base_.end() - reset_; }
		
		template <typename T, typename... Args>
		typename std::enable_if<std::is_pod<T>::value, T*>::type
		create(Args&&... args) {
			void* p = allocate(sizeof(T), alignof(T));
			return new(p) T(std::forward<Args>(args)...);
		}
		
		template <typename T, typename... Args>
		typename std::enable_if<!std::is_pod<T>::value, T*>::type
		create(Args&&... args) {
			void* p = allocate_with_finalizer(sizeof(T), alignof(T), destruct<T>);
			return new(p) T(std::forward<Args>(args)...);
		}
	
	private:
		struct Finalizer {
			void* ptr;
			void(*finalize)(void*);
			Finalizer* next;
		};
	
		LinearAllocator& base_;
		byte* reset_ = nullptr;
		Finalizer* finalizers_ = nullptr;
		byte* last_current_ = nullptr;
	};
	
	namespace detail {
		static const byte   UNALLOCATED_MEMORY_PATTERN = 0xab;
		static const byte UNINITIALIZED_MEMORY_PATTERN = 0xcd;
		static const byte         FREED_MEMORY_PATTERN = 0xef;
		
		inline void poison_memory(byte* begin, byte* end, byte pattern) {
			if (POISON_MEMORY) {
				memset(begin, pattern, end-begin);
			}
		}
	}
	
	
	inline void* LinearAllocator::reallocate(void *ptr, size_t old_size, size_t new_size, size_t alignment) {
		void* new_ptr = allocate(new_size, alignment);
		ASSERT(ptr < new_ptr);
		if (ptr != nullptr) {
			::memcpy(new_ptr, ptr, old_size);
		}
		return new_ptr;
	}
    
    inline void* LinearAllocator::allocate_large(size_t nbytes, size_t alignment, size_t &out_actually_allocated) {
        out_actually_allocated = nbytes;
        return allocate(nbytes, alignment);
    }
	
	inline void LinearAllocator::free(void*, size_t) {
		// no-op
	}
    
    inline void LinearAllocator::free_large(void* ptr, size_t actual_size) {
        // no-op
    }
	
	inline byte* LinearAllocator::current() const {
		return current_;
	}
	
	inline void LinearAllocator::reset(byte* new_current) {
		ASSERT(new_current >= begin_ && new_current < end_);
		detail::poison_memory(new_current, current_, detail::FREED_MEMORY_PATTERN);
		current_ = new_current;
	}
	
	inline void* ScratchAllocator::allocate(size_t nbytes, size_t alignment) {
		byte* ptr = (byte*)base_.allocate(nbytes, alignment);
		last_current_ = base_.current();
		detail::poison_memory(ptr, ptr + nbytes, detail::UNINITIALIZED_MEMORY_PATTERN);
		return ptr;
	}
	
	inline void* ScratchAllocator::reallocate(void *ptr, size_t old_size, size_t new_size, size_t alignment) {
		void* new_ptr = allocate(new_size, alignment);
		ASSERT(ptr < new_ptr);
		if (ptr != nullptr) {
			::memcpy(new_ptr, ptr, old_size);
		}
		return new_ptr;
	}
	
	inline void* ScratchAllocator::allocate_with_finalizer(size_t nbytes, size_t alignment, void (*finalize)(void *)) {
		void* ptr = allocate(nbytes, alignment);
		Finalizer* f = (Finalizer*)allocate(sizeof(Finalizer), alignof(Finalizer));
		f->ptr = ptr;
		f->finalize = finalize;
		f->next = finalizers_;
		finalizers_ = f;
		return ptr;
	}
    
    inline void* ScratchAllocator::allocate_large(size_t nbytes, size_t alignment, size_t &out_actually_allocated) {
		byte* ptr = (byte*)base_.allocate_large(nbytes, alignment, out_actually_allocated);
		last_current_ = base_.current();
		detail::poison_memory(ptr, ptr + out_actually_allocated, detail::UNINITIALIZED_MEMORY_PATTERN);
		return ptr;
    }
	
	inline ScratchAllocator::~ScratchAllocator() {
		// The following is a check for asymmetric allocators — i.e., this scratch allocator goes out of scope
		// before one that uses memory further down does, and that's an error.
		//
		// Correct:
		// ScratchAllocator alloc1;
		// ScratchAllocator alloc2;
		// alloc2.~ScratchAllocator();
		// alloc1.~ScratchAllocator();
		//
		// The situation that triggers this assert:
		// ScratchAllocator alloc1;
		// ScratchAllocator alloc2;
		// alloc1.~ScratchAllocator();
		// alloc2.~ScratchAllocator();
		ASSERT(last_current_ == base_.current());
		for (Finalizer* f = finalizers_; f; f = f->next) {
			f->finalize(f->ptr);
		}
		base_.reset(reset_);
	}
	
	template <typename T>
	typename std::enable_if<std::is_polymorphic<T>::value, void>::type
	destroy(T* ptr, grace::IAllocator& alloc) {
		if (ptr != nullptr) {
			/*
			ptr might be a pointer to an interface on a real, larger object.
			Freeing the interface pointer will cause errors, because the object
			will be allocated at a different address. The following line is a
			special case of dynamic_cast that gets the "topmost" object pointer.
			*/
			void* largest_object = dynamic_cast<void*>(const_cast<typename std::remove_const<T>::type*>(ptr));

			ptr->~T();

			size_t object_size = sizeof(T); // There is no way to portably know the size of a polymorphic object in C++.

			alloc.free(largest_object, object_size);
		}
	}

	template <typename T>
	typename std::enable_if<!std::is_polymorphic<T>::value, void>::type
	destroy(T* ptr, grace::IAllocator& alloc) {
		if (ptr != nullptr) {
			ptr->~T();
			alloc.free(ptr, sizeof(T));
		}
	}
}

inline void* operator new(size_t nbytes) {
	return grace::default_allocator().allocate(nbytes, 16);
}
inline void* operator new[](size_t nbytes) {
	return grace::default_allocator().allocate(nbytes, 16);
}

inline void operator delete(void* ptr) throw() {
	grace::default_allocator().free(ptr);
}
inline void operator delete[](void* ptr) throw() {
	grace::default_allocator().free(ptr);
}

inline void* operator new(size_t nbytes, grace::IAllocator& alloc, size_t alignment = 0) {
	return alloc.allocate(nbytes, alignment ? alignment : 16);
}



#endif
