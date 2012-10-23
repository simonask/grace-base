//
//  geometry.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 12/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_geometry_hpp
#define falling_geometry_hpp

#include "base/vector.hpp"

namespace falling {
	struct Rect {
		Rect() {}
		Rect(float32 x, float32 y, float32 w, float32 h) : origin(x, y), size(w, h) {}
		Rect(vec2 origin, vec2 size) : origin(origin), size(size) {}
	
		vec2 origin;
		vec2 size;
		
		static Rect zero() {
			return Rect(vec2::zero(), vec2::zero());
		}
		
		// TODO: intersect, union, etc.
	};
}

#endif
