//
//  object_template.h
//  grace
//
//  Created by Simon Ask Ulsnes on 07/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef __grace__object_template__
#define __grace__object_template__

#include "base/basic.hpp"
#include "serialization/document.hpp"
#include "io/resource.hpp"

namespace grace {
	struct ObjectTemplate : public Resource {
		Document document;
		
		ObjectTemplate(IAllocator& alloc) : document(alloc) {}
	};
}

#endif /* defined(__grace__object_template__) */
