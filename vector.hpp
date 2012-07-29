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


namespace falling {

	namespace internal {
		using namespace simd;
		
		template <typename ElementType, size_t N> struct GetVectorType;
		template <> struct GetVectorType<int32, 1>   { typedef int32  Type; typedef uint32 MaskType; };
		template <> struct GetVectorType<int32, 2>   { typedef ivec2  Type; typedef uvec2  MaskType; };
		template <> struct GetVectorType<int32, 3>   { typedef ivec4  Type; typedef uvec4  MaskType; };
		template <> struct GetVectorType<int32, 4>   { typedef ivec4  Type; typedef uvec4  MaskType; };
		template <> struct GetVectorType<int64, 2>   { typedef ilvec2 Type; typedef ulvec2 MaskType; };
		template <> struct GetVectorType<int64, 3>   { typedef ilvec4 Type; typedef ulvec4 MaskType; };
		template <> struct GetVectorType<int64, 4>   { typedef ilvec4 Type; typedef ulvec4 MaskType; };
		template <> struct GetVectorType<uint32, 1>  { typedef uint32 Type; typedef uint32 MaskType; };
		template <> struct GetVectorType<uint32, 2>  { typedef uvec2  Type; typedef uvec2  MaskType; };
		template <> struct GetVectorType<uint32, 3>  { typedef uvec4  Type; typedef uvec4  MaskType; };
		template <> struct GetVectorType<uint32, 4>  { typedef uvec4  Type; typedef uvec4  MaskType; };
		template <> struct GetVectorType<uint64, 2>  { typedef ulvec2 Type; typedef ulvec2 MaskType; };
		template <> struct GetVectorType<uint64, 3>  { typedef ulvec4 Type; typedef ulvec4 MaskType; };
		template <> struct GetVectorType<uint64, 4>  { typedef ulvec4 Type; typedef ulvec4 MaskType; };
		
		template <> struct GetVectorType<float32, 1> { typedef float32 Type; typedef uint32  MaskType; };
		template <> struct GetVectorType<float32, 2> { typedef fvec2  Type; typedef uvec2  MaskType; };
		template <> struct GetVectorType<float32, 3> { typedef fvec4  Type; typedef uvec4  MaskType; };
		template <> struct GetVectorType<float32, 4> { typedef fvec4  Type; typedef uvec4  MaskType; };
		template <> struct GetVectorType<float64, 1> { typedef float64 Type; typedef uint64 MaskType; };
		template <> struct GetVectorType<float64, 2> { typedef flvec2 Type; typedef ulvec2 MaskType; };
		template <> struct GetVectorType<float64, 3> { typedef flvec4 Type; typedef ulvec4 MaskType; };
		template <> struct GetVectorType<float64, 4> { typedef flvec4 Type; typedef ulvec4 MaskType; };
	}
	
	template <typename T, size_t N>
	struct VectorCrosser;
	
	template <typename ElementType, size_t N> struct VectorDataDefinition {
		typedef internal::GetVectorType<ElementType, N> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskType MaskType;
	};
	
