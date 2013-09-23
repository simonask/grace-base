//
//  resource.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 20/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/resource.hpp"
#include "io/resource_manager.hpp"

namespace grace {
	IInputStream* StreamingResource::create_reader(IAllocator& alloc) const {
		return ResourceManager::create_reader_for_resource_id(alloc, resource_id());
	}
}
