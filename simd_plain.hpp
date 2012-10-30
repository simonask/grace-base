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
		
		SIMD_DEFINE_VECTOR_TYPE(1, int32,  int32,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(2, int32,  ivec2,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(3, int32,  ivec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(4, int32,  ivec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(1, int64,  int64,   uint64)
		SIMD_DEFINE_VECTOR_TYPE(2, int64,  ilvec2,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(3, int64,  ilvec4,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(4, int64,  ilvec4,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(1, uint32, uint32,  uint32)
		SIMD_DEFINE_VECTOR_TYPE(2, uint32, uvec2,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(3, uint32, uvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(4, uint32, uvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(1, uint64, uint64,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(2, uint64, ulvec2,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(3, uint64, ulvec4,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(4, uint64, ulvec4,  uint64)
		
		SIMD_DEFINE_VECTOR_TYPE(1, float32, float32, uint32)
		SIMD_DEFINE_VECTOR_TYPE(2, float32, fvec2,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(3, float32, fvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(4, float32, fvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(1, float64, float64, uint64)
		SIMD_DEFINE_VECTOR_TYPE(2, float64, flvec2,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(3, float64, flvec4,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(4, float64, flvec4,  uint64)
		
		inline void unaligned_load(fvec4& vec, const float32* data) {
			vec[0] = data[0];
			vec[1] = data[1];
			vec[2] = data[2];
			vec[3] = data[3];
		}
		
		inline void replicate(fvec4& vec, float32 f) {
			vec[0] = vec[1] = vec[2] = vec[3] = f;
		}
		
		template <typename From, typename To>
		ALWAYS_INLINE void convert2(From from, To& to) {
			to[0] = from[0];
			to[1] = from[1];
		}
		ALWAYS_INLINE void convert(ivec2 from, fvec2& to) { convert2(from, to); }
		ALWAYS_INLINE void convert(uvec2 from, fvec2& to) { convert2(from, to); }
		ALWAYS_INLINE void convert(fvec2 from, ivec2& to) { convert2(from, to); }
		ALWAYS_INLINE void convert(fvec2 from, uvec2& to) { convert2(from, to); }
		
		inline uvec4 cmp_lt( fvec4 a, fvec4 b) { return a < b;  }
		inline uvec4 cmp_lte(fvec4 a, fvec4 b) { return a <= b; }
		inline uvec4 cmp_gt( fvec4 a, fvec4 b) { return a > b;  }
		inline uvec4 cmp_gte(fvec4 a, fvec4 b) { return a >= b; }
		inline uvec4 cmp_eq( ivec4 a, ivec4 b) { return a == b; }
		inline uvec4 cmp_neq(ivec4 a, ivec4 b) { return a != b; }
		inline uvec4 cmp_eq( uvec4 a, uvec4 b) { return a == b; }
		inline uvec4 cmp_neq(uvec4 a, uvec4 b) { return a != b; }

		inline uvec2 cmp_lt( fvec2 a, fvec2 b) { return a < b;  }
		inline uvec2 cmp_lte(fvec2 a, fvec2 b) { return a <= b; }
		inline uvec2 cmp_gt( fvec2 a, fvec2 b) { return a > b;  }
		inline uvec2 cmp_gte(fvec2 a, fvec2 b) { return a >= b; }
		inline uvec2 cmp_eq( ivec2 a, ivec2 b) { return a == b; }
		inline uvec2 cmp_neq(ivec2 a, ivec2 b) { return a != b; }
		inline uvec2 cmp_eq( uvec2 a, uvec2 b) { return a == b; }
		inline uvec2 cmp_neq(uvec2 a, uvec2 b) { return a != b; }
		
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
			return (fvec4){::sqrtf(vec[0]), ::sqrtf(vec[1]), ::sqrtf(vec[2]), ::sqrtf(vec[3])};
		}
		
		inline fvec4 rsqrt(fvec4 vec) {
			return (fvec4){1.f/vec[0], 1.f/vec[1], 1.f/vec[2], 1.f/vec[3]};
		}
		
		template <size_t N, typename V>
		inline V hadd(V v) {
			V r;
			for (size_t i = 1; i < N; ++i) {
				r[0] += r[i];
			}
			for (size_t i = 1; i < N; ++i) {
				r[i] = r[0];
			}
			return r;
		}
		
		inline uvec2 hadd2u(uvec2 v) {
			return hadd<2>(v);
		}
		inline uvec4 hadd3u(uvec4 v) {
			return hadd<3>(v);
		}
		inline uvec4 hadd4u(uvec4 v) {
			return hadd<4>(v);
		}
		inline ivec2 hadd2i(ivec2 v) {
			return hadd<2>(v);
		}
		inline ivec4 hadd3i(ivec4 v) {
			return hadd<3>(v);
		}
		inline ivec4 hadd4i(ivec4 v) {
			return hadd<4>(v);
		}
		
		inline int32 abs(int32 n) {
			return n > 0 ? n : -n;
		}
		
		inline fvec4 abs(fvec4 v) {
			return (fvec4){fabsf(v[0]), fabsf(v[1]), fabsf(v[2]), fabsf(v[3])};
		}
		
		inline uvec2 abs(uvec2 v) {
			return v;
		}
		
		inline ivec2 abs(ivec2 v) {
			return (ivec2){abs(v[0]), abs(v[1])};
		}
		
		inline fvec4 neg(fvec4 v) {
			return (fvec4){-v[0], -v[1], -v[2], -v[3]};
		}
	}
}

#endif
