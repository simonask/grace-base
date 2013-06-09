//
//  static_allocator.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 26/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_static_allocator_hpp
#define grace_static_allocator_hpp

#include "memory/allocator.hpp"

namespace grace {
	LinearAllocator& static_allocator();
}

#define new_static new(grace::static_allocator())

#endif
