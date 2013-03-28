//
//  fixed_allocator.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 25/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_fixed_allocator_hpp
#define falling_fixed_allocator_hpp

#include "memory/allocator.hpp"

namespace falling {
	template <size_t Max, size_t Alignment = 16>
	class FixedAllocator : public IAllocator {
	public:
		static_assert(Max >= sizeof(void**), "FixedAllocator must allocate objects of sizeof(void*) or larger.");
		
		explicit FixedAllocator(IAllocator& base) : base_(base) {}
		FixedAllocator(FixedAllocator&& other) : base_(other.base_) {
			free_list_ = other.free_list_;
			other.free_list_ = nullptr;
			usage_ = other.usage_;
			other.usage_ = 0;
		}
		~FixedAllocator() {
			while (free_list_) {
				void* tmp = *free_list_;
				base_.free(free_list_, Max);
				free_list_ = (void**)tmp;
			}
		}
		
		void* allocate(size_t nbytes, size_t alignment) final {
			ASSERT(nbytes <= Max);
			ASSERT(alignment <= Alignment);
			usage_ += Max;
			if (free_list_) {
				void* p = *free_list_;
				free_list_ = *(void***)p;
				return p;
			} else {
				return base_.allocate(Max, Alignment);
			}
		}
		
		void free(void* ptr, size_t nbytes) final {
			if (ptr == nullptr) return;
			ASSERT(nbytes <= Max);
			usage_ -= Max;
			*(void**)ptr = free_list_;
			free_list_ = (void**)ptr;
		}
		
		void* reallocate(void* ptr, size_t new_size, size_t old_size, size_t alignment) final {
			ASSERT(new_size <= Max);
			ASSERT(alignment <= Alignment);
			return ptr;
		}
		
		void* allocate_large(size_t nbytes, size_t alignment, size_t& actual) final {
			actual = Max;
			return allocate(nbytes, alignment);
		}
		
		void free_large(void* ptr, size_t actual_size) final {
			ASSERT(actual_size <= Max);
			this->free(ptr, actual_size);
		}
		
		size_t usage() const { return usage_; }
		size_t capacity() const { return base_.capacity(); }
	private:
		IAllocator& base_;
		void** free_list_ = nullptr;
		size_t usage_ = 0;
	};
}

#endif