	template <typename ElementType, size_t N> struct VectorData;
	template <typename T> struct VectorData<T, 1> {
		typedef internal::GetVectorType<T, 1> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskType MaskType;
		
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
		typedef internal::GetVectorType<T, 2> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskType MaskType;
		
		union {
			Type m;
			MaskType mask;
			T v[2];
			struct {
				T x;
				T y;
			};
		};
	};
	template <typename T> struct VectorData<T, 3> {
		typedef internal::GetVectorType<T, 3> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskType MaskType;
		
		union {
			Type m;
			MaskType mask;
			T v[3];
			struct {
				T x;
				T y;
				T z;
			};
		};
	};
	template <typename T> struct VectorData<T, 4> {
		typedef internal::GetVectorType<T, 4> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskType MaskType;
		
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
		};
	};
	template <typename T, size_t N> struct VectorData {
		typedef internal::GetVectorType<T, N> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskType MaskType;
		
		union {
			Type m;
			MaskType mask;
			T v[N];
		};
	};
	
	template <typename ElementType, size_t N>
	struct TVector : VectorData<ElementType, N> {
		typedef internal::GetVectorType<ElementType, N> GetVectorType;
		typedef typename GetVectorType::Type Type;
		typedef typename GetVectorType::MaskType MaskType;
		typedef ElementType ComponentType;
		typedef TVector<ElementType, N> Self;
		
		typedef TVector<MaskType, N> ComparisonResult;
		
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
		
		TVector() {}
		explicit TVector(const ElementType* elements) { simd::unaligned_load(this->m, elements); }
		explicit TVector(ElementType elements[N])     { simd::unaligned_load(this->m, elements); }
		TVector(const Self& other) { this->m = other.m; }
		TVector(Type repr) { this->m = repr; }
		
		
		template <typename Enable = void>
		explicit TVector(ElementType x, typename std::enable_if<N == 1, Enable*>::type = nullptr) { this->m = Type{x}; }
		template <typename Enable = void>
		TVector(ElementType x, ElementType y, typename std::enable_if<N == 2, Enable*>::type = nullptr) { this->m = Type{x, y}; }
		template <typename Enable = void>
		TVector(ElementType x, ElementType y, ElementType z, typename std::enable_if<N == 3, Enable*>::type = nullptr) { this->m = Type{x, y, z}; }
		template <typename Enable = void>
		TVector(ElementType x, ElementType y, ElementType z, ElementType w, typename std::enable_if<N == 4, Enable*>::type = nullptr) { this->m = Type{x, y, z, w}; }
		
		Self& operator=(Self other) { this->m = other.m; return *this; }
		
		
		
		// Comparison
		
		ComparisonResult operator<(const Self& other) const { return simd::cmp_lt(this->m, other.m); }
		ComparisonResult operator<=(const Self& other) const { return simd::cmp_lte(this->m, other.m); }
		ComparisonResult operator>(const Self& other) const { return simd::cmp_gt(this->m, other.m); }
		ComparisonResult operator>=(const Self& other) const { return simd::cmp_gte(this->m, other.m); }
		
		template <typename T = ElementType>
		typename std::enable_if<!IsFloatingPoint<T>::Value, ComparisonResult>::type
		operator==(const Self& other) const { return simd::cmp_eq(this->m, other.m); }
		
		template <typename T = ElementType>
		typename std::enable_if<!IsFloatingPoint<T>::Value, ComparisonResult>::type
		operator!=(const Self& other) const { return simd::cmp_neq(this->m, other.m); }
		
		
		
		// Arithmetic
		
		Self& operator+=(Self other) { this->m += other.m; return *this; }
		Self operator+(Self other) { return Self(this->m + other.m); }
		Self& operator-=(Self other) { this->m -= other.m; return *this; }
		Self operator-(Self other) { return Self(this->m - other.m); }
		Self& operator*=(Self other) { this->m *= other.m; return *this; }
		Self operator*(Self other) { return Self(this->m * other.m); }
		Self& operator/=(Self other) { this->m /= other.m; return *this; }
		Self operator/(Self other) { return Self(this->m / other.m); }
		
		
		// Masking
		
		MaskType operator&(MaskType msk) const { return this->mask & msk; }
		MaskType operator|(MaskType msk) const { return this->mask | msk; }
		MaskType operator^(MaskType msk) const { return this->mask ^ msk; }
		MaskType operator~() const { return ~this->mask; }
		Self& operator&=(MaskType  msk) { this->mask &= msk; return *this; }
		Self& operator|=(MaskType msk) { this->mask |= msk; return *this; }
		Self& operator^=(MaskType msk) { this->mask ^= msk; return *this; }
		

		// Convenience
		static TVector<ElementType, N> replicate(ElementType value) {
			Self v;
			for (size_t i = 0; i < N; ++i) {
				v[i] = value;
			}
			return v;
		}
		
		static constexpr Self zero() { return replicate(0); }
		static constexpr Self one() { return replicate(1); }
		static constexpr Self two() { return replicate(2); }
		
		// Geometry
		
		Self normalized() const;
		Self lengthv() const;
		ElementType length() const;
		ElementType dot(Self other) const;
	
		Self cross(Self other) const {
			return VectorCrosser<ElementType,N>::cross(*this, other);
		}
	};
	
	// Tuple-like interface
	template <size_t Idx, typename T, size_t N>
	T& get(TVector<T,N>& vector) {
		return vector.v[Idx];
	}
	template <size_t Idx, typename T, size_t N>
	constexpr T get(const TVector<T,N>& vector) {
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
	
	
	inline vec1 sumv(vec1 vec) { return vec; }
	inline vec2 sumv(vec2 vec) { return vec2(simd::hadd2(vec.m)); }
	inline vec3 sumv(vec3 vec) { return vec3(simd::hadd3(vec.m)); }
	inline vec4 sumv(vec4 vec) { return vec4(simd::hadd4(vec.m)); }
	
	inline vec4 sqrt(vec4 vec) {
		return vec4(simd::sqrt(vec.m));
	}
	
	template <typename ElementType, size_t VectorSize>
	struct AssignVectorElements {
		typedef TVector<ElementType, VectorSize> Vector;
		
		template <size_t Idx>
		static typename std::enable_if<Idx == VectorSize>::type // If you hit this error, you tried to instantiate a vector with the wrong number of constructor arguments. :)
		assign_element(Vector&) {}
		
		template <size_t Idx, typename... Rest>
		static void assign_element(Vector& vector, ElementType element, Rest&&... rest) {
			get<Idx>(vector) = element;
			assign_element<Idx+1>(vector, std::forward<Rest>(rest)...);
		}
		
		template <typename... Elements>
		static void assign(Vector& vector, Elements&&... elements) {
			assign_element<0>(vector, std::forward<Elements>(elements)...);
		}
	};
	
	template <typename T, size_t N>
	TVector<T,N> TVector<T,N>::normalized() const {
		return (*this) / lengthv();
	}
	
	template <typename T, size_t N>
	TVector<T,N> TVector<T,N>::lengthv() const {
		TVector<T,N> sq = (*this) * (*this);
		return sqrt(sumv(sq));
	}
	
	template <typename T, size_t N>
	T TVector<T,N>::length() const {
		return get<0>(lengthv());
	}
	
	template <typename T, size_t N>
	T TVector<T,N>::dot(Self other) const {
		auto product = (*this) * other;
		return sumv(product);
	}
	
	template <Axis X_, Axis Y_, Axis Z_, Axis W_, typename T>
	TVector<T, 4> shuffle(TVector<T, 4> vec) {
		return simd::shuffle<X_, Y_, Z_, W_>(vec.m);
	}
	
	template <Axis X_, Axis Y_, Axis Z_, typename T>
	TVector<T, 3> shuffle(TVector<T, 3> vec) {
		return simd::shuffle<X_, Y_, Z_, W>(vec.m);
	}
	
	template <Axis X_, Axis Y_, typename T>
	TVector<T, 2> shuffle(TVector<T, 2> vec) {
		return simd::shuffle<X_, Y_>(vec.m);
	}
	
	template <Axis A_, typename T>
	TVector<T, 4> splat(TVector<T, 4> vec) {
		return shuffle<A_, A_, A_, A_>(vec);
	}
	
	template <Axis A_, typename T>
	TVector<T, 3> splat(TVector<T, 3> vec) {
		return shuffle<A_, A_, A_>(vec);
	}
	
	template <Axis A_, typename T>
	TVector<T, 2> splat(TVector<T, 2> vec) {
		return shuffle<A_, A_>(vec);
	}
	
	template <typename T> struct VectorCrosser<T,3> {
		typedef TVector<T,3> V;
		static V cross(V a, V b) {
			V a_left = shuffle<Y,Z,X>(a);
			V b_left = shuffle<Z,X,Y>(b);
			V left = a_left * b_left;
			V a_right = shuffle<Z,X,Y>(a);
			V b_right = shuffle<Y,Z,X>(b);
			V right = a_right * b_right;
			return left - right;
		}
	};
}


#endif
