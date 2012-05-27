//
//  simd_plain.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/05/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_simd_plain_hpp
#define falling_simd_plain_hpp

#include <math.h>

namespace falling {
	namespace simd {
		typedef int32   __attribute__((vector_size(8 ))) ivec2;
		typedef int32   __attribute__((vector_size(16))) ivec4;
		typedef int64   __attribute__((vector_size(16))) ilvec2;
		typedef int64   __attribute__((vector_size(32))) ilvec4;
		typedef uint32  __attribute__((vector_size(8 ))) uvec2;
		typedef uint32  __attribute__((vector_size(16))) uvec4;
		typedef uint64  __attribute__((vector_size(16))) ulvec2;
		typedef uint64  __attribute__((vector_size(32))) ulvec4;
		typedef float32 __attribute__((vector_size(8 ))) fvec2;
		typedef float32 __attribute__((vector_size(16))) fvec4;
		typedef float64 __attribute__((vector_size(16))) flvec2;
		typedef float64 __attribute__((vector_size(32))) flvec4;
		
		inline void unaligned_load(fvec4& vec, const float32* data) {
			std::copy(data, data+4, &vec);
		}
		
		inline void replicate(fvec4& vec, float32 f) {
			vec[0] = vec[1] = vec[2] = vec[3] = f;
		}
		
		inline uvec4 cmp_lt(fvec4 a, fvec4 b)  { return a < b; }
		inline uvec4 cmp_lte(fvec4 a, fvec4 b) { return a <= b; }
		inline uvec4 cmp_gt(fvec4 a, fvec4 b)  { return a > b; }
		inline uvec4 cmp_gte(fvec4 a, fvec4 b) { return a >= b; }
		inline uvec4 cmp_eq(ivec4 a, ivec4 b) { return a == b; }
		inline uvec4 cmp_neq(ivec4 a, ivec4 b) { return a != b; }
		inline uvec4 cmp_eq(uvec4 a, uvec4 b) { return a == b; }
		inline uvec4 cmp_neq(uvec4 a, uvec4 b) { return a != b; }
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_ = W>
		fvec4 shuffle(fvec4 v) {
			fvec4 r;
			r[0] = v[(size_t)X_];
			r[1] = v[(size_t)Y_];
			r[2] = v[(size_t)Z_];
			r[3] = v[(size_t)W_];
			return r;
		}
		
		inline fvec4 hadd3(fvec4 vec) {
			float32 f = vec[0] + vec[1] + vec[2];
			return (fvec4){f, f, f, 0};
		}
		
		inline fvec4 hadd4(fvec4 vec) {
			float32 f = vec[0] + vec[1] + vec[2] + vec[3];
			return (fvec4){f, f, f, f};
		}
		
		inline fvec2 hadd2(fvec2 vec) {
			float32 f = vec[0] + vec[1];
			return (fvec2){f, f};
		}
		
		inline fvec4 sqrt(fvec4 vec) {
			float32 f = ::sqrtf(vec[0]);
			return (fvec4){f, f, f, f};
		}
	}
}

#endif
