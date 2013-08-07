//
//  util.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/08/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef grace_util_hpp
#define grace_util_hpp

#include "geometry/vector.hpp"
#include "geometry/matrix.hpp"
#include "geometry/rect.hpp"
#include "geometry/units.hpp"
#include "base/stack_array.hpp"

namespace grace {
	inline Maybe<vec2> line_intersection(vec2 p0, vec2 p1, vec2 q0, vec2 q1) {
		vec2 s0 = p1 - p0;
		vec2 s1 = q1 - q0;
		
		// divisor = replicate(-s1.x * s0.y + s0.x * s1.y)
		vec2 div_a = shuffle2<0, X, 1, X>(-s1, s0);
		vec2 div_b = shuffle2<0, Y, 1, Y>(s0, s1);
		vec2 div_c = div_a * div_b;
		vec2 divisor = div_c.sumv();
		
		//auto s = (-s0.y * (p0.x - q0.x) + s0.x * (p0.y - q0.y)) / (-s1.x * s0.y + s0.x * s1.y);
		//auto t = ( s1.x * (p0.y - q0.y) - s1.y * (p0.x - q0.x)) / (-s1.x * s0.y + s0.x * s1.y);
		vec2 a = shuffle2<0, Y, 1, X>(-s0, s1);
		vec2 b = p0 - q0;
		vec2 c = a * b;
		vec2 d = shuffle2<0, X, 1, Y>(s0, -s1);
		vec2 e = shuffle<Y, X>(b);
		vec2 f = d * e;
		vec2 st = (c + f) / divisor;
		
		
		// if s >= 0 && s <= 1 && t >= 0 && t <= 1
		const auto zero = vec2::zero();
		const auto one = vec2::one();
		auto m = st >= zero & st <= one;
		if (m.all_true()) {
			vec2 scalar = shuffle<Y, Y>(st);
			// return {p0.x + t * s0.x, p0.y + t * s0.y}
			return p0 + scalar * s0;
		}
		return Nothing;
	}
	
	template <typename InputIterator>
	void project_polygon(vec2 axis, InputIterator p_begin, InputIterator p_end, float32& begin, float32& end) {
		begin = HUGE_VALF;
		end = -HUGE_VALF;
		for (auto it = p_begin; it != p_end; ++it) {
			float32 dot = it->dot(axis);
			if (dot < begin) {
				begin = dot;
			} else if (dot > end) {
				end = dot;
			}
		}
	}
	
	inline float32 interval_distance(float32 a0, float32 a1, float32 b0, float32 b1) {
		if (a0 < b0) {
			return b0 - a1;
		} else {
			return a0 - b1;
		}
	}
	
