//
//  simd_sse.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/05/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_simd_sse_hpp
#define falling_simd_sse_hpp

#include <xmmintrin.h>
#include <pmmintrin.h>

#if defined(__SSSE3__)
#include <tmmintrin.h>
#endif
#if defined(__SSE4_1__)
#include <smmintrin.h>
#endif
#if defined(__AVX__)
#include <immintrin.h>
#endif

#include <cmath>

namespace falling {
	namespace simd {
		template <typename T, typename M>
		struct ALIGNED(16) M128 {
			using V = M;
			using ElementType = T;
			static const size_t NumComponents = 16 / sizeof(T);
			M v;
			operator M&() { return v; }
			operator const M&() const { return v; }
		};
		// TODO: 256-bit vectors (AVX)
	
		using ivec4  = M128<int32, __m128i>;
		using ilvec2 = M128<int64, __m128i>;
		using uvec4  = M128<uint32, __m128i>;
		using ulvec2 = M128<uint64, __m128i>;
		using fvec4  = M128<float32, __m128>;
		using flvec2 = M128<float64, __m128d>;
		
		SIMD_DEFINE_VECTOR_TYPE(4, byte,   uint32,  byte);
		SIMD_DEFINE_VECTOR_TYPE(1, int32,  int32,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(2, int32,  ivec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(3, int32,  ivec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(4, int32,  ivec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(1, int64,  int64,   uint64)
		SIMD_DEFINE_VECTOR_TYPE(2, int64,  ilvec2,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(1, uint32, uint32,  uint32)
		SIMD_DEFINE_VECTOR_TYPE(2, uint32, uvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(3, uint32, uvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(4, uint32, uvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(1, uint64, uint64,  uint64)
		SIMD_DEFINE_VECTOR_TYPE(2, uint64, ulvec2,  uint64)
		
		SIMD_DEFINE_VECTOR_TYPE(1, float32, float32, uint32)
		SIMD_DEFINE_VECTOR_TYPE(2, float32, fvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(3, float32, fvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(4, float32, fvec4,   uint32)
		SIMD_DEFINE_VECTOR_TYPE(1, float64, float64, uint64)
		SIMD_DEFINE_VECTOR_TYPE(2, float64, flvec2,  uint64)
		
#pragma mark Unaligned Load
		template <typename M> struct UnalignedLoad;
		template <> struct UnalignedLoad<M128<float32,__m128>> {
			void operator()(__m128& dst, const float32* p) {
				dst = _mm_loadu_ps(p);
			}
		};
		
		template <typename T>
		struct UnalignedLoad<M128<T,__m128i>> {
			void operator()(__m128i& dst, const T* p) {
				dst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
			}
		};
		template <> struct UnalignedLoad<M128<float64,__m128d>> {
			void operator()(__m128d& dst, const float64* p) {
				dst = _mm_loadu_pd(p);
			}
		};
		template <typename M>
		void unaligned_load(M& dst, const typename M::ElementType* p) {
			UnalignedLoad<M>()(dst, p);
		}
		
#pragma mark Aligned Load
		template <typename M> struct AlignedLoad;
		template <> struct AlignedLoad<M128<float32, __m128>> {
			void operator()(__m128& dst, const float32* p) {
				dst = _mm_load_ps(p);
			}
		};
		template <typename T> struct AlignedLoad<M128<T, __m128i>> {
			void operator()(__m128i& dst, const T* p) {
				dst = _mm_load_si128(reinterpret_cast<const __m128i*>(p));
			}
		};
		template <> struct AlignedLoad<M128<float64, __m128d>> {
			void operator()(__m128d& dst, const float64* p) {
				dst = _mm_load_pd(p);
			}
		};
		template <typename M>
		void aligned_load(M& dst, const typename M::ElementType* p) {
			AlignedLoad<M>()(dst.v, p);
		}

#pragma mark Set
		ALWAYS_INLINE fvec4 set(float32 x, float32 y, float32 z, float32 w) {
			return {_mm_set_ps(w, z, y, x)};
		}
		ALWAYS_INLINE ivec4 set(int32 x, int32 y, int32 z, int32 w) {
			return {_mm_set_epi32(w, z, y, x)};
		}
		ALWAYS_INLINE uvec4 set(uint32 x, uint32 y, uint32 z, uint32 w) {
			return {_mm_set_epi32(w, z, y, x)};
		}
		ALWAYS_INLINE flvec2 set(float64 x, float64 y) {
			return {_mm_set_pd(y, x)};
		}
		ALWAYS_INLINE ilvec2 set(int64 x, int64 y) {
			return {_mm_set_epi64x(y, x)};
		}
		ALWAYS_INLINE ulvec2 set(uint64 x, uint64 y) {
			return {_mm_set_epi64x(y, x)};
		}
		ALWAYS_INLINE fvec4 set(float32 x, float32 y) {
			return {_mm_set_ps(0.f, 1.f, y, x)};
		}
		ALWAYS_INLINE ivec4 set(int32 x, int32 y) {
			return {_mm_set_epi32(0, 1, y, x)};
		}
		ALWAYS_INLINE uvec4 set(uint32 x, uint32 y) {
			return {_mm_set_epi32(0, 1, y, x)};
		}

#pragma mark Get element
		template <Axis A>
		float32 get(fvec4 x) {
#if defined(__SSE4_1)
			const int a = (int)A;
			return _mm_extract_ps(x, 3-a);
#else
			return reinterpret_cast<float32*>(&x.v)[A];
#endif
		}
		template <Axis A>
		int32 get(ivec4 x) {
#if defined(__SSE4_1__)
			const int a = (int)A;
			return _mm_extract_epi32(x.v, 3-a);
#else
			return reinterpret_cast<int32*>(&x.v)[A];
#endif
		}
		template <Axis A>
		uint32 get(uvec4 x) {
#if defined(__SSE4_1__)
			const int a = (int)A;
			return _mm_extract_epi32(x.v, 3-a);
#else
			return reinterpret_cast<uint32*>(&x.v)[A];
#endif
		}
		template <Axis A>
		float64 get(flvec2 x) {
			static_assert(A < 2, "Out of range.");
			float64* v = reinterpret_cast<float64*>(&x.v);
			return v[A];
		}
		template <Axis A>
		int64 get(ilvec2 x) {
			static_assert(A < 2, "Out of range.");
			int64* v = reinterpret_cast<int64*>(&x.v);
			return v[A];
		}
		template <Axis A>
		uint64 get(ulvec2 x) {
			static_assert(A < 2, "Out of range.");
			uint64* v = reinterpret_cast<uint64*>(&x.v);
			return v[A];
		}
		
#pragma mark Replicate
		template <typename M> struct Replicate;
		template <> struct Replicate<__m128> {
			void operator()(__m128& dst, float32 f) {
				dst = _mm_load1_ps(&f);
			}
		};
		template <> struct Replicate<__m128d> {
			void operator()(__m128d& dst, float64 f) {
				dst = _mm_load1_pd(&f);
			}
		};
		template <> struct Replicate<__m128i> {
			void operator()(__m128i& dst, int32 i) {
				dst = _mm_load1_ps(reinterpret_cast<float32*>(&i));
			}
			void operator()(__m128i& dst, uint32 i) {
				dst = _mm_load1_ps(reinterpret_cast<float32*>(&i));
			}
			void operator()(__m128i& dst, int64 i) {
				dst = _mm_loadh_pd(dst, reinterpret_cast<float64*>(&i));
				dst = _mm_loadl_pd(dst, reinterpret_cast<float64*>(&i));
			}
			void operator()(__m128i& dst, uint64 i) {
				dst = _mm_loadh_pd(dst, reinterpret_cast<float64*>(&i));
				dst = _mm_loadl_pd(dst, reinterpret_cast<float64*>(&i));
			}
		};
		template <typename M>
		void replicate(M& dst, typename M::ElementType x) {
			Replicate<typename M::V>()(dst.v, x);
		}
	
#pragma mark Conversion
		struct ConvertElementType {
			// 4->4 conversion
			void operator()(ivec4& dst, fvec4 src) {
				dst.v = _mm_cvttps_epi32(src.v);
			}
			void operator()(uvec4& dst, fvec4 src) {
				dst.v = _mm_cvttps_epi32(src.v);
			}
			void operator()(fvec4& dst, ivec4 src) {
				dst.v = _mm_cvtepi32_ps(src.v);
			}
			void operator()(fvec4& dst, uvec4 src) {
				dst.v = _mm_cvtepi32_ps(src.v);
			}
			
			// 2->2 conversion
			void operator()(ilvec2& dst, flvec2 src) {
				float64 x = get<X>(src);
				float64 y = get<Y>(src);
				dst = set((int64)x, (int64)y);
			}
			void operator()(ulvec2& dst, flvec2 src) {
				float64 x = get<X>(src);
				float64 y = get<Y>(src);
				dst = set((uint64)x, (uint64)y);
			}
			void operator()(flvec2& dst, ilvec2 src) {
				int64 x = get<X>(src);
				int64 y = get<Y>(src);
				dst = set((float64)x, (float64)y);
			}
			void operator()(flvec2& dst, ulvec2 src) {
				uint64 x = get<X>(src);
				uint64 y = get<Y>(src);
				dst = set((float64)x, (float64)y);
			}
		};

		struct ConvertSize {
			// 2->4 conversion
			void operator()(fvec4& dst, flvec2 src) {
				dst.v = _mm_cvtpd_ps(src.v);
			}
			void operator()(ivec4& dst, ilvec2 src) {
				// Warning: L-H-S ???
				int64 x = get<X>(src);
				int64 y = get<Y>(src);
				dst.v = _mm_set_epi32(0, 0, (int32)y, (int32)x);
			}
			void operator()(uvec4& dst, ulvec2 src) {
				// Warning: L-H-S ???
				uint64 x = get<X>(src);
				uint64 y = get<Y>(src);
				dst.v = _mm_set_epi32(0, 0, (uint32)y, (uint32)x);
			}
			
			// 4->2 conversion
			void operator()(flvec2& dst, fvec4 src) {
				dst.v = _mm_cvtps_pd(src.v);
			}
			void operator()(ilvec2& dst, ivec4 src) {
#if defined(__SSE4_1__)
				dst.v = _mm_cvtepi32_epi64(src.v);
#else
				dst = set((int64)get<X>(src), (int64)get<Y>(src));
#endif
			}
			void operator()(ulvec2& dst, uvec4 src) {
#if defined(__SSE4_1__)
				dst.v = _mm_cvtepu32_epi64(src.v);
#else
				dst = set((uint64)get<X>(src), (uint64)get<Y>(src));
#endif
			}
		};
		
		template <typename FromT, size_t FromN, typename ToT, size_t ToN>
		struct ConvertImpl {
			using FromM = typename GetVectorType<FromT, FromN>::Type;
			using ToM = typename GetVectorType<ToT, ToN>::Type;
			static const bool IsSameElementType = std::is_same<FromT, ToT>::value;
			static const bool IsSameSize = FromN == ToN;
			
			template <typename ToT_ = ToT, size_t ToN_ = ToN>
			typename std::enable_if<std::is_same<ToT_, FromT>::value && ToN_ == FromN, void>::type
			operator()(ToM& to, FromM from) {
				to = from;
			}
			
			template <typename ToT_ = ToT, size_t ToN_ = ToN>
			typename std::enable_if<!std::is_same<ToT_, FromT>::value && ToN_ == FromN, void>::type
			operator()(ToM& to, FromM from) {
				ConvertElementType()(to, from);
			}
			
			template <typename ToT_ = ToT, size_t ToN_ = ToN>
			typename std::enable_if<std::is_same<ToT_, FromT>::value && ToN_ != FromN, void>::type
			operator()(ToM& to, FromM from) {
				ConvertSize()(to, from);
			}
			
			template <typename ToT_ = ToT, size_t ToN_ = ToN>
			typename std::enable_if<!std::is_same<ToT_, FromT>::value && ToN_ != FromN, void>::type
			operator()(ToM& to, FromM from) {
				using IntermediateM = typename GetVectorType<FromT, ToN>::Type;
				IntermediateM tmp;
				ConvertSize()(tmp, from);
				ConvertElementType()(to, tmp);
			}
		};
		
		template <typename FromM, typename ToM> struct Convert {
			using FromT = typename FromM::ElementType;
			static const auto FromN = FromM::NumComponents;
			using ToT = typename ToM::ElementType;
			static const auto ToN = ToM::NumComponents;
			void operator()(ToM& to, FromM m) {
				ConvertImpl<FromT, FromN, ToT, ToN>()(to, m);
			}
		};
		template <typename From, typename To>
		void convert(From from, To& to) {
			Convert<From, To>()(to, from);
		}

#pragma mark Arithmetic (Add/Sub/Mul/Div)

		ALWAYS_INLINE fvec4 add(fvec4 a, fvec4 b) {
			return {_mm_add_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 add(ivec4 a, ivec4 b) {
			return {_mm_add_epi32(a, b)};
		}
		ALWAYS_INLINE uvec4 add(uvec4 a, uvec4 b) {
			return {_mm_add_epi32(a, b)};
		}

		ALWAYS_INLINE fvec4 sub(fvec4 a, fvec4 b) {
			return {_mm_sub_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 sub(ivec4 a, ivec4 b) {
			return {_mm_sub_epi32(a, b)};
		}
		ALWAYS_INLINE uvec4 sub(uvec4 a, uvec4 b) {
			return {_mm_sub_epi32(a, b)};
		}

		ALWAYS_INLINE fvec4 mul(fvec4 a, fvec4 b) {
			return {_mm_mul_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 mul(ivec4 a, ivec4 b) {
#if defined(__SSE4_1__)
			return {_mm_mul_epi32(a, b)};
#else
			return set(get<X>(a)*get<X>(b), get<Y>(a)*get<Y>(b), get<Z>(a)*get<Z>(b), get<W>(a)*get<W>(b));
#endif
		}
		ALWAYS_INLINE uvec4 mul(uvec4 a, uvec4 b) {
#if defined(__SSE4_1__)
			return {_mm_mul_epi32(a, b)};
#else
			return set(get<X>(a)*get<X>(b), get<Y>(a)*get<Y>(b), get<Z>(a)*get<Z>(b), get<W>(a)*get<W>(b));
#endif
		}

		ALWAYS_INLINE fvec4 div(fvec4 a, fvec4 b) {
			return {_mm_div_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 div(ivec4 a, ivec4 b) {
#if defined(_mm_idiv_epi32)
			return {_mm_idiv_epi32(a, b)};
#else
			// Warning: Super slow!
			return set(get<X>(a)/get<X>(b), get<Y>(a)/get<Y>(b), get<Z>(a)/get<Z>(b), get<W>(a)/get<W>(b));
#endif
		}
		ALWAYS_INLINE uvec4 div(uvec4 a, uvec4 b) {
#if defined(_mm_idiv_epi32)
			return {_mm_idiv_epi32(a, b)};
#else
			// Warning: Super slow!
			return set(get<X>(a)/get<X>(b), get<Y>(a)/get<Y>(b), get<Z>(a)/get<Z>(b), get<W>(a)/get<W>(b));
#endif
		}
		
		ALWAYS_INLINE ivec4 divrem(ivec4 a, ivec4 b, ivec4* rem) {
#if defined(_mm_idivrem_epi32)
			return {_mm_idivrem_epi32(&rem, a, b)};
#else
			// Warning: Super slow!
			*rem = set(get<X>(a)%get<X>(b), get<Y>(a)%get<Y>(b), get<Z>(a)%get<Z>(b), get<W>(a)%get<W>(b));
			return set(get<X>(a)/get<X>(b), get<Y>(a)/get<Y>(b), get<Z>(a)/get<Z>(b), get<W>(a)/get<W>(b));
#endif
		}
		
		ALWAYS_INLINE uvec4 divrem(uvec4 a, uvec4 b, uvec4* rem) {
#if defined(_mm_idivrem_epi32)
			return {_mm_idivrem_epi32(&rem, a, b)};
#else
			// Warning: Super slow!
			*rem = set(get<X>(a)%get<X>(b), get<Y>(a)%get<Y>(b), get<Z>(a)%get<Z>(b), get<W>(a)%get<W>(b));
			return set(get<X>(a)/get<X>(b), get<Y>(a)/get<Y>(b), get<Z>(a)/get<Z>(b), get<W>(a)/get<W>(b));
#endif
		}

#pragma mark Bitwise (And/Or/Xor/Not)

		ALWAYS_INLINE fvec4 bitwise_and(fvec4 a, fvec4 b) {
			return {_mm_and_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 bitwise_and(ivec4 a, ivec4 b) {
			return {_mm_and_si128(a, b)};
		}
		ALWAYS_INLINE uvec4 bitwise_and(uvec4 a, uvec4 b) {
			return {_mm_and_si128(a, b)};
		}

		ALWAYS_INLINE fvec4 bitwise_or(fvec4 a, fvec4 b) {
			return {_mm_or_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 bitwise_or(ivec4 a, ivec4 b) {
			return {_mm_or_si128(a, b)};
		}
		ALWAYS_INLINE uvec4 bitwise_or(uvec4 a, uvec4 b) {
			return {_mm_or_si128(a, b)};
		}

		ALWAYS_INLINE fvec4 bitwise_xor(fvec4 a, fvec4 b) {
			return {_mm_xor_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 bitwise_xor(ivec4 a, ivec4 b) {
			return {_mm_xor_si128(a, b)};
		}
		ALWAYS_INLINE uvec4 bitwise_xor(uvec4 a, uvec4 b) {
			return {_mm_xor_si128(a, b)};
		}

		ALWAYS_INLINE fvec4 bitwise_not(fvec4 a) {
			__m128 junk = _mm_set1_epi32(0);
			__m128 t = _mm_cmpeq_ps(junk, junk);
			return {_mm_xor_ps(a.v, t)};
		}
		ALWAYS_INLINE ivec4 bitwise_not(ivec4 a) {
			__m128 junk = _mm_set1_epi32(0);
			__m128i t = _mm_cmpeq_epi32(junk, junk);
			return {_mm_xor_si128(a.v, t)};
		}
		ALWAYS_INLINE uvec4 bitwise_not(uvec4 a) {
			__m128 junk = _mm_set1_epi32(0);
			__m128i t = _mm_cmpeq_epi32(junk, junk);
			return {_mm_xor_si128(a.v, t)};
		}

#pragma mark Comparison
		ALWAYS_INLINE uvec4 cmp_lt(fvec4 a, fvec4 b) {
			return {_mm_cmplt_ps(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_lte(fvec4 a, fvec4 b) {
			return {_mm_cmple_ps(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_gt(fvec4 a, fvec4 b) {
			return {_mm_cmpgt_ps(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_gte(fvec4 a, fvec4 b) {
			return {_mm_cmpge_ps(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_eq(ivec4 a, ivec4 b) {
			return {_mm_cmpeq_epi32(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_neq(ivec4 a, ivec4 b) {
			__m128i junk = _mm_set1_ps(0.f);
			__m128i ones = _mm_cmpeq_epi32(junk, junk);
			__m128i eq = _mm_cmpeq_epi32(a, b);
			return {_mm_xor_si128(ones, eq)};
		}
		ALWAYS_INLINE uvec4 cmp_lt(ivec4 a, ivec4 b) {
			return {_mm_cmplt_epi32(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_lte(ivec4 a, ivec4 b) {
			return {_mm_or_si128(cmp_eq(a, b).v, cmp_lt(a, b).v)};
		}
		ALWAYS_INLINE uvec4 cmp_gt(ivec4 a, ivec4 b) {
			return {_mm_cmpgt_epi32(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_gte(ivec4 a, ivec4 b) {
			return {_mm_or_si128(cmp_eq(a, b).v, cmp_gt(a, b).v)};
		}
		ALWAYS_INLINE uvec4 cmp_eq(uvec4 a, uvec4 b) {
			return {_mm_cmpeq_epi32(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_neq(uvec4 a, uvec4 b) {
			__m128i junk = _mm_set1_ps(0.f);
			__m128i ones = _mm_cmpeq_epi32(junk, junk);
			__m128i eq = _mm_cmpeq_epi32(a, b);
			return {_mm_xor_si128(ones, eq)};
		}
		ALWAYS_INLINE uvec4 cmp_lt(uvec4 a, uvec4 b) {
			return {_mm_cmplt_epi32(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_lte(uvec4 a, uvec4 b) {
			return {_mm_or_si128(cmp_eq(a, b).v, cmp_lt(a, b).v)};
		}
		ALWAYS_INLINE uvec4 cmp_gt(uvec4 a, uvec4 b) {
			return {_mm_cmpgt_epi32(a.v, b.v)};
		}
		ALWAYS_INLINE uvec4 cmp_gte(uvec4 a, uvec4 b) {
			return {_mm_or_si128(cmp_eq(a, b).v, cmp_gt(a, b).v)};
		}


#pragma mark Shuffle
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		fvec4 shuffle2aabb(fvec4 a, fvec4 b) {
			return {_mm_shuffle_ps(a, b, _MM_SHUFFLE(W_, Z_, Y_, X_))};
		}
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		ivec4 shuffle2aabb(ivec4 a, ivec4 b) {
			return {_mm_shuffle_ps(a, b, _MM_SHUFFLE(W_, Z_, Y_, X_))};
		}
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		uvec4 shuffle2aabb(uvec4 a, uvec4 b) {
			return {_mm_shuffle_ps(a, b, _MM_SHUFFLE(W_, Z_, Y_, X_))};
		}
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_ = W>
		fvec4 shuffle(fvec4 v) {
			return shuffle2aabb<X_, Y_, Z_, W_>(v, v);
		}
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_ = W>
		ivec4 shuffle(ivec4 v) {
			return {_mm_shuffle_epi32(v, _MM_SHUFFLE(W_, Z_, Y_, X_))};
		}
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_ = W>
		uvec4 shuffle(uvec4 v) {
			return {_mm_shuffle_epi32(v, _MM_SHUFFLE(W_, Z_, Y_, X_))};
		}
		
		template <Axis X_, Axis Y_>
		fvec4 shuffle(fvec4 v) {
			return shuffle<X_, Y_, Z, W>(v);
		}
		template <Axis X_, Axis Y_>
		ivec4 shuffle(ivec4 v) {
			return shuffle<X_, Y_, Z, W>(v);
		}
		template <Axis X_, Axis Y_>
		uvec4 shuffle(uvec4 v) {
			return shuffle<X_, Y_, Z, W>(v);
		}
		
		template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, Axis Z_, size_t WVector = ZVector, Axis W_ = W>
		struct Shuffle2;
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<0, X_, 0, Y_, 0, Z_, 0, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				return falling::simd::shuffle<X_, Y_, Z_, W_>(a);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<0, X_, 0, Y_, 0, Z_, 1, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<IgnoreAxis, Z_, IgnoreAxis, W_>(a, b);
				auto tmp1 = shuffle2aabb<X_, Y_, Y, W>(a, tmp0);
				return tmp1;
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<0, X_, 0, Y_, 1, Z_, 0, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<IgnoreAxis, Z_, IgnoreAxis, W_>(b, a);
				auto tmp1 = shuffle2aabb<X_, Y_, Y, W>(a, tmp0);
				return tmp1;
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<0, X_, 0, Y_, 1, Z_, 1, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				return {_mm_shuffle_ps(a, b, _MM_SHUFFLE(W_, Z_, Y_, X_))};
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<0, X_, 1, Y_, 0, Z_, 0, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<IgnoreAxis, X_, IgnoreAxis, Y_>(a, b);
				auto tmp1 = shuffle2aabb<Y, W, Z_, W_>(tmp0, a);
				return tmp1;
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<0, X_, 1, Y_, 0, Z_, 1, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<X_, Z_, Y_, W_>(a, b);
				return shuffle2aabb<X, Z, Y, W>(tmp0, tmp0);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<0, X_, 1, Y_, 1, Z_, 0, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<X_, W_, Y_, Z_>(a, b);
				return shuffle2aabb<X, Z, W, Y>(tmp0, tmp0);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<0, X_, 1, Y_, 1, Z_, 1, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<X_, IgnoreAxis, Y_, IgnoreAxis>(a, b);
				auto tmp1 = shuffle2aabb<X, Z, Z_, W_>(tmp0, b);
				return tmp1;
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<1, X_, 0, Y_, 0, Z_, 0, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<X_, IgnoreAxis, Y_, IgnoreAxis>(b, a);
				auto tmp1 = shuffle2aabb<X, Z, Z_, W_>(tmp0, a);
				return tmp1;
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<1, X_, 0, Y_, 0, Z_, 1, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<X_, W_, Y_, Z_>(b, a);
				return shuffle2aabb<X, Z, W, Y>(tmp0, tmp0);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<1, X_, 0, Y_, 1, Z_, 0, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<Y_, W_, X_, Z_>(a, b);
				return shuffle2aabb<Z, X, W, Y>(tmp0, tmp0);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<1, X_, 0, Y_, 1, Z_, 1, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<X_, IgnoreAxis, Y_, IgnoreAxis>(b, a);
				return shuffle2aabb<X, Z, Z_, W_>(tmp0, b);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<1, X_, 1, Y_, 0, Z_, 0, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				return shuffle2aabb<X_, Y_, Z_, W_>(b, a);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<1, X_, 1, Y_, 0, Z_, 1, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<Z_, IgnoreAxis, W_, IgnoreAxis>(a, b);
				return shuffle2aabb<X_, Y_, X, Z>(b, tmp0);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<1, X_, 1, Y_, 1, Z_, 0, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				auto tmp0 = shuffle2aabb<Z_, IgnoreAxis, W_, IgnoreAxis>(b, a);
				return shuffle2aabb<X_, Y_, X, Z>(b, tmp0);
			}
		};
		
		template <Axis X_, Axis Y_, Axis Z_, Axis W_>
		struct Shuffle2<1, X_, 1, Y_, 1, Z_, 1, W_> {
			template <typename V>
			static V shuffle(V a, V b) {
				return shuffle2aabb<X_, Y_, Z_, W_>(b, b);
			}
		};

		template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, Axis Z_, size_t WVector = 1, Axis W_ = W>
		fvec4 shuffle2(fvec4 a, fvec4 b) {
			return Shuffle2<XVector, X_, YVector, Y_, ZVector, Z_, WVector, W_>::shuffle(a, b);
		}
		
		template <size_t XVector, Axis X_, size_t YVector, Axis Y_>
		fvec4 shuffle2(fvec4 a, fvec4 b) {
			return shuffle2<XVector, X_, YVector, Y_, XVector, Z, XVector, W>(a, b);
		}
		
		template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, Axis Z_, size_t WVector = 1, Axis W_ = W>
		ivec4 shuffle2(ivec4 a, ivec4 b) {
			return Shuffle2<XVector, X_, YVector, Y_, ZVector, Z_, WVector, W_>::shuffle(a, b);
		}
		template <size_t XVector, Axis X_, size_t YVector, Axis Y_>
		ivec4 shuffle2(ivec4 a, ivec4 b) {
			return shuffle2<XVector, X_, YVector, Y_, XVector, Z, XVector, W>(a, b);
		}
		
		template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, Axis Z_, size_t WVector = 1, Axis W_ = W>
		uvec4 shuffle2(uvec4 a, uvec4 b) {
			return Shuffle2<XVector, X_, YVector, Y_, ZVector, Z_, WVector, W_>::shuffle(a, b);
		}
		template <size_t XVector, Axis X_, size_t YVector, Axis Y_>
		uvec4 shuffle2(uvec4 a, uvec4 b) {
			return shuffle2<XVector, X_, YVector, Y_, XVector, Z, XVector, W>(a, b);
		}
		
#pragma mark Select

		ALWAYS_INLINE fvec4 select(uvec4 mask, fvec4 if_true, fvec4 if_false) {
			uvec4 nmask = bitwise_not(mask);
			return {_mm_or_si128(_mm_and_si128(mask.v, if_true.v), _mm_and_si128(nmask.v, if_false.v))};
		}
		ALWAYS_INLINE ivec4 select(uvec4 mask, ivec4 if_true, ivec4 if_false) {
			uvec4 nmask = bitwise_not(mask);
			return {_mm_or_si128(_mm_and_si128(mask.v, if_true.v), _mm_and_si128(nmask.v, if_false.v))};	
		}
		ALWAYS_INLINE uvec4 select(uvec4 mask, uvec4 if_true, uvec4 if_false) {
			uvec4 nmask = bitwise_not(mask);
			return {_mm_or_si128(_mm_and_si128(mask.v, if_true.v), _mm_and_si128(nmask.v, if_false.v))};
		}
		

#pragma mark Horizontal Addition
		ALWAYS_INLINE fvec4 hadd3(fvec4 v) {
#if defined(__SSE3__)
			__m128 zero = _mm_set1_ps(0.f);
			__m128 masked = shuffle2<0, X, 0, Y, 0, Z, 1, W>(v, {zero});
			__m128 tmp = _mm_hadd_ps(masked, masked);
			__m128 r = _mm_hadd_ps(tmp, tmp);
			return shuffle<X,X,X,X>(fvec4{r});
#else
			fvec4 a = shuffle<X,X,X,X>(v);
			fvec4 b = shuffle<Y,Y,Y,Y>(v);
			fvec4 c = shuffle<Z,Z,Z,Z>(v);
			return a + b + c;
#endif
		}
		
		ALWAYS_INLINE fvec4 hadd4(fvec4 v) {
#if defined(__SSE3__)
			__m128 tmp = _mm_hadd_ps(v, v);
			return {_mm_hadd_ps(tmp, tmp)};
#else
			// TODO: This could be done with one less shuffle, I suppose?
			fvec4 a = shuffle<X,X,X,X>(v);
			fvec4 b = shuffle<Y,Y,Y,Y>(v);
			fvec4 c = shuffle<Z,Z,Z,Z>(v);
			fvec4 d = shuffle<W,W,W,W>(v);
			return a + b + c + d;
#endif
		}
		
		ALWAYS_INLINE fvec4 hadd2(fvec4 v) {
			fvec4 tmp = {_mm_hadd_ps(v, v)};
			return shuffle<X, X>(tmp);
		}
		
		ALWAYS_INLINE ivec4 hadd2(ivec4 v) {
#if defined(__SSSE3__)
			ivec4 tmp = {_mm_hadd_epi32(v, v)};
			return shuffle<X, X>(tmp);
#else
			int32 sum = get<X>(v) + get<Y>(v);
			return set(sum, sum);
#endif
		}
		
		ALWAYS_INLINE uvec4 hadd2(uvec4 v) {
#if defined(__SSSE3__)
			uvec4 tmp = {_mm_hadd_epi32(v,v)};
			return shuffle<X,X>(tmp);
#else
			uint32 sum = get<X>(v) + get<Y>(v);
			return set(sum, sum);
#endif
		}
		
		ALWAYS_INLINE ivec4 hadd3(ivec4 v) {
#if defined(__SSSE3__)
			__m128i zero = _mm_set1_epi32(0);
			__m128i masked = shuffle2<0, X, 0, Y, 0, Z, 1, W>(v, {zero});
			__m128i tmp = _mm_hadd_epi32(masked, masked);
			__m128i r = _mm_hadd_epi32(tmp, tmp);
			return shuffle<X,X,X,X>(ivec4{r});
#else
			auto sum = get<X>(v) + get<Y>(v) + get<Z>(v);
			return set(sum, sum, sum, sum);
#endif
		}
		
		ALWAYS_INLINE uvec4 hadd3(uvec4 v) {
#if defined(__SSSE3__)
			__m128i zero = _mm_set1_epi32(0);
			__m128i masked = shuffle2<0, X, 0, Y, 0, Z, 1, W>(v, {zero});
			__m128i tmp = _mm_hadd_epi32(masked, masked);
			__m128i r = _mm_hadd_epi32(tmp, tmp);
			return shuffle<X,X,X,X>(uvec4{r});
#else
			auto sum = get<X>(v) + get<Y>(v) + get<Z>(v);
			return set(sum, sum, sum, sum);
#endif
		}
		
		ALWAYS_INLINE ivec4 hadd4(ivec4 v) {
#if defined(__SSSE3__)
			auto tmp = _mm_hadd_epi32(v, v);
			return {_mm_hadd_epi32(tmp, tmp)};
#else
			auto sum = get<X>(v) + get<Y>(v) + get<Z>(v) + get<W>(v);
			return set(sum, sum, sum, sum);
#endif
		}
		
		ALWAYS_INLINE uvec4 hadd4(uvec4 v) {
#if defined(__SSSE3__)
			auto tmp = _mm_hadd_epi32(v, v);
			return {_mm_hadd_epi32(tmp, tmp)};
#else
			auto sum = get<X>(v) + get<Y>(v) + get<Z>(v) + get<W>(v);
			return set(sum, sum, sum, sum);
#endif
		}
		
#pragma mark Sqrt/Rsqrt
		ALWAYS_INLINE fvec4 sqrt(fvec4 vec) {
			return {_mm_sqrt_ps(vec)};
		}
		
		ALWAYS_INLINE fvec4 rsqrt(fvec4 vec) {
			return {_mm_rsqrt_ps(vec)};
		}
		
		ALWAYS_INLINE float32 sqrt(float32 f) {
			fvec4 v = {_mm_set_ss(f)};
			fvec4 r = {_mm_sqrt_ss(v)};
			return get<X>(r);
		}
		
		ALWAYS_INLINE float32 rsqrt(float32 f) {
			fvec4 v = {_mm_set_ss(f)};
			fvec4 r = {_mm_rsqrt_ss(v)};
			return get<X>(r);
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
		
#pragma mark Abs/Negate
		static const uint32 SIGNMASK_FLOAT32 = 0x80000000;
		static const uint32 SIGNMASK_INT32 = 0x80000000;
		static const uvec4 SIGNMASK_VEC4 = {_mm_set1_epi32(SIGNMASK_FLOAT32)};
		static const uvec4 SIGNMASK_IVEC4 = {_mm_set1_epi32(SIGNMASK_INT32)};
		
		ALWAYS_INLINE fvec4 neg(fvec4 v) {
			fvec4 negval = {_mm_xor_ps(v, SIGNMASK_VEC4)};
			return negval;
		}

		ALWAYS_INLINE ivec4 neg(ivec4 v) {
			__m128i zero = _mm_set1_epi32(0);
			return {_mm_sub_epi32(zero, v)};
		}
		
		ALWAYS_INLINE float32 neg(float32 f) {
			return -f;
		}
		
		ALWAYS_INLINE fvec4 abs(fvec4 v) {
			fvec4 absval = {_mm_andnot_ps(SIGNMASK_VEC4, v)};
			return absval;
		}

		ALWAYS_INLINE ivec4 abs(ivec4 v) {
			__m128i zero = _mm_set1_epi32(0);
			auto below_zero = cmp_lt(v, {zero});
			auto nv = neg(v);
			ivec4 absval = select(below_zero, nv, v);
			return absval;
		}

		ALWAYS_INLINE uvec4 abs(uvec4 v) { return v; }

#pragma mark Rounding
		template <typename T, size_t N>
		typename std::enable_if<std::is_integral<T>::value, typename GetVectorType<T, N>::Type>::type
		round(typename GetVectorType<T, N>::Type v) {
			return v;
		}
		template <typename T, size_t N>
		typename std::enable_if<std::is_integral<T>::value, typename GetVectorType<T, N>::Type>::type
		ceil(typename GetVectorType<T, N>::Type v) {
			return v;
		}
		template <typename T, size_t N>
		typename std::enable_if<std::is_integral<T>::value, typename GetVectorType<T, N>::Type>::type
		floor(typename GetVectorType<T, N>::Type v) {
			return v;
		}

		ALWAYS_INLINE ivec4 round(ivec4 v) { return v; }
		ALWAYS_INLINE uvec4 round(uvec4 v) { return v; }
		ALWAYS_INLINE ivec4 floor(ivec4 v) { return v; }
		ALWAYS_INLINE uvec4 floor(uvec4 v) { return v; }
		ALWAYS_INLINE ivec4 ceil(ivec4 v) { return v; }
		ALWAYS_INLINE uvec4 ceil(uvec4 v) { return v; }
		
		ALWAYS_INLINE fvec4 round(fvec4 v) {
#if defined(__SSE4_1__)
			return {_mm_round_ps(v, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEAREST_INT)};
#else
			_MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
			__m128i as_integer = _mm_cvtps_epi32(v);
			return {_mm_cvtepi32_ps(as_integer)};
#endif
		}
		
		ALWAYS_INLINE fvec4 floor(fvec4 v) {
#if defined(__SSE4_1__)
			return {_mm_round_ps(v, _MM_FROUND_NO_EXC | _MM_FROUND_TO_NEG_INF)};
#else
			_MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);
			__m128i as_integer = _mm_cvtps_epi32(v);
			return {_mm_cvtepi32_ps(as_integer)};
#endif
		}
		
		ALWAYS_INLINE fvec4 ceil(fvec4 v) {
#if defined(__SSE4_1__)
			return {_mm_round_ps(v, _MM_FROUND_NO_EXC | _MM_FROUND_TO_POS_INF)};
#else
			_MM_SET_ROUNDING_MODE(_MM_ROUND_UP);
			__m128i as_integer = _mm_cvtps_epi32(v);
			return {_mm_cvtepi32_ps(as_integer)};
#endif
		}
		
#pragma mark Min/Max
		ALWAYS_INLINE fvec4 min(fvec4 a, fvec4 b) {
			return {_mm_min_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 min(ivec4 a, ivec4 b) {
#if defined(__SSE4_1__)
			return {_mm_min_epi32(a, b)};
#else
			return select(cmp_lt(a, b), a, b);
#endif
		}
		ALWAYS_INLINE uvec4 min(uvec4 a, uvec4 b) {
#if defined(__SSE4_1__)
			// TODO: Unsigned comparison?
			return {_mm_min_epi32(a, b)};
#else
			return select(cmp_lt(a, b), a, b);
#endif
		}
		
		ALWAYS_INLINE fvec4 max(fvec4 a, fvec4 b) {
			return {_mm_max_ps(a, b)};
		}
		ALWAYS_INLINE ivec4 max(ivec4 a, ivec4 b) {
#if defined(__SSE4_1__)
			return {_mm_max_epi32(a, b)};
#else
			return select(cmp_gt(a, b), a, b);
#endif
		}
		ALWAYS_INLINE uvec4 max(uvec4 a, uvec4 b) {
#if defined(__SSE4_1__)
			// TODO: Unsigned comparison?
			return {_mm_max_epi32(a, b)};
#else
			return select(cmp_gt(a, b), a, b);
#endif
		}
		
#pragma mark Truthiness
		ALWAYS_INLINE size_t num_true(uvec4 v, const int m = 0xffff) {
			int mask = _mm_movemask_epi8(v) & m;
			size_t n = 0;
			if (mask & 0xf000) ++n;
			if (mask & 0x0f00) ++n;
			if (mask & 0x00f0) ++n;
			if (mask & 0x000f) ++n;
			return n;
		}
		
		ALWAYS_INLINE bool all_true4(uvec4 v) {
			int mask = _mm_movemask_epi8(v);
			return mask == 0xffff;
		}
		
		ALWAYS_INLINE bool all_true3(uvec4 v) {
			int mask = _mm_movemask_epi8(v);
			return (mask & 0x0fff) == 0x0fff;
		}
		
		ALWAYS_INLINE bool all_true2(uvec4 v) {
			int mask = _mm_movemask_epi8(v);
			return (mask & 0x00ff) == 0x00ff;
		}
		
		ALWAYS_INLINE bool any_true4(uvec4 v) {
			int mask = _mm_movemask_epi8(v);
			return mask != 0;
		}
		
		ALWAYS_INLINE bool any_true3(uvec4 v) {
			int mask = _mm_movemask_epi8(v);
			return (mask & 0x0fff) != 0;
		}
		
		ALWAYS_INLINE bool any_true2(uvec4 v) {
			int mask = _mm_movemask_epi8(v);
			return (mask & 0x00ff) != 0;
		}
		
		ALWAYS_INLINE bool all_false4(uvec4 v) { return !any_true4(v); }
		ALWAYS_INLINE bool all_false3(uvec4 v) { return !any_true3(v); }
		ALWAYS_INLINE bool all_false2(uvec4 v) { return !any_true2(v); }
		ALWAYS_INLINE bool any_false4(uvec4 v) { return !all_true4(v); }
		ALWAYS_INLINE bool any_false3(uvec4 v) { return !all_true3(v); }
		ALWAYS_INLINE bool any_false2(uvec4 v) { return !all_true2(v); }
		
		template <size_t N> bool all_true(uvec4 v) { return N == 4 ? all_true4(v) : (N == 3 ? all_true3(v) : all_true2(v)); }
		template <size_t N> bool all_false(uvec4 v) { return N == 4 ? all_false4(v) : (N == 3 ? all_false3(v) : all_false2(v)); }
		template <size_t N> bool any_true(uvec4 v) { return N == 4 ? any_true4(v) : (N == 3 ? any_true3(v) : any_true2(v)); }
		template <size_t N> bool any_false(uvec4 v) { return N == 4 ? any_false4(v) : (N == 3 ? any_false3(v) : any_false2(v)); }
	}
}

#endif
