//
//  spatial_object.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "object/spatial_object.hpp"
#include "type/vector_type.hpp"
#include "object/reflect.hpp"
#include "base/geometry.hpp"

namespace falling {
	BEGIN_TYPE_INFO(SpatialObject)
	property(&SpatialObject::position, "position", "World Position");
	END_TYPE_INFO()
	
	vec2 SpatialObject::get_focus_point() const {
		return position + (size / vec2::two());
	}
	
	matrix33 SpatialObject::transform() const {
		return make_2d_transform_matrix(position, scale, rotation);
	}
}
