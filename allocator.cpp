//
//  allocator.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 17/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "memory/allocator.hpp"
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <malloc/malloc.h>
#include "io/formatted_stream.hpp"
#include "io/formatters.hpp"

#define DETECT_OVERRUN 0
#define DETECT_REUSE_AFTER_FREE 0
#if defined(malloc_size)
#define MALLOC_SIZE ::malloc_size
#elif defined(_msize)
#define MALLOC_SIZE ::_msize
#else
#undef MALLOC_SIZE
#endif

namespace falling {
#if !defined(PAGE_SIZE)
	static const size_t PAGE_SIZE = 4096; // defined by malloc.h
#endif

	namespace {
		uint64 next_pow2(uint64 n) {
			--n;
			n |= n >> 1;
			n |= n >> 2;
			n |= n >> 4;
			n |= n >> 8;
			n |= n >> 16;
			n |= n >> 32;
			return ++n;
		}
		
		uint32 next_pow2(uint32 n) {
			--n;
			n |= n >> 1;
			n |= n >> 2;
			n |= n >> 4;
			n |= n >> 8;
			n |= n >> 16;
			return ++n;
		}
		
		template <typename T = size_t>
		typename std::enable_if<sizeof(T) == sizeof(uint64), T>::type
		next_pow2(size_t n) { return next_pow2((uint64)n); }
		
		template <typename T = size_t>
		typename std::enable_if<sizeof(T) == sizeof(uint32), T>::type
		next_pow2(size_t n) { return next_pow2((uint32)n); }
	}
	


	namespace {
		void system_free(void*, size_t = 0);
		
		size_t system_alloc_size(void* ptr) {
#if defined(MALLOC_SIZE)
			return ptr ? MALLOC_SIZE(ptr) : 0;
#else
			return 0;
#endif
		}
		
