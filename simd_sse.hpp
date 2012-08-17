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

#include <cmath>

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

		ALWAYS_INLINE void unaligned_load(float32& v, const float32* p) { v = p[0]; }
		ALWAYS_INLINE void unaligned_load(fvec4& vec, const float32* p) { vec = _mm_loadu_ps(p); }
		ALWAYS_INLINE void unaligned_load(fvec2& vec, const float32* p) { vec[0] = p[0]; vec[1] = p[1]; }

		ALWAYS_INLINE void unaligned_load(int32& v, const int32* p) { v = p[0]; }
		ALWAYS_INLINE void unaligned_load(ivec4& vec, const int32* p) { vec = _mm_loadu_ps(reinterpret_cast<const float32*>(p)); }
		ALWAYS_INLINE void unaligned_load(ivec2& vec, const int32* p) { vec = _mm_set_pi32(p[0], p[1]); }

		ALWAYS_INLINE void unaligned_load(uint32& v, const uint32* p) { v = p[0]; }
		ALWAYS_INLINE void unaligned_load(uvec4& vec, const uint32* p) { vec = _mm_loadu_ps(reinterpret_cast<const float32*>(p)); }
		ALWAYS_INLINE void unaligned_load(uvec2& vec, const uint32* p) { vec = _mm_set_pi32(p[0], p[1]); }
		
		ALWAYS_INLINE void aligned_load(float32& v, const float32* p) { v = p[0]; }
		ALWAYS_INLINE void aligned_load(fvec4& vec, const float32* p) { vec = _mm_load_ps(p); }
		ALWAYS_INLINE void aligned_load(fvec2& vec, const float32* p) { vec[0] = p[0]; vec[1] = p[1]; }
		
		ALWAYS_INLINE void aligned_load(int32& v, const int32* p) { v = p[0]; }
		ALWAYS_INLINE void aligned_load(ivec4& vec, const int32* p) { vec = _mm_load_ps(reinterpret_cast<const float32*>(p)); }
		ALWAYS_INLINE void aligned_load(ivec2& vec, const int32* p) { vec = _mm_set_pi32(p[0], p[1]); }
		
		ALWAYS_INLINE void aligned_load(uint32& v, const uint32* p) { v = p[0]; }
		ALWAYS_INLINE void aligned_load(uvec4& vec, const uint32* p) { vec = _mm_load_ps(reinterpret_cast<const float32*>(p)); }
		ALWAYS_INLINE void aligned_load(uvec2& vec, const uint32* p) { vec = _mm_set_pi32(p[0], p[1]); }
		
		ALWAYS_INLINE void replicate(fvec4& vec, float32 f) { vec = _mm_load1_ps(&f); }
		
		template <typename From, typename To>
		ALWAYS_INLINE void convert2(From from, To& to) {
			to[0] = from[0];
			to[1] = from[1];
		}
		ALWAYS_INLINE void convert(ivec2 from, fvec2& to) { convert2(from, to); }
		ALWAYS_INLINE void convert(uvec2 from, fvec2& to) { convert2(from, to); }
		ALWAYS_INLINE void convert(fvec2 from, ivec2& to) { convert2(from, to); }
		ALWAYS_INLINE void convert(fvec2 from, uvec2& to) { convert2(from, to); }
		
		ALWAYS_INLINE void convert(ivec4 from, fvec4& to) {
			to = _mm_cvtepi32_ps(from);
		}
		
		ALWAYS_INLINE void convert(fvec4 from, ivec4& to) {
			to = _mm_cvtps_epi32(from);
		}
		
		ALWAYS_INLINE void convert(uvec4 from, fvec4& to) {
			// There is no _mm_cvtepu32_ps instruction, so do it manually, and leave the rest to the compiler.
			to[0] = from[0];
			to[1] = from[1];
			to[2] = from[2];
			to[3] = from[3];
		}
		
		ALWAYS_INLINE void convert(fvec4 from, uvec4& to) {
			// Likewise, there is no _mm_cvtps_epu32 instruction :(
			to[0] = from[0];
			to[1] = from[1];
			to[2] = from[2];
			to[3] = from[3];
		}
		
		ALWAYS_INLINE uvec4 cmp_lt(fvec4 a, fvec4 b)  { return a < b; }
		ALWAYS_INLINE uvec4 cmp_lte(fvec4 a, fvec4 b) { return a <= b; }
		ALWAYS_INLINE uvec4 cmp_gt(fvec4 a, fvec4 b)  { return a > b; }
		ALWAYS_INLINE uvec4 cmp_gte(fvec4 a, fvec4 b) { return a >= b; }
		ALWAYS_INLINE uvec4 cmp_eq(ivec4 a, ivec4 b) { return a == b; }
		ALWAYS_INLINE uvec4 cmp_neq(ivec4 a, ivec4 b) { return a != b; }
		ALWAYS_INLINE uvec4 cmp_eq(uvec4 a, uvec4 b) { return a == b; }
		ALWAYS_INLINE uvec4 cmp_neq(uvec4 a, uvec4 b) { return a != b; }

		ALWAYS_INLINE uvec2 cmp_lt(fvec2 a, fvec2 b)  { return a < b; }
		ALWAYS_INLINE uvec2 cmp_lte(fvec2 a, fvec2 b) { return a <= b; }
		ALWAYS_INLINE uvec2 cmp_gt(fvec2 a, fvec2 b)  { return a > b; }
		ALWAYS_INLINE uvec2 cmp_gte(fvec2 a, fvec2 b) { return a >= b; }
		ALWAYS_INLINE uvec2 cmp_eq(ivec2 a, ivec2 b) { return a == b; }
		ALWAYS_INLINE uvec2 cmp_neq(ivec2 a, ivec2 b) { return a != b; }
		ALWAYS_INLINE uvec2 cmp_eq(uvec2 a, uvec2 b) { return a == b; }
		ALWAYS_INLINE uvec2 cmp_neq(uvec2 a, uvec2 b) { return a != b; }

		ALWAYS_INLINE bool cmp_lt(float32 a, float32 b)  { return a < b; }
		ALWAYS_INLINE bool cmp_lte(float32 a, float32 b) { return a <= b; }
		ALWAYS_INLINE bool cmp_gt(float32 a, float32 b)  { return a > b; }
		ALWAYS_INLINE bool cmp_gte(float32 a, float32 b) { return a >= b; }
		ALWAYS_INLINE bool cmp_eq(int32 a, int32 b) { return a == b; }
		ALWAYS_INLINE bool cmp_neq(int32 a, int32 b) { return a != b; }
		ALWAYS_INLINE bool cmp_eq(uint32 a, uint32 b) { return a == b; }
		ALWAYS_INLINE bool cmp_neq(uint32 a, uint32 b) { return a != b; }
		
		ALWAYS_INLINE float32 min(float32 a, float32 b) { return a < b ? a : b; }
		ALWAYS_INLINE int32   min(int32 a,   int32 b)   { return a < b ? a : b; }
		ALWAYS_INLINE uint32  min(uint32 a,  uint32 b)  { return a < b ? a : b; }
		ALWAYS_INLINE fvec2 min(fvec2 a, fvec2 b) { return (fvec2){min(a[0], b[0]), min(a[1], b[1])}; }
		ALWAYS_INLINE ivec2 min(ivec2 a, ivec2 b) { return (ivec2){min(a[0], b[0]), min(a[1], b[1])}; }
		ALWAYS_INLINE uvec2 min(uvec2 a, uvec2 b) { return (uvec2){min(a[0], b[0]), min(a[1], b[1])}; }
		ALWAYS_INLINE fvec4 min(fvec4 a, fvec4 b) { return _mm_min_ps(a, b); }
		ALWAYS_INLINE ivec4 min(ivec4 a, ivec4 b) { return _mm_min_ps(a, b); }
		ALWAYS_INLINE uvec4 min(uvec4 a, uvec4 b) { return _mm_min_ps(a, b); }

		ALWAYS_INLINE float32 max(float32 a, float32 b) { return a < b ? a : b; }
		ALWAYS_INLINE int32   max(int32 a,   int32 b)   { return a < b ? a : b; }
		ALWAYS_INLINE uint32  max(uint32 a,  uint32 b)  { return a < b ? a : b; }
		ALWAYS_INLINE fvec2 max(fvec2 a, fvec2 b) { return (fvec2){max(a[0], b[0]), max(a[1], b[1])}; }
		ALWAYS_INLINE ivec2 max(ivec2 a, ivec2 b) { return (ivec2){max(a[0], b[0]), max(a[1], b[1])}; }
		ALWAYS_INLINE uvec2 max(uvec2 a, uvec2 b) { return (uvec2){max(a[0], b[0]), max(a[1], b[1])}; }
		ALWAYS_INLINE fvec4 max(fvec4 a, fvec4 b) { return _mm_max_ps(a, b); }
		ALWAYS_INLINE ivec4 max(ivec4 a, ivec4 b) { return _mm_max_ps(a, b); }
		ALWAYS_INLINE uvec4 max(uvec4 a, uvec4 b) { return _mm_max_ps(a, b); }

		
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_ = W>
		fvec4 shuffle(fvec4 v) {
			return _mm_shuffle_ps(v, v, _MM_SHUFFLE(W_, Z_, Y_, X_));
		}
		
		ALWAYS_INLINE fvec4 hadd3(fvec4 v) {
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
		
		ALWAYS_INLINE fvec4 hadd4(fvec4 v) {
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
		
		ALWAYS_INLINE fvec2 hadd2(fvec2 v) {
			fvec2 r;
			r[0] = v[0] + v[1];
			r[1] = r[0];
			return r;
		}
		
		ALWAYS_INLINE ivec2 hadd2i(ivec2 v) {
			ivec2 r;
			r[0] = v[0] + v[1];
			r[1] = r[0];
			return r;
		}
		
		ALWAYS_INLINE uvec2 hadd2u(uvec2 v) {
			uvec2 r;
			r[0] = v[0] + v[1];
			r[1] = r[0];
			return r;
		}
		
		ALWAYS_INLINE ivec4 hadd3i(ivec4 v) {
			int32 r = v[0] + v[1] + v[2];
			return ivec4{r, r, r, r};
		}
		
		ALWAYS_INLINE uvec4 hadd3u(uvec4 v) {
			uint32 r = v[0] + v[1] + v[2];
			return uvec4{r, r, r, r};
		}
		
		ALWAYS_INLINE ivec4 hadd4i(ivec4 v) {
			int32 r = v[0] + v[1] + v[2] + v[3];
			return ivec4{r, r, r, r};
		}
		
		ALWAYS_INLINE uvec4 hadd4u(uvec4 v) {
			uint32 r = v[0] + v[1] + v[2] + v[3];
			return uvec4{r, r, r, r};
		}
		
		ALWAYS_INLINE fvec4 sqrt(fvec4 vec) {
			return _mm_sqrt_ps(vec);
		}
		
		ALWAYS_INLINE fvec4 rsqrt(fvec4 vec) {
			return _mm_rsqrt_ps(vec);
		}
		
		ALWAYS_INLINE fvec2 sqrt(fvec2 vec) {
			// TODO: Is there a faster way to convert between fvec2 and fvec4?
			fvec4 v = fvec4{vec[0], vec[1], 0.f, 0.f};
			fvec4 result = sqrt(v);
			return fvec2{result[0], result[1]};
		}
		
		ALWAYS_INLINE fvec2 rsqrt(fvec2 vec) {
			// TODO: Is there a faster way to convert between fvec2 and fvec4?
			fvec4 v = fvec4{vec[0], vec[1], 0.f, 0.f};
			fvec4 result = rsqrt(v);
			return fvec2{result[0], result[1]};
		}
		
		ALWAYS_INLINE float32 sqrt(float32 f) {
			fvec4 v = _mm_set_ps1(f);
			v = sqrt(v);
			return v[0];
		}
		
		ALWAYS_INLINE float32 rsqrt(float32 f) {
			fvec4 v = _mm_set_ps1(f);
			v = rsqrt(v);
			return v[0];
		}
		
		ALWAYS_INLINE ivec2 sqrt(ivec2 v) {
			fvec2 result;
			convert(v, result);
			result = sqrt(result);
			convert(result, v);
			return v;
		}
		
		ALWAYS_INLINE ivec2 rsqrt(ivec2 v) {
			fvec2 result;
			convert(v, result);
			result = sqrt(result);
			convert(result, v);
			return v;
		}
		
		ALWAYS_INLINE uvec2 sqrt(uvec2 v) {
			fvec2 result;
			convert(v, result);
			result = sqrt(result);
			convert(result, v);
			return v;
		}
		
		ALWAYS_INLINE uvec2 rsqrt(uvec2 v) {
			fvec2 result;
			convert(v, result);
			result = sqrt(result);
			convert(result, v);
			return v;
		}

		ALWAYS_INLINE ivec4 sqrt(ivec4 v) {
			fvec4 result;
			convert(v, result);
			result = sqrt(result);
			convert(result, v);
			return v;
		}
		
		ALWAYS_INLINE ivec4 rsqrt(ivec4 v) {
			fvec4 result;
			convert(v, result);
			result = sqrt(result);
			convert(result, v);
			return v;
		}
		
		ALWAYS_INLINE uvec4 sqrt(uvec4 v) {
			fvec4 result;
			convert(v, result);
			result = sqrt(result);
			convert(result, v);
			return v;
		}
		
		ALWAYS_INLINE uvec4 rsqrt(uvec4 v) {
			fvec4  result;
			convert(v, result);
			result = sqrt(result);
			convert(result, v);
			return v;
		}
		
		
		static const uint32 SIGNMASK_FLOAT32 = 0x80000000;
		static const __m128 SIGNMASK_VEC4 = _mm_set1_epi32(SIGNMASK_FLOAT32);
		static const __m64 SIGNMASK_VEC2 = _mm_set1_pi32(SIGNMASK_FLOAT32);
		
		ALWAYS_INLINE fvec4 neg(fvec4 v) {
			fvec4 negval = _mm_xor_ps(v, SIGNMASK_VEC4);
			return negval;
		}
		
		ALWAYS_INLINE fvec2 neg(fvec2 v) {
			fvec2 negval = {-v[0], -v[1]};
			return negval;
		}
		
		ALWAYS_INLINE float32 neg(float32 f) {
			return -f;
		}
		
		ALWAYS_INLINE fvec4 abs(fvec4 v) {
			fvec4 absval = _mm_andnot_ps(SIGNMASK_VEC4, v);
			return absval;
		}
		
		ALWAYS_INLINE fvec2 abs(fvec2 v) {
			fvec2 absval = _mm_andnot_si64(SIGNMASK_VEC2, v);
			return absval;
		}
		
		ALWAYS_INLINE float32 abs(float32 f) {
			uint32* p = reinterpret_cast<uint32*>(&f);
			uint32 masked = (~SIGNMASK_FLOAT32) & *p;
			return *reinterpret_cast<float32*>(&masked);
		}
	}
}

#endif
