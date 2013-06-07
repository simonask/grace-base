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
		EpsilonAndULPs,
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
	
	template <typename T>
	struct Approximately<T, Approximation::ULPs> {
		using ULPs = typename IntegerTypeOfSize<sizeof(T)>::Type;
		const T value;
		const ULPs ulps;
		
		bool contains(T v) const {
			const bool sign_a = value < 0;
			const bool sign_b = v < 0;
			if (sign_a == sign_b) {
				ULPs a = *reinterpret_cast<ULPs*>(&value);
				ULPs b = *reinterpret_cast<ULPs*>(&v);
				return abs(a - b) <= ulps;
			} else {
				return value == v;
			}
		}
	};
	
	template <typename T>
	struct Approximately<T, Approximation::EpsilonAndULPs> {
		using ULPs = typename IntegerTypeOfSize<sizeof(T)>::Type;
		const T value;
		const T epsilon;
		const ULPs ulps;
		
		bool contains(T v) const {
			T absdiff = abs(value - v);
			if (absdiff < epsilon) {
				return true;
			}
			const bool sign_a = value < 0;
			const bool sign_b = v < 0;
			if (sign_a != sign_b) return false;
			
			ULPs a = *reinterpret_cast<ULPs*>(&value);
			ULPs b = *reinterpret_cast<ULPs*>(&v);
			return abs(a - b) <= ulps;
		}
	};
	
	inline constexpr Approximately<float32, Approximation::Epsilon> approximately(float32 value, float32 within) {
		return Approximately<float32, Approximation::Epsilon>{value, within};
	}
	inline Approximately<float32, Approximation::ULPs> approximately(float32 value, int32 ulps) {
		return Approximately<float32, Approximation::ULPs>{value, ulps};
	}
	inline Approximately<float32, Approximation::EpsilonAndULPs> approximately(float32 value, float32 max_diff, int32 max_ulps_diff) {
		return Approximately<float32, Approximation::EpsilonAndULPs>{value, max_diff, max_ulps_diff};
	}
	inline constexpr Approximately<float64, Approximation::Epsilon> approximately(float64 value, float64 within) {
		return Approximately<float64, Approximation::Epsilon>{value, within};
	}
	inline constexpr Approximately<float64, Approximation::ULPs> approximately(float64 value, int64 ulps) {
		return Approximately<float64, Approximation::ULPs>{value, ulps};
	}
	inline Approximately<float64, Approximation::EpsilonAndULPs> approximately(float64 value, float64 max_diff, int64 max_ulps_diff) {
		return Approximately<float64, Approximation::EpsilonAndULPs>{value, max_diff, max_ulps_diff};
	}
	
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, Approximately<T, Approximation::Epsilon>>::type
	approximately(T value, T epsilon) {
		return Approximately<T, Approximation::Epsilon>{value, epsilon};
	}
	
	
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
