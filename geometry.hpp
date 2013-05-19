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
	template <typename T>
	struct TRect {
		using V = TVector<T, 2>;
		using Self = TRect<T>;
		TRect() {}
		TRect(T x, T y, T w, T h) : origin(x, y), size(w, h) {}
		TRect(V origin, V size) : origin(origin), size(size) {}
		TRect(const Self& other) = default;
		Self& operator=(const Self&) = default;
		
		static TRect<T> zero() {
			return TRect<T>(V::zero(), V::zero());
		}
		
		V origin;
		V size;
		
		V intersection_area(Self other) const {
			auto self_end = origin + size;
			auto other_end = other.origin + other.size;
			auto end_xs = shuffle2<0, X, 1, X>(self_end, other_end);
			auto end_ys = shuffle2<0, Y, 1, Y>(self_end, other_end);
			auto origin_xs = shuffle2<0, X, 1, X>(origin, other.origin);
			auto origin_ys = shuffle2<0, Y, 1, Y>(origin, other.origin);
			auto overlap = max(V::zero(), min(end_xs, end_ys) - max(origin_xs, origin_ys));
			return overlap;
		}
		
		Self intersection(Self other) const {
			V isize = intersection_area(other);
			V iorigin = max(origin, other.origin);
			// Return 0-rect if no intersection
			iorigin = select(isize == V::zero(), V::zero(), iorigin);
			return Self(iorigin, size);
		}
		
		bool intersects(V coordinate) const {
			auto end = origin + size;
			auto m0 = coordinate >= origin;
			auto m1 = coordinate < end;
			auto bools = (m0 & m1) & V::MaskVector::replicate(1);
			return bools.sum() == 2;
		}
		
		Self join(Self other) const {
			return *this + other;
		}
		
		Self operator+(Self other) const {
			Self s = *this;
			s += other;
			return s;
		}
		Self& operator+=(Self other) {
			auto other_end = other.origin + other.size;
			auto self_end = origin + size;
			auto biggest_end = max(self_end, other_end);
			origin = min(origin, other.origin);
			size = biggest_end - origin;
			return *this;
		}
	};
	
	using Rect = TRect<float32>;
	
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
	
	inline matrix33 make_2d_rotation_matrix33(float32 theta, vec2 around_offset) {
		float32 cosv = cosf(theta);
		float32 sinv = sinf(theta);
		vec3 r1 = { cosv, sinv, 0.f };
		vec3 r2 = { -sinv, cosv, 0.f };
		vec3 r3 = vec3::zero();
		auto m1 = make_2d_translation_matrix33(-around_offset);
		auto m2 = matrix33::from_rows({r1, r2, r3});
		auto m3 = make_2d_translation_matrix33(around_offset);
		return m1 * m2 * m3;
	}
	
	inline matrix33 make_2d_scaling_matrix(vec2 scale_axes) {
		matrix33 scale = matrix33::identity();
		scale.row_at(0).x = scale_axes.x;
		scale.row_at(1).y = scale_axes.y;
		return scale;
	}
	
	inline matrix33 make_2d_transform_matrix(vec2 translation, vec2 scale, float32 rotation) {
		// TODO: Optimize
		return make_2d_scaling_matrix(scale) * make_rotation_matrix33_z(rotation) * make_2d_translation_matrix33(translation);
	}
}

#endif
