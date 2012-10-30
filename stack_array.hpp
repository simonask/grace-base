//
//  stack_array.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_stack_array_hpp
#define falling_stack_array_hpp

#include "base/array_ref.hpp"
#include <alloca.h>

namespace falling {
	template <typename T>
	struct StackArrayHolder {
		T* begin;
		T* end;
		StackArrayHolder(T* begin, T* end) : begin(begin), end(end) {
			construct_range(begin, end);
		}
		~StackArrayHolder() {
			destruct_range(begin, end);
		}
	};
	
#define DEFINE_STACK_ARRAY(T, NAME, SIZE) \
	T* NAME ## _data_ ## __LINE__ = (T*)alloca(sizeof(T) * SIZE); \
	StackArrayHolder<T> NAME ## _holder_ ## __LINE__(NAME ## _data_ ## __LINE__, NAME ## _data_ ## __LINE__ + SIZE); \
	ArrayRef<T> NAME(NAME ## _data_ ## __LINE__, NAME ## _data_ ## __LINE__ + SIZE)

}

#endif
