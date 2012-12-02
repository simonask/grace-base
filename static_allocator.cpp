//
//  static_allocator.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 26/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "memory/static_allocator.hpp"

namespace falling {
	static const size_t STATIC_MEMORY_ARENA = 0x080000; // 512 KiB

	namespace {
		byte static_allocator_mem[sizeof(LinearAllocator)];
	}
	
	LinearAllocator& static_allocator() {
		static LinearAllocator* p = nullptr;
		if (p == nullptr) {
			p = new(static_allocator_mem) LinearAllocator(STATIC_MEMORY_ARENA);
		}
		return *p;
	}
}