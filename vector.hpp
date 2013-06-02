//
//  vector.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/05/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#pragma once
#ifndef falling_vector_hpp
#define falling_vector_hpp

#include "base/basic.hpp"
#include "base/simd.hpp"
#include "base/approximately.hpp"

#include <math.h>
#include <numeric>

namespace falling {
	
	template <typename T, size_t N>
	struct VectorCrosser;
	
	static const char VectorComponentNames[][2] = {"x", "y", "z", "w"};
	
	// The VectorData struct is inherited by TVector, and its purpose
	// is the direct accessors x/y/z/w, which wouldn't otherwise be
	// possible to implement.
	template <typename ElementType, size_t N> struct VectorData;
	
	template <typename T> struct VectorData<T, 1> {
		typedef simd::GetVectorType<T, 1> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskElementType MaskElementType;
		typedef typename simd::GetVectorType<MaskElementType, 1>::Type MaskType;
		
		union {
			Type m;
			MaskType mask;
			T v[1];
			struct {
				T x;
			};
		};
	};
	template <typename T> struct VectorData<T, 2> {
		typedef simd::GetVectorType<T, 2> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskElementType MaskElementType;
		typedef typename simd::GetVectorType<MaskElementType, 2>::Type MaskType;
		
		union {
			Type m;
			MaskType mask;
			T v[2];
			struct {
				T x;
				T y;
			};
			struct {
				T width;
				T height;
			};
		};
	};
	template <typename T> struct VectorData<T, 3> {
		typedef simd::GetVectorType<T, 3> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskElementType MaskElementType;
		typedef typename simd::GetVectorType<MaskElementType, 3>::Type MaskType;
		
		union {
			Type m;
			MaskType mask;
			T v[3];
			struct {
				T x;
				T y;
				T z;
			};
			struct {
				T width;
				T height;
				T depth;
			};
		};
	};
	template <typename T> struct VectorData<T, 4> {
		typedef simd::GetVectorType<T, 4> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskElementType MaskElementType;
		typedef typename simd::GetVectorType<MaskElementType, 4>::Type MaskType;
		
		union {
			Type m;
			MaskType mask;
			T v[4];
			struct {
				T x;
				T y;
				T z;
				T w;
			};
			struct {
				T width;
				T height;
				T depth;
				// maybe time? :)
			};
		};
	};
	
	template <typename ElementType, size_t N>
	struct TVector : VectorData<ElementType, N> {
		typedef simd::GetVectorType<ElementType, N> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskElementType MaskElementType;
		typedef typename simd::GetVectorType<MaskElementType, N>::Type MaskType;
		typedef ElementType ComponentType;
		typedef TVector<ElementType, N> Self;
		typedef TVector<MaskElementType, N> MaskVector;
		typedef MaskVector ComparisonResult;
		
		// Array-like interface
		
		typedef ElementType* iterator;
		typedef const ElementType* const_iterator;
		iterator begin() { return this->v; }
		iterator end()   { return this->v + N; }
		const_iterator begin() const { return this->v; }
		const_iterator end() const   { return this->v + N; }
		size_t size() const { return N; }
		
		ElementType& operator[](size_t idx) {
			ASSERT(idx < N);
			return this->v[idx];
		}
		
		const ElementType& operator[](size_t idx) const {
			ASSERT(idx < N);
			return this->v[idx];
		}
		
		
		// Constructors
		
		ALWAYS_INLINE TVector() {}
		ALWAYS_INLINE explicit TVector(const ElementType* elements) { simd::unaligned_load(this->m, elements); }
		ALWAYS_INLINE explicit TVector(ElementType elements[N])     { simd::unaligned_load(this->m, elements); }
		ALWAYS_INLINE TVector(const Self& other) { this->m = other.m; }
		ALWAYS_INLINE TVector(Type repr) { this->m = repr; }
		
		
		template <typename Enable = void>
		ALWAYS_INLINE explicit TVector(ElementType x, typename std::enable_if<N == 1, Enable*>::type = nullptr) { this->m = Type{x}; }
		template <typename Enable = void>
		ALWAYS_INLINE TVector(ElementType x, ElementType y, typename std::enable_if<N == 2, Enable*>::type = nullptr) { this->m = Type{x, y}; }
		template <typename Enable = void>
		ALWAYS_INLINE TVector(ElementType x, ElementType y, ElementType z, typename std::enable_if<N == 3, Enable*>::type = nullptr) { this->m = Type{x, y, z}; }
		template <typename Enable = void>
		ALWAYS_INLINE TVector(ElementType x, ElementType y, ElementType z, ElementType w, typename std::enable_if<N == 4, Enable*>::type = nullptr) { this->m = Type{x, y, z, w}; }
		
