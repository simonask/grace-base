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
#include "base/geometry.hpp"

namespace falling {
	class SpatialObject : public Object {
		REFLECT;
	public:
		virtual vec2 get_focus_point() const; // default: middle of bounds
		
		vec2 position;
		vec2 size;
		Rect bounds; // x, y, w, h
	};
}

#endif
