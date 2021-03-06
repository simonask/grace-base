//
//  resource_loader.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 26/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/resource_loader.hpp"
#include "io/resource_manager.hpp"

namespace grace {
	IAllocator& ResourceLoaderBase::allocator() const {
		return ResourceManager::allocator();
	}
}
