//
//  array_utils.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/02/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_array_utils_hpp
#define grace_array_utils_hpp

#include "base/basic.hpp"
#include "memory/allocator.hpp"
#include "base/type_traits.hpp"

namespace grace {
	/*
	 Reallocate array memory, with overallocation.
	 Returns: New data.
	 The value passed in "data" must be replaced by the return value.
	*/
	template <typename T>
	inline T* resize_allocation(IAllocator& allocator, T* data, size_t* inout_alloc_size, size_t constructed_size, size_t new_size, const size_t overallocation_dividend = 3, const size_t overallocation_divisor = 2) {
		ASSERT(overallocation_dividend >= overallocation_divisor);
		if (new_size > *inout_alloc_size) {
			size_t req_size = new_size;
			if (new_size * sizeof(T) < 0x1000) { // don't overallocate >4K (page size)
				req_size *= overallocation_dividend;
				req_size += req_size & 1; // round up
				req_size /= overallocation_divisor;
			}
			ASSERT(req_size >= new_size);
			T* new_data;
			if (IsTriviallyCopyable<T>::Value) {
				new_data = (T*)allocator.reallocate(data, sizeof(T) * (*inout_alloc_size), sizeof(T) * req_size, alignof(T));
			} else {
				new_data = (T*)allocator.allocate(sizeof(T)*req_size, alignof(T));
				for (size_t i = 0; i < constructed_size; ++i) {
					new(new_data+i) T(move(data[i]));
					data[i].~T();
				}
				allocator.free(data, sizeof(T) * (*inout_alloc_size));
			}
			*inout_alloc_size = req_size;
			return new_data;
		} else if (new_size < *inout_alloc_size) {
			// TODO: Downsize.
		}
		return data;
	}
}

#endif