		ALWAYS_INLINE Self& operator=(Self other) { this->m = other.m; return *this; }
		
		
		// Conversion
		
		template <typename U>
		explicit operator TVector<U,N>() const;
		
		
		// Comparison
		
		ALWAYS_INLINE ComparisonResult operator<(const Self& other) const { return simd::cmp_lt(this->m, other.m); }
		ALWAYS_INLINE ComparisonResult operator<=(const Self& other) const { return simd::cmp_lte(this->m, other.m); }
		ALWAYS_INLINE ComparisonResult operator>(const Self& other) const { return simd::cmp_gt(this->m, other.m); }
		ALWAYS_INLINE ComparisonResult operator>=(const Self& other) const { return simd::cmp_gte(this->m, other.m); }
		
		template <typename T = ElementType>
		ALWAYS_INLINE ComparisonResult
		operator==(const Self& other) const {
			static_assert(!IsFloatingPoint<T>::Value, "You should not compare vectors of floats directly with operator==. Use approximately(...) to pad one value with acceptable bounds.");
			return simd::cmp_eq(this->m, other.m);
		}
		
		template <typename T = ElementType>
		ALWAYS_INLINE ComparisonResult
		operator!=(const Self& other) const {
			static_assert(!IsFloatingPoint<T>::Value, "You should not compare vectors of floats directly with operator!=. Use approximately(...) to pad one value with acceptable bounds.");
			return simd::cmp_neq(this->m, other.m);
		}
		
		ALWAYS_INLINE ComparisonResult equal_within(const Self& other, ElementType epsilon) const {
			Self diff = (*this - other).abs();
			return diff <= replicate(epsilon);
		}
		
		ALWAYS_INLINE bool any_equal_within(const Self& other, ElementType epsilon) const {
			return simd::any_ones<N>(equal_within(other, epsilon).m);
		}
		
		template <typename T = ElementType>
		ALWAYS_INLINE
		typename std::enable_if<!IsFloatingPoint<T>::Value, bool>::type
		all_equal(const Self& other) const {
			return simd::all_ones<N>((*this == other).m);
		}
				
		template <typename T = ElementType>
		ALWAYS_INLINE typename std::enable_if<IsFloatingPoint<T>::Value, bool>::type
		any_equal(const Self& other, MaskElementType ulps = 5) const {
			auto abs_diff = MaskVector(this->mask - other.mask).abs();
			return simd::any_ones(abs_diff <= MaskVector::replicate(ulps));
		}
		
		template <typename T = ElementType>
		ALWAYS_INLINE typename std::enable_if<!IsFloatingPoint<T>::Value, bool>::type
		any_equal(const Self& other) const {
			return simd::any_ones(*this == other);
		}
		
		template <typename T = ElementType>
		ALWAYS_INLINE typename std::enable_if<IsFloatingPoint<T>::Value, bool>::type
		all_is_nan() const {
			// TODO: Do this by masking and summing.
			size_t n = 0;
			for (size_t i = 0; i < N; ++i) {
				if (isnan((*this)[i])) ++n;
			}
			return n == N;
		}
		
		template <typename T = ElementType>
		ALWAYS_INLINE typename std::enable_if<IsFloatingPoint<T>::Value, bool>::type
		any_is_nan() const {
			// TODO: Do this by masking and summing.
			size_t n = 0;
			for (size_t i = 0; i < N; ++i) {
				if (isnan((*this)[i])) ++n;
			}
			return n > 0;
		}
		
		template <typename T = ElementType>
		ALWAYS_INLINE typename std::enable_if<IsFloatingPoint<T>::Value, bool>::type
		all_is_infinity() const {
			// TODO: Do this by masking and summing.
			size_t n = 0;
			for (size_t i = 0; i < N; ++i) {
				if (isinf((*this)[i])) ++n;
			}
			return n == N;
		}
		