	template <typename PathA, typename PathB>
	bool polygon_intersection(const PathA& a, const PathB& b, const matrix33& transform_a, const matrix33& transform_b) {
		if (a.size() == 0 || b.size() == 0) return false;
		
		// Quickly check bounds overlap
		Rect bounds_a = {a[0].position, Rect::V::zero()};
		Rect bounds_b = {b[0].position, Rect::V::zero()};
		for (auto& p: a) {
			bounds_a += p.position;
		}
		for (auto& p: b) {
			bounds_b += p.position;
		}
		if (bounds_a.intersection_area(bounds_b) == approximately(Rect::V::zero(), 0.000001f)) {
			return false;
		}
		
		// They overlap, do polygon overlap check
		
		using VTa = typename PathA::ValueType;
		using VTb = typename PathB::ValueType;
		using Va = decltype(get_member_type(&VTa::position));
		using Vb = decltype(get_member_type(&VTb::position));
		
		DEFINE_STACK_ARRAY(Va, transformed_positions_a, a.size());
		for (size_t i = 0; i < a.size(); ++i) {
			transformed_positions_a[i] = matrix_transform(transform_a, a[i].position);
		}
		DEFINE_STACK_ARRAY(Vb, transformed_positions_b, b.size());
		for (size_t i = 0; i < b.size(); ++i) {
			transformed_positions_b[i] = matrix_transform(transform_b, b[i].position);
		}
		
		for (size_t i = 0; i < a.size(); ++i) {
			vec2 p0 = transformed_positions_a[i].position;
			vec2 p1 = transformed_positions_a[(i + 1) % a.size()].position;
			vec2 axis = (p1 - p0).normalize();
			float32 begin_a, end_a, begin_b, end_b;
			project_polygon(axis, transformed_positions_a.begin(), transformed_positions_a.end(), begin, end);
			project_polygon(axis, transformed_positions_b.begin(), transformed_positions_b.end(), begin, end);
			
			if (interval_distance(begin_a, end_a, begin_b, end_b) > 0) {
				return false;
			}
		}
		for (size_t i = 0; i < b.size(); ++i) {
			vec2 p0 = transformed_positions_b[i].position;
			vec2 p1 = transformed_positions_b[(i + 1) % a.size()].position;
			vec2 axis = (p1 - p0).normalize();
			float32 begin_a, end_a, begin_b, end_b;
			project_polygon(axis, transformed_positions_a.begin(), transformed_positions_a.end(), begin, end);
			project_polygon(axis, transformed_positions_b.begin(), transformed_positions_b.end(), begin, end);
			
			if (interval_distance(begin_a, end_a, begin_b, end_b) > 0) {
				return false;
			}
		}
	}
	
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
	inline matrix33 make_rotation_matrix33_z(Radians theta) {
		vec3 v = vec3::replicate(theta);
		vec3 cosv = cos(v);
		vec3 sinv = sin(v);
		vec3 msinv = -sinv;
		vec3 r1 = shuffle2<0, X, 1, Y, 0, IgnoreAxis>(cosv, sinv);
		vec3 r2 = shuffle2<0, X, 1, Y, 0, IgnoreAxis>(msinv, cosv);
		vec3 r3 = { 0.f, 0.f, 1.f };
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
	
	inline matrix33 make_2d_rotation_matrix33(Degrees deg, vec2 around_offset) {
		Radians rad = deg2rad(deg);
		float32 cosv = cos(rad);
		float32 sinv = sin(rad);
		vec3 r1 = { cosv, sinv, 0.f };
		vec3 r2 = { -sinv, cosv, 0.f };
		vec3 r3 = { 0.f, 0.f, 1.f };
		auto m1 = make_2d_translation_matrix33(-around_offset);
		auto m2 = matrix33::from_rows({r1, r2, r3});
		auto m3 = make_2d_translation_matrix33(around_offset);
		return m3 * m2 * m1;
	}
	
	inline matrix33 make_2d_scaling_matrix(vec2 scale_axes) {
		matrix33 scale = matrix33::identity();
		scale.row_at(0).x = scale_axes.x;
		scale.row_at(1).y = scale_axes.y;
		return scale;
	}
	
	inline matrix33 make_2d_transform_matrix(vec2 translation, vec2 scale, Degrees rotation) {
		// TODO: Optimize
		matrix33 m = make_2d_scaling_matrix(scale);
		m = m * make_2d_translation_matrix33(translation);
		m = m * make_rotation_matrix33_z(deg2rad(rotation));
		return m;
	}
	
	// Rotates around center
	inline Rect bounds_of_rotated_rectangle(const Rect& r, Degrees deg) {
		const auto rad = deg2rad(deg);
		const auto cosr = std::abs(std::cos(rad));
		const auto sinr = std::abs(std::sin(rad));
		const auto sincosv = vec2{sinr, cosr};
		const auto cossinv = vec2{cosr, sinr};
		const auto new_width = (r.size * cossinv).sumv();
		const auto new_height = (r.size * sincosv).sumv();
		const auto new_size = shuffle2<0, X, 1, Y>(new_width, new_height);
		const auto new_origin = r.origin;
		return Rect{new_origin, new_size};
	}
}

#endif
