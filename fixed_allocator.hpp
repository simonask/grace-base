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
		~FixedAllocator() {
			while (free_list_) {
				void* tmp = *free_list_;
				base_.free(free_list_);
				free_list_ = (void**)tmp;
			}
		}
		
		void* allocate(size_t nbytes, size_t alignment) final {
			ASSERT(nbytes <= Max);
			ASSERT(alignment <= Alignment);
			if (free_list_) {
				void* p = *free_list_;
				free_list_ = *(void***)p;
				return p;
			} else {
				return base_.allocate(Max, Alignment);
			}
		}
		
		void free(void* ptr) final {
			*(void**)ptr = free_list_;
			free_list_ = (void**)ptr;
		}
		
		void* allocate_large(size_t nbytes, size_t alignment, size_t& actual) final {
			actual = Max;
			return allocate(nbytes, alignment);
		}
		
		void free_large(void* ptr, size_t actual_size) final {
			ASSERT(actual_size <= Max);
			this->free(ptr);
		}
	private:
		IAllocator& base_;
		void** free_list_ = nullptr;
	};
}

#endif