		template <typename T = ElementType>
		ALWAYS_INLINE typename std::enable_if<IsFloatingPoint<T>::Value, bool>::type
		any_is_infinity() const {
			// TODO: Do this by masking and summing.
			size_t n = 0;
			for (size_t i = 0; i < N; ++i) {
				if (isinf((*this)[i])) ++n;
			}
			return n > 0;
		}
		
		// Arithmetic
		
		ALWAYS_INLINE Self& operator+=(Self other) { this->m += other.m; return *this; }
		ALWAYS_INLINE Self operator+(Self other) const { return Self(this->m + other.m); }
		ALWAYS_INLINE Self& operator-=(Self other) { this->m -= other.m; return *this; }
		ALWAYS_INLINE Self operator-(Self other) const { return Self(this->m - other.m); }
		ALWAYS_INLINE Self operator-() const;
		ALWAYS_INLINE Self& operator*=(Self other) { this->m *= other.m; return *this; }
		ALWAYS_INLINE Self operator*(Self other) const { return Self(this->m * other.m); }
		ALWAYS_INLINE Self& operator/=(Self other) { this->m /= other.m; return *this; }
		ALWAYS_INLINE Self operator/(Self other) const { return Self(this->m / other.m); }
		
		
		// Masking
		
		ALWAYS_INLINE Self operator&(MaskVector msk) const { return this->mask & msk.mask; }
		ALWAYS_INLINE Self operator|(MaskVector msk) const { return this->mask | msk.mask; }
		ALWAYS_INLINE Self operator^(MaskVector msk) const { return this->mask ^ msk.mask; }
		ALWAYS_INLINE Self operator~() const { return ~this->mask; }
		ALWAYS_INLINE Self& operator&=(MaskVector  msk) { this->mask &= msk.m; return *this; }
		ALWAYS_INLINE Self& operator|=(MaskVector msk) { this->mask |= msk.m; return *this; }
		ALWAYS_INLINE Self& operator^=(MaskVector msk) { this->mask ^= msk.m; return *this; }
		
		Self abs() const;
		Self sumv() const;
		ElementType sum() const;
		Self round() const; // if >0: ceil, if <0: floor
		Self floor() const;
		Self ceil() const;
		

		// Convenience
		
		static TVector<ElementType, N> replicate(ElementType value) {
			Self v;
			for (size_t i = 0; i < N; ++i) {
				v[i] = value;
			}
			return v;
		}
		
		ALWAYS_INLINE static constexpr Self zero() { return replicate(0); }
		ALWAYS_INLINE static constexpr Self one() { return replicate(1); }
		ALWAYS_INLINE static constexpr Self two() { return replicate(2); }
		ALWAYS_INLINE static constexpr Self nan() { return zero() / zero(); }
		ALWAYS_INLINE static constexpr Self infinity() { return one() / zero(); }
		
		// Geometry
		
		Self normalize() const;
		Self normalize_safe() const;
		Self lengthv() const;
		ElementType length() const;
		ElementType dot(Self other) const;
		
		ALWAYS_INLINE Self sqrt() const { return simd::sqrt(this->m); }
		ALWAYS_INLINE Self rsqrt() const { return simd::rsqrt(this->m); }
		ALWAYS_INLINE Self fast_sqrt() const { return rsqrt() * (*this); }
		
		template <size_t N_ = N>
		ALWAYS_INLINE typename std::enable_if<N_ == 3, Self>::type
		cross(Self other) const {
			return VectorCrosser<ElementType,N>::cross(*this, other);
		}
	};
	
	// Tuple-like interface
	template <size_t Idx, typename T, size_t N>
	typename std::enable_if<(Idx < N), T&>::type
	get(TVector<T,N>& vector) {
		return vector.v[Idx];
	}
	template <size_t Idx, typename T, size_t N>
	constexpr typename std::enable_if<(Idx < N), T>::type
	get(const TVector<T,N>& vector) {
		return vector.v[Idx];
	}
	
