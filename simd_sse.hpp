//
//  simd_sse.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/05/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_simd_sse_hpp
#define falling_simd_sse_hpp

#if defined(__SSE__) && defined(__SSE2__)
#include <xmmintrin.h>
#include <mmintrin.h>
#if defined(__SSE3__)
#include <pmmintrin.h>
#endif
#endif


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

		inline void unaligned_load(fvec4& vec, const float32* p) { vec = _mm_loadu_ps(p); }
		inline void unaligned_load(ivec4& vec, const int32* p)   { vec = _mm_loadu_ps(reinterpret_cast<const float32*>(p)); }
		
		inline void aligned_load(fvec4& vec, const float32* p) { vec = _mm_load_ps(p); }
		
		inline void replicate(fvec4& vec, float32 f) { vec = _mm_load1_ps(&f); }
		
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
			return _mm_shuffle_ps(v, v, _MM_SHUFFLE(W_, Z_, Y_, X_));
		}
		
		inline fvec4 hadd3(fvec4 v) {
#if defined(__SSE3__)
			fvec4 tmp = shuffle<X,Y,X,Y>(v);
			tmp = _mm_hadd_ps(tmp, tmp); // Is hadd actually faster than shuffle?
			fvec4 tmp2 = shuffle<Z,Z,Z,Z>(v);
			return tmp + tmp2;
#else
			fvec4 a = shuffle<X,X,X,X>(v);
			fvec4 b = shuffle<Y,Y,Y,Y>(v);
			fvec4 c = shuffle<Z,Z,Z,Z>(v);
			return a + b + c;
#endif
		}
		
		inline fvec4 hadd4(fvec4 v) {
#if defined(__SSE3__)
			fvec4 tmp = _mm_hadd_ps(v, v);
			return _mm_hadd_ps(tmp, tmp);
#else
			// TODO: This could be done with one less shuffle, I suppose?
			fvec4 a = shuffle<X,X,X,X>(v);
			fvec4 b = shuffle<Y,Y,Y,Y>(v);
			fvec4 c = shuffle<Z,Z,Z,Z>(v);
			fvec4 d = shuffle<W,W,W,W>(v);
			return a + b + c + d;
#endif
		}
		
		inline fvec4 sqrt(fvec4 vec) {
			return _mm_sqrt_ps(vec);
		}
		
		
		inline fvec2 hadd2(fvec2 v) {
			fvec2 r;
			r[0] = v[0] + v[1];
			r[1] = r[0];
			return r;
		}
		
		
	}
}

#endif
