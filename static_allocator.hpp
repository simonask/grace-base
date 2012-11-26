//
//  static_allocator.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 26/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_static_allocator_hpp
#define falling_static_allocator_hpp

#include "memory/allocator.hpp"

namespace falling {
	LinearAllocator& static_allocator();
}

#define new_static new(falling::static_allocator())

#endif