	using vec4 = TVector<float32, 4>;
	using vec3 = TVector<float32, 3>;
	using vec2 = TVector<float32, 2>;
	using vec1 = TVector<float32, 1>;
	using ivec4 = TVector<int32, 4>;
	using ivec3 = TVector<int32, 3>;
	using ivec2 = TVector<int32, 2>;
	using ivec1 = TVector<int32, 1>;
	using uvec4 = TVector<uint32, 4>;
	using uvec3 = TVector<uint32, 3>;
	using uvec2 = TVector<uint32, 2>;
	using uvec1 = TVector<uint32, 1>;
	using u8vec4 = TVector<byte, 4>;
	
	extern template struct TVector<float32, 4>;
	extern template struct TVector<float32, 3>;
	extern template struct TVector<float32, 2>;
	extern template struct TVector<int32, 4>;
	extern template struct TVector<int32, 3>;
	extern template struct TVector<int32, 2>;
	extern template struct TVector<uint32, 4>;
	extern template struct TVector<uint32, 3>;
	extern template struct TVector<uint32, 2>;
	
	
	// Conversion
	
	template <typename T, size_t N>
	template <typename U>
	TVector<T, N>::operator TVector<U,N>() const {
		typename simd::GetVectorType<T,N>::Type from = this->m;
		typename simd::GetVectorType<U,N>::Type to;
		simd::convert(from, to);
		return to;
	}
	
	// Comparison
	
	template <typename T, size_t N>
	struct Approximately<TVector<T, N>, Approximation::Epsilon> {
		const TVector<T, N> value;
		const T epsilon;
		
		constexpr TVector<T, N> min() const {
			return value - TVector<T,N>::replicate(epsilon);
		}
		constexpr TVector<T,N> max() const {
			return value + TVector<T,N>::replicate(epsilon);
		}
		bool contains(TVector<T,N> v) const {
			auto abs_diff = (value - v).abs();
			const auto veps = TVector<T,N>::replicate(epsilon);
			auto bool_mask = abs_diff < veps;
			return simd::all_ones<N>(bool_mask.m);
		}
	};
	template <typename T, size_t N>
	struct Approximately<TVector<T, N>, Approximation::ULPs> {
		const TVector<T, N> value;
		using ULPs = typename IntegerTypeOfSize<sizeof(T)>::UnsignedType;
		const ULPs ulps;
		
		bool contains(TVector<T,N> v) const {
			auto diff = value - v;
			auto abs_diff = diff.abs();
			TVector<ULPs, N> ulps_diff(abs_diff.m);
			auto result = ulps_diff <= TVector<ULPs, N>::replicate(ulps);
			return simd::all_ones<N>(result.m);
		}
	};
	
	template <typename T, size_t N>
	Approximately<TVector<T,N>, Approximation::Epsilon> approximately(TVector<T,N> value, T epsilon) {
		return Approximately<TVector<T, N>, Approximation::Epsilon>{value, epsilon};
	}
	template <typename T, size_t N>
	Approximately<TVector<T,N>, Approximation::ULPs> approximately(TVector<T,N> value, typename TVector<T,N>::MaskElementType ulps) {
		return Approximately<TVector<T, N>, Approximation::ULPs>{value, ulps};
	}
	
	
	
