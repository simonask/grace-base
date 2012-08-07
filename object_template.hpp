//
//  object_template.h
//  falling
//
//  Created by Simon Ask Ulsnes on 07/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef __falling__object_template__
#define __falling__object_template__

#include "base/basic.hpp"
#include "serialization/yaml_archive.hpp"
#include "io/resource.hpp"

namespace falling {
	struct ObjectTemplate : public Resource {
		YAMLArchive archive;
	};
}

#endif /* defined(__falling__object_template__) */
