//
//  approximately.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 20/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_approximately_hpp
#define falling_approximately_hpp

namespace falling {
	enum class Approximation {
		Epsilon,
		ULPs,
	};

	template <typename T, Approximation A>
	struct Approximately;
	template <typename T>
	struct Approximately<T, Approximation::Epsilon> {
		const T value;
		const T epsilon;
		
		constexpr T min() const { return value - epsilon; }
		constexpr T max() const { return value + epsilon; }
		constexpr bool contains(T v) const {
			return fabsf(value - v) <= epsilon;
		}
	};
	template <size_t N> struct IntegerTypeOfSize;
	template <> struct IntegerTypeOfSize<4> { using Type = int32; using UnsignedType = uint32; };
	template <> struct IntegerTypeOfSize<8> { using Type = int64; using UnsignedType = uint64; };
	
	constexpr Approximately<float32, Approximation::Epsilon> approximately(float32 value, float32 within) {
		return Approximately<float32, Approximation::Epsilon>{value, within};
	}
	/*constexpr Approximately<float32, Approximation::ULPs> approximately(float32 value, uint32 ulps = 5) {
		return Approximately<float32, Approximation::ULPs>{value, ulps};
	}*/
	constexpr Approximately<float64, Approximation::Epsilon> approximately(float64 value, float64 within) {
		return Approximately<float64, Approximation::Epsilon>{value, within};
	}
	/*constexpr Approximately<float64, Approximation::ULPs> approximately(float64 value, uint64 ulps = 5) {
		return Approximately<float64, Approximation::ULPs>{value, ulps};
	}*/
	
	template <typename T, Approximation A>
	inline bool operator==(T value, const Approximately<T, A>& approx) {
		return approx.contains(value);
	}
	template <typename T, Approximation A>
	inline bool operator==(const Approximately<T, A>& approx, T value) {
		return value == approx;
	}
	template <typename T, Approximation A>
	inline bool operator!=(T value, const Approximately<T, A>& approx) {
		return !(value == approx);
	}
	template <typename T, Approximation A>
	inline bool operator!=(const Approximately<T, A>& approx, T value) {
		return !(approx == value);
	}
	
	template <typename OS, typename T>
	inline OS& operator<<(OS& os, const Approximately<T, Approximation::Epsilon>& approx) {
		return os << "approximately " << approx.value << "±" << approx.epsilon;
	}
	template <typename OS, typename T>
	inline OS& operator<<(OS& os, const Approximately<T, Approximation::ULPs>& approx) {
		return os << "approximately " << approx.value << "±U" << approx.ulps;
	}
}

#endif