	ALWAYS_INLINE vec1 sumv(vec1 vec) { return vec; }
	ALWAYS_INLINE vec2 sumv(vec2 vec) { return vec2(simd::hadd2(vec.m)); }
	ALWAYS_INLINE vec3 sumv(vec3 vec) { return vec3(simd::hadd3(vec.m)); }
	ALWAYS_INLINE vec4 sumv(vec4 vec) { return vec4(simd::hadd4(vec.m)); }
	ALWAYS_INLINE uvec1 sumv(uvec1 vec) { return vec; }
	ALWAYS_INLINE uvec2 sumv(uvec2 vec) { return uvec2(simd::hadd2u(vec.m)); }
	ALWAYS_INLINE uvec3 sumv(uvec3 vec) { return uvec3(simd::hadd3u(vec.m)); }
	ALWAYS_INLINE uvec4 sumv(uvec4 vec) { return uvec4(simd::hadd4u(vec.m)); }
	ALWAYS_INLINE ivec1 sumv(ivec1 vec) { return vec; }
	ALWAYS_INLINE ivec2 sumv(ivec2 vec) { return ivec2(simd::hadd2i(vec.m)); }
	ALWAYS_INLINE ivec3 sumv(ivec3 vec) { return ivec3(simd::hadd3i(vec.m)); }
	ALWAYS_INLINE ivec4 sumv(ivec4 vec) { return ivec4(simd::hadd4i(vec.m)); }
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::sumv() const {
		return falling::sumv(*this);
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE T TVector<T,N>::sum() const {
		return sumv()[0];
	}
	
	ALWAYS_INLINE float32 sqrt(float32 f) {
		return ::sqrtf(f);
	}
	
	ALWAYS_INLINE float64 sqrt(float64 f) {
		return ::sqrt(f);
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> sqrt(TVector<T,N> vec) {
		return vec.sqrt();
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> rsqrt(TVector<T,N> vec) {
		return vec.rsqrt();
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> fast_sqrt(TVector<T,N> vec) {
		return vec.fast_sqrt();
	}
	
	template <typename ElementType, size_t VectorSize>
	struct AssignVectorElements {
		typedef TVector<ElementType, VectorSize> Vector;
		
		template <size_t Idx>
		static typename std::enable_if<Idx == VectorSize>::type // If you hit this error, you tried to instantiate a vector with the wrong number of constructor arguments. :)
		assign_element(Vector&) {}
		
		template <size_t Idx, typename... Rest>
		ALWAYS_INLINE static void assign_element(Vector& vector, ElementType element, Rest&&... rest) {
			get<Idx>(vector) = element;
			assign_element<Idx+1>(vector, std::forward<Rest>(rest)...);
		}
		
		template <typename... Elements>
		ALWAYS_INLINE static void assign(Vector& vector, Elements&&... elements) {
			assign_element<0>(vector, std::forward<Elements>(elements)...);
		}
	};
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::normalize() const {
		return (*this) / lengthv();
	}
	
	void throw_normalize_zero_length_vector_exception();
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::normalize_safe() const {
		auto v = lengthv();
		if (v == approximately(zero(), 0)) {
			throw_normalize_zero_length_vector_exception();
		}
		return (*this) / v;
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::lengthv() const {
		TVector<T,N> sq = (*this) * (*this);
		return falling::sqrt(sq.sumv());
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE T TVector<T,N>::length() const {
		return get<0>(lengthv());
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE T TVector<T,N>::dot(Self other) const {
		auto product = (*this) * other;
		return product.sum();
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::abs() const {
		return simd::abs(this->m);
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::round() const {
		return simd::round(this->m);
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::floor() const {
		return simd::floor(this->m);
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::ceil() const {
		return simd::ceil(this->m);
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T,N> TVector<T,N>::operator-() const {
		return simd::neg(this->m);
	}
	
	template <Axis X_, Axis Y_, Axis Z_, Axis W_, typename T>
	ALWAYS_INLINE TVector<T, 4> shuffle(TVector<T, 4> vec) {
		return simd::shuffle<X_, Y_, Z_, W_>(vec.m);
	}
	
	template <Axis X_, Axis Y_, Axis Z_, typename T>
	ALWAYS_INLINE TVector<T, 3> shuffle(TVector<T, 3> vec) {
		static_assert(X_ < 3 && Y_ < 3 && Z_ < 3, "No such axis for a 3-dimensional vector.");
		return simd::shuffle<X_, Y_, Z_, W>(vec.m);
	}
	
	template <Axis X_, Axis Y_, typename T>
	ALWAYS_INLINE TVector<T, 2> shuffle(TVector<T, 2> vec) {
		static_assert(X_ < 2 && Y_ < 2, "No such axis for a 2-dimensional vector.");
		return simd::shuffle<X_, Y_>(vec.m);
	}
	
	template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, Axis Z_, size_t WVector, Axis W_, typename T>
	ALWAYS_INLINE TVector<T, 4> shuffle2(TVector<T, 4> vecx, TVector<T, 4> vecy) {
		static_assert(X_ < 4 && Y_ < 4, "No such axis for a 4-dimensional vector.");
		static_assert(XVector < 4, "Vector index out of bounds.");
		static_assert(YVector < 4, "Vector index out of bounds.");
		return simd::shuffle2<XVector, X_, YVector, Y_, ZVector, Z_, WVector, W_>(vecx.m, vecy.m);
	}
	
	template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, Axis Z_, typename T>
	ALWAYS_INLINE TVector<T, 3> shuffle2(TVector<T, 3> vecx, TVector<T, 3> vecy) {
		static_assert(X_ < 3 && Y_ < 3 && Z_ < 3, "No such axis for a 3-dimensional vector.");
		static_assert(XVector < 3, "Vector index out of bounds.");
		static_assert(YVector < 3, "Vector index out of bounds.");
		return simd::shuffle2<XVector, X_, YVector, Y_, ZVector, Z_>(vecx.m, vecy.m);
	}
	
	template <size_t XVector, Axis X_, size_t YVector, Axis Y_, typename T>
	ALWAYS_INLINE TVector<T, 2> shuffle2(TVector<T, 2> vecx, TVector<T, 2> vecy) {
		static_assert(X_ < 2 && Y_ < 2, "No such axis for a 2-dimensional vector.");
		static_assert(XVector < 2, "Vector index out of bounds.");
		static_assert(YVector < 2, "Vector index out of bounds.");
		return simd::shuffle2<XVector, X_, YVector, Y_>(vecx.m, vecy.m);
	}
	
	template <typename T>
	ALWAYS_INLINE TVector<T,4> rotate_right1(TVector<T,4> vec) {
		return shuffle<W,X,Y,Z>(vec);
	}
	
	template <size_t I, typename T>
	ALWAYS_INLINE TVector<T,4> rotate_right(TVector<T,4> vec) {
		for (size_t i = 0; i < I; ++i) {
			vec = rotate_right1(vec);
		}
		return vec;
	}
	
	template <typename T>
	ALWAYS_INLINE TVector<T,4> rotate_left1(TVector<T,4> vec) {
		return shuffle<Y,Z,W,X>(vec);
	}
	
	template <size_t I, typename T>
	ALWAYS_INLINE TVector<T,4> rotate_left(TVector<T,4> vec) {
		for (size_t i = 0; i < I; ++i) {
			vec = rotate_left1(vec);
		}
		return vec;
	}
	
	template <typename T>
	ALWAYS_INLINE TVector<T,4> shift_right1(TVector<T,4> vec) {
		vec = rotate_right1(vec);
		vec.x = 0;
		return vec;
	}
	
	template <size_t I, typename T>
	ALWAYS_INLINE TVector<T,4> shift_right(TVector<T,4> vec) {
		for (size_t i = 0; i < I; ++i) {
			vec = shift_right1(vec);
		}
		return vec;
	}
	
	template <Axis A_, typename T>
	ALWAYS_INLINE TVector<T, 4> splat(TVector<T, 4> vec) {
		return shuffle<A_, A_, A_, A_>(vec);
	}
	
	template <Axis A_, typename T>
	ALWAYS_INLINE TVector<T, 3> splat(TVector<T, 3> vec) {
		return shuffle<A_, A_, A_>(vec);
	}
	
	template <Axis A_, typename T>
	ALWAYS_INLINE TVector<T, 2> splat(TVector<T, 2> vec) {
		return shuffle<A_, A_>(vec);
	}
	
	template <typename T> struct VectorCrosser<T,3> {
		typedef TVector<T,3> V;
		inline static V cross(V a, V b) {
			V a_left = shuffle<Y,Z,X>(a);
			V b_left = shuffle<Z,X,Y>(b);
			V left = a_left * b_left;
			V a_right = shuffle<Z,X,Y>(a);
			V b_right = shuffle<Y,Z,X>(b);
			V right = a_right * b_right;
			return left - right;
		}
	};
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T, N> select(typename TVector<T,N>::ComparisonResult cmp, TVector<T,N> if_true, TVector<T,N> if_false) {
		return (if_true & cmp) | (if_false & ~cmp).mask;
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T, N> min(TVector<T,N> a, TVector<T,N> b) {
#if defined(__arm__)
		return select(a < b, a, b);
#else
		return simd::min(a.m, b.m);
#endif
	}
	
	template <typename T, size_t N>
	ALWAYS_INLINE TVector<T, N> max(TVector<T,N> a, TVector<T,N> b) {
#if defined(__arm__)
		return select(a > b, a, b);
#else
		return simd::max(a.m, b.m);
#endif
	}
}


#endif
