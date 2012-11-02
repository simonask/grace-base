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
#include "base/matrix.hpp"
#include "base/math.hpp"
#include "type/type.hpp"

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
	
	namespace detail {
		const Type* build_rect_type();
	}
	
	template <>
	struct BuildTypeInfo<Rect> {
		static const Type* build() { return detail::build_rect_type(); }
	};
	
	inline matrix22 make_rotation_matrix22(float32 theta) {
		vec2 v = vec2::replicate(theta);
		vec2 cosv = cos(v);
		vec2 sinv = sin(v);
		vec2 msinv = -sinv;
		vec2 r1 = shuffle2<0, X, 1, Y>(cosv, sinv);
		vec2 r2 = shuffle2<0, X, 1, Y>(msinv, cosv);
		return matrix22::from_rows({r1, r2});
	}
	
	// TODO: Arbitrary rotation matrices for 3D-space
	inline matrix33 make_rotation_matrix33_z(float32 theta) {
		vec3 v = vec3::replicate(theta);
		vec3 cosv = cos(v);
		vec3 sinv = sin(v);
		vec3 msinv = -sinv;
		vec3 r1 = shuffle2<0, X, 1, Y, 0, IgnoreAxis>(cosv, sinv);
		vec3 r2 = shuffle2<0, X, 1, Y, 0, IgnoreAxis>(msinv, cosv);
		vec3 r3 = vec3::zero();
		r1.z = 0.f;
		r2.z = 0.f;
		r3.z = 1.f; // identity
		return matrix33::from_rows({r1, r2, r3});
	}
	
	inline matrix33 make_2d_translation_matrix33(vec2 translation) {
		matrix33 trans = matrix33::identity();
		trans.row_at(0).z = translation.x;
		trans.row_at(1).z = translation.y;
		ASSERT(trans.row_at(2).z == 1.f);
		return trans;
	}
}

#endif
