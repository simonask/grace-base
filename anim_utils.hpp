//
//  anim_utils.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_anim_utils_hpp
#define falling_anim_utils_hpp

#include "base/basic.hpp"
#include "base/vector.hpp"
#include "base/matrix.hpp"
#include "base/color.hpp"

namespace falling {
	inline float32 anim_lerp(float32 current, float32 target, float64 progress_ratio) {
		return current + ((target - current) * progress_ratio);
	}
	
	inline float32 anim_lerp(float32 current, float32 target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline float64 anim_lerp(float64 current, float64 target, float64 progress_ratio) {
		return current + ((target - current) * progress_ratio);
	}
	
	inline float64 anim_lerp(float64 current, float64 target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline int32 anim_lerp(int32 current, int32 target, float64 progress_ratio, float64& progress_accum) {
		progress_accum += progress_ratio;
		int32 progress_by = (target - current) * progress_accum;
		if (progress_by == 0) {
			return current;
		} else {
			progress_accum -= (float64)progress_by / ((float64)target - (float64)current);
			return current + progress_by;
		}
	}
	
	inline int64 anim_lerp(int64 current, int64 target, float64 progress_ratio, float64& progress_accum) {
		progress_accum += progress_ratio;
		int64 progress_by = (target - current) * progress_accum;
		if (progress_by == 0) {
			return current;
		} else {
			progress_accum -= (float64)progress_by / ((float64)target - (float64)current);
			return current + progress_by;
		}
	}
	
	inline vec2 anim_lerp(vec2 current, vec2 target, float64 progress_ratio) {
		return current + ((target - current) * vec2::replicate(progress_ratio));
	}

	inline vec2 anim_lerp(vec2 current, vec2 target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline vec3 anim_lerp(vec3 current, vec3 target, float64 progress_ratio) {
		return current + ((target - current) * vec3::replicate(progress_ratio));
	}

	inline vec3 anim_lerp(vec3 current, vec3 target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline vec4 anim_lerp(vec4 current, vec4 target, float64 progress_ratio) {
		return current + ((target - current) * vec4::replicate(progress_ratio));
	}

	inline vec4 anim_lerp(vec4 current, vec4 target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline matrix22 anim_lerp(const matrix22& current, const matrix22& target, float64 progress_ratio) {
		return current + ((target - current) * progress_ratio);
	}

	inline matrix22 anim_lerp(const matrix22& current, const matrix22& target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline matrix33 anim_lerp(const matrix33& current, const matrix33& target, float64 progress_ratio) {
		return current + ((target - current) * progress_ratio);
	}

	inline matrix33 anim_lerp(const matrix33& current, const matrix33& target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline matrix43 anim_lerp(const matrix43& current, const matrix43& target, float64 progress_ratio) {
		return current + ((target - current) * progress_ratio);
	}

	inline matrix43 anim_lerp(const matrix43& current, const matrix43& target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline matrix44 anim_lerp(const matrix44& current, const matrix44& target, float64 progress_ratio) {
		return current + ((target - current) * progress_ratio);
	}

	inline matrix44 anim_lerp(const matrix44& current, const matrix44& target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline Color anim_lerp(Color current, Color target, float64 progress_ratio) {
		return Color(anim_lerp(current.components(), target.components(), progress_ratio));
	}

	inline Color anim_lerp(Color current, Color target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
	
	inline CompactColor anim_lerp(CompactColor current, CompactColor target, float64 progress_ratio) {
		return anim_lerp((Color)current, (Color)target, progress_ratio);
	}

	inline CompactColor anim_lerp(CompactColor current, CompactColor target, float64 progress_ratio, float64& _unused) {
		return anim_lerp(current, target, progress_ratio);
	}
}

#endif