		void* system_alloc(size_t nbytes, size_t alignment) {
#if DETECT_OVERRUN || DETECT_REUSE_AFTER_FREE
			size_t object_size = PAGE_SIZE;
			while (nbytes > object_size) object_size += PAGE_SIZE;
			byte* pages = (byte*)::mmap(nullptr, object_size + PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
			if (pages == nullptr) {
				throw OutOfMemoryError();
			}
			void* object = (void*)((intptr_t)((pages + object_size) - nbytes) & ~(alignment-1));
			detail::poison_memory((byte*)pages, (byte*)object, detail::UNALLOCATED_MEMORY_PATTERN);
			detail::poison_memory((byte*)object, pages + object_size, detail::UNINITIALIZED_MEMORY_PATTERN);
			byte* guard = pages + object_size;
			ASSERT(((intptr_t)guard & (PAGE_SIZE-1)) == 0);
			::mprotect(guard, PAGE_SIZE, PROT_NONE);
			return object;
#else
			if (alignment <= sizeof(void*)*2) {
				void* p = ::malloc(nbytes);
				if (!p)
					throw OutOfMemoryError();
				// TODO: Register for leak check;
				return p;
			} else {
				alignment = next_pow2(alignment);
				void* ptr;
				int r = posix_memalign(&ptr, alignment, nbytes);
				if (r == EINVAL) {
					throw BadAlignmentError();
				} else if (r == ENOMEM) {
					throw OutOfMemoryError();
				}
				detail::poison_memory((byte*)ptr, (byte*)ptr + nbytes, detail::UNINITIALIZED_MEMORY_PATTERN);
				// TODO: Register for leak check;
				return ptr;
			}
#endif
		}
		
		void* system_realloc(void* ptr, size_t old_size, size_t new_size, size_t alignment) {
#if DETECT_OVERRUN || DETECT_REUSE_AFTER_FREE
			if (old_size < new_size) {
				void* new_ptr = system_alloc(nbytes, alignment);
				memcpy(new_ptr, ptr, old_size);
				system_free(ptr);
				return new_ptr;
			} else {
				return ptr;
			}
#else
			if (alignment <= sizeof(void*)*2) {
				void* p = ::realloc(ptr, new_size);
				if (!p) {
					throw OutOfMemoryError();
				}
				// TODO: Register for leak check;
				return p;
			} else {
				if (old_size < new_size) {
					void* new_ptr = system_alloc(new_size, alignment);
					memcpy(new_ptr, ptr, old_size);
					system_free(ptr);
					return new_ptr;
				} else {
					return ptr;
				}
			}
#endif
		}
		
		void system_free(void* ptr, size_t poison_bytes) {
#if DETECT_OVERRUN || DETECT_REUSE_AFTER_FREE
            void* begin = (void*)((intptr_t)ptr & ~(PAGE_SIZE-1));
            size_t allocated_size = PAGE_SIZE;
#else
            void* begin = ptr;
            size_t allocated_size = poison_bytes;
#endif
            detail::poison_memory((byte*)begin, (byte*)begin + allocated_size, detail::FREED_MEMORY_PATTERN);
            
#if DETECT_REUSE_AFTER_FREE
            ::mprotect(begin, allocated_size, PROT_NONE);
#elsif DETECT_OVERRUN
            ::munmap(begin, allocated_size + PAGE_SIZE);
#else
            ::free(ptr);
#endif
		}
        
        void* system_alloc_large(size_t nbytes, size_t alignment, size_t& out_actually_allocated) {
            size_t object_size = PAGE_SIZE;
			while (nbytes > object_size) object_size += PAGE_SIZE;
            size_t to_allocate = object_size;
#if DETECT_OVERRUN
            to_allocate += PAGE_SIZE;
#endif
            byte* pages = (byte*)::mmap(nullptr, to_allocate, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
            if (pages == nullptr) {
				throw OutOfMemoryError();
			}
            void* object = pages;
            detail::poison_memory(pages, pages + object_size, detail::UNINITIALIZED_MEMORY_PATTERN);
#if DETECT_OVERRUN
            ::mprotect(pages + object_size, PAGE_SIZE, PROT_NONE);
#endif
            out_actually_allocated = object_size;
            return object;
        }
        
        void system_free_large(void* ptr, size_t actual_size) {
            byte* o = (byte*)ptr;
            detail::poison_memory(o, o + actual_size, detail::FREED_MEMORY_PATTERN);
#if DETECT_REUSE_AFTER_FREE
            ::mprotect(o, actual_size, PROT_NONE);
#elsif DETECT_OVERRUN
            ::munmap(ptr, actual_size + PAGE_SIZE);
#else
            ::munmap(ptr, actual_size);
#endif
        }
	}
	
	void* SystemAllocator::allocate(size_t nbytes, size_t alignment) {
		void* caller_ip;
		GET_CALLER_IP_VIRTUAL(caller_ip);
		void* ptr = system_alloc(nbytes, alignment);
		usage_ += system_alloc_size(ptr);
		tracker_.track_allocation(ptr, caller_ip);
		return ptr;
	}
	
	void* SystemAllocator::reallocate(void *ptr, size_t old_size, size_t new_size, size_t alignment) {
		void* caller_ip;
		GET_CALLER_IP_VIRTUAL(caller_ip);
		usage_ -= old_size;
		void* result = system_realloc(ptr, old_size, new_size, alignment);
		tracker_.track_free(ptr);
		tracker_.track_allocation(result, caller_ip);
		usage_ += new_size;
		return result;
	}
		
	void SystemAllocator::free(void* ptr) {
		if (ptr != nullptr) {
			size_t sz = system_alloc_size(ptr);
			usage_ -= sz;
			system_free(ptr);
			tracker_.track_free(ptr);
		}
	}
	
	void SystemAllocator::free(void* ptr, size_t nbytes) {
		if (ptr != nullptr) {
			size_t sz = system_alloc_size(ptr);
			usage_ -= sz;
			system_free(ptr, nbytes);
			tracker_.track_free(ptr);
		}
	}
    
    void* SystemAllocator::allocate_large(size_t nbytes, size_t alignment, size_t& out_actual_size) {
        void* ptr = system_alloc_large(nbytes, alignment, out_actual_size);
		usage_ += out_actual_size;
		void* caller_ip;
		GET_CALLER_IP_VIRTUAL(caller_ip);
		tracker_.track_allocation(ptr, caller_ip);
		return ptr;
    }
    
    void SystemAllocator::free_large(void *ptr, size_t actual_size) {
		usage_ -= actual_size;
        system_free_large(ptr, actual_size);
		tracker_.track_free(ptr);
    }
	
	void SystemAllocator::start_allocation_tracking() {
		tracker_.start();
	}
	
	void SystemAllocator::pause_allocation_tracking() {
		tracker_.pause();
	}
	
	Array<MemoryLeak> SystemAllocator::finish_allocation_tracking(IAllocator& leak_info_alloc) {
		Array<MemoryLeak> leaks(leak_info_alloc);
		tracker_.stop();
		tracker_.get_results(leaks);
		return move(leaks);
	}
	
	SystemAllocator::~SystemAllocator() {
		// TODO: Do leak checks.
	}
	
	static byte default_allocator_mem[sizeof(SystemAllocator)];
	
	SystemAllocator& default_allocator() {
		static SystemAllocator* sys_alloc = nullptr;
		if (sys_alloc == nullptr) {
			// Bypass operator new.
			sys_alloc = (SystemAllocator*)default_allocator_mem;
			new(sys_alloc) SystemAllocator;
		}
		return *sys_alloc;
	}
	
	LinearAllocator::LinearAllocator(size_t size) {
		if (size) {
			begin_ = (byte*)::mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
			if (begin_ == MAP_FAILED) {
				throw OutOfMemoryError();
			}
			detail::poison_memory(begin_, begin_+size, detail::UNALLOCATED_MEMORY_PATTERN);
		} else {
			begin_ = nullptr;
		}
		current_ = begin_;
		end_ = begin_ + size;
	}
	
	LinearAllocator::~LinearAllocator() {
		if (begin_) {
			detail::poison_memory(begin_, end_, detail::FREED_MEMORY_PATTERN);
			::munmap(begin_, end_-begin_);
		}
	}
	
	void* LinearAllocator::allocate(size_t nbytes, size_t alignment) {
		intptr_t c = (intptr_t)current_;
		if (alignment > 1) {
			alignment = next_pow2(alignment);
			intptr_t rest = c & (alignment-1);
			if (rest != alignment) {
				c += alignment - rest;
			}
		}
		current_ = (byte*)c;
		void* ptr = current_;
		current_ += nbytes;
		if (current_ < begin_ || current_ > end_) {
			throw OutOfMemoryError();
		}
		detail::poison_memory((byte*)ptr, (byte*)ptr + nbytes, detail::UNINITIALIZED_MEMORY_PATTERN);
		
		if (alignment) {
			ASSERT(((intptr_t)ptr & (alignment-1)) == 0);
		}
		
		return ptr;
	}
	
	static byte linear_allocator_mem[sizeof(LinearAllocator)];
	static const size_t STANDARD_LINEAR_ALLOCATOR_SIZE = 0x2000000; // 32 MiB
	
	LinearAllocator& scratch_linear_allocator() {
		// TODO: Thread-local
		static LinearAllocator* p = nullptr;
		if (p == nullptr) {
			p = new(linear_allocator_mem) LinearAllocator(STANDARD_LINEAR_ALLOCATOR_SIZE);
		}
		return *p;
	}
}
