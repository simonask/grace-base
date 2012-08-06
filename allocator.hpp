//
//  allocator.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 06/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_allocator_hpp
#define falling_allocator_hpp

#include <memory>
#include <type_traits>
#include "base/basic.hpp"

void* malloc(size_t);
void* realloc(void* ptr, size_t);
void free(void*);

namespace falling {
	template <typename T> struct Allocator;
	
	template <typename T, bool IsPod> struct AllocatorReallocSpecialization;
	
	template <typename T> struct AllocatorReallocSpecialization<T, true> {
		inline T* allocate(size_t n) {
			return (T*)::malloc(sizeof(T)*n);
		}
		inline T* reallocate(T* ptr, size_t old_constructed_size, size_t new_alloc_size) {
			return (T*)::realloc(ptr, sizeof(T)*new_alloc_size);
		}
		inline void deallocate(T* ptr) {
			::free(ptr);
		}
	};
	
	template <typename T> struct AllocatorReallocSpecialization<T, false> {
		inline T* allocate(size_t n) {
			return (T*)::malloc(sizeof(T)*n);
		}
		inline T* reallocate(T* ptr, size_t old_constructed_size, size_t new_alloc_size) {
			T* new_ptr = (T*)::malloc(sizeof(T)*new_alloc_size);
			// move and destroy old values
			for (size_t i = 0; i < old_constructed_size; ++i) {
				new(new_ptr + i) T(std::move(ptr[i]));
				ptr[i].~T();
			}
			::free(ptr);
			return new_ptr;
		}
		inline void deallocate(T* ptr) {
			::free(ptr);
		}
	};
	
	template <typename T> struct Allocator : AllocatorReallocSpecialization<T, std::is_pod<T>::value> {
		
	};
}

#endif
