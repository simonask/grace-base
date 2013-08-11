//
//  paths.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 07/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_paths_hpp
#define grace_paths_hpp

#include "base/string.hpp"

namespace grace {
	String get_bundle_resource_path(IAllocator& alloc = default_allocator());
	String get_executable_path(IAllocator& alloc = default_allocator());
}

#endif
