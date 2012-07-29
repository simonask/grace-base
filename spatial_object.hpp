//
//  spatial_object.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_spatial_object_hpp
#define falling_spatial_object_hpp

#include "object/object.hpp"
#include "base/vector.hpp"

namespace falling {
	class SpatialObject : public Object {
		REFLECT;
	public:
		vec2 position;
		vec2 size;
		vec4 bounds; // x, y, w, h
	};
}

#endif
