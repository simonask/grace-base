//
//  math.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 31/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_math_hpp
#define grace_math_hpp

#include <cmath>
#include "geometry/vector.hpp"

namespace grace {
	static constexpr const float32 E = M_E;
	static constexpr const float32 Log2E = M_LOG2E;
	static constexpr const float32 Log10E = M_LOG10E;
	static constexpr const float32 Ln2 = M_LN2;
	static constexpr const float32 Ln10 = M_LN10;
	static constexpr const float32 Pi = M_PI;
	static constexpr const float32 Tau = Pi * 2;
	static constexpr const float32 PiOver2 = M_PI_2;
	static constexpr const float32 PiOver4 = M_PI_4;
	static constexpr const float32 OneOverPi = M_1_PI;
	static constexpr const float32 TwoOverPi = M_2_PI;
	static constexpr const float32 TwoOverSqrtPi = M_2_SQRTPI;
	static constexpr const float32 Sqrt2 = M_SQRT2;
	static constexpr const float32 OneOverSqrt2 = M_SQRT1_2;

	template <typename T, size_t N>
	TVector<T,N> sin_precise(TVector<T,N> vec) {
		TVector<T,N> r;
		for (size_t i = 0; i < N; ++i) {
			r[i] = std::sin(vec[i]);
		}
		return r;
	}
	
	template <typename T, size_t N>
	TVector<T,N> sin(TVector<T,N> vec) {
		static const auto LowerBound = TVector<T,N>::replicate(-Pi);
		static const auto UpperBound = TVector<T,N>::replicate(Pi);
		static const auto ClampAdd   = TVector<T,N>::two() * UpperBound;
		static const auto Zero = TVector<T,N>::zero();

		auto add_mask = vec < LowerBound;
		auto sub_mask = vec > UpperBound;
		vec += select(add_mask, ClampAdd, Zero);
		vec -= select(sub_mask, ClampAdd, Zero);

		static const auto C1 = TVector<T,N>::replicate(1.27323954);
		static const auto C2 = TVector<T,N>::replicate(0.405284735);
		static const auto C3 = TVector<T,N>::replicate(0.225);
		
		
		const auto c1vec = C1 * vec;
		const auto c2vecvec = C2 * vec * vec;

		// for x < 0
		auto rb0 = c1vec + c2vecvec;
		// for rb0 < 0
		auto rb0b0 = C3 * (rb0 * -rb0 - rb0) + rb0;
		// for rb0 >= 0
		auto rb0a0 = C3 * (rb0 *  rb0 - rb0) + rb0;
		rb0 = select(rb0 < Zero, rb0b0, rb0a0);

		// x >= 0
		auto ra0 = c1vec - c2vecvec;
		// for ra0 < 0
		auto ra0b0 = C3 * (ra0 * -ra0 - ra0) + ra0;
		// for ra0 >= 0
		auto ra0a0 = C3 * (ra0 *  ra0 - ra0) + ra0;
		ra0 = select(ra0 < Zero, ra0b0, ra0a0);

		auto r = select(vec < Zero, rb0, ra0);
		return r;
	}
	
	template <typename T, size_t N>
	TVector<T,N> cos_precise(TVector<T,N> vec) {
		TVector<T,N> r;
		for (size_t i = 0; i < N; ++i) {
			r[i] = std::cos(vec[i]);
		}
		return r;
	}
	
	template <typename T, size_t N>
	TVector<T,N> cos(TVector<T,N> vec) {
		// TODO: Calculate cos with its own approximation instead of in terms of sin
		// to avoid unnecessary fp math imprecision.
		// cos(x) == -sin(x - π/2)
		return -sin(vec - TVector<T,N>::replicate(PiOver2));
	}

	template <typename T, size_t N>
	TVector<T,N> tan_precise(TVector<T,N> vec) {
		TVector<T,N> r;
		for (size_t i = 0; i < N; ++i) {
			r[i] = std::tan(vec[i]);
		}
		return r;
	}
	
	template <typename T, size_t N>
	TVector<T,N> tan(TVector<T,N> vec) {
		// TODO: Calculate tan with its own approximation instead of in terms of sin/cos
		// to avoid unnecessary fp math imprecision (and excess computation).
		return sin(vec) / cos(vec);
	}

	template <typename T, size_t N>
	TVector<T,N> acos_precise(TVector<T,N> vec) {
		TVector<T,N> r;
		for (size_t i = 0; i < N; ++i) {
			r[i] = std::acos(vec[i]);
		}
		return r;
	}
	
	template <typename T, size_t N>
	TVector<T,N> acos(TVector<T,N> vec) {
		// TODO: Optimize with approximation
		return acos_precise(vec);
	}

	template <typename T, size_t N>
	TVector<T,N> asin_precise(TVector<T,N> vec) {
		TVector<T,N> r;
		for (size_t i = 0; i < N; ++i) {
			r[i] = std::asin(vec[i]);
		}
		return r;
	}
	
	template <typename T, size_t N>
	TVector<T,N> asin(TVector<T,N> vec) {
		// TODO: Optimize with approximation
		return asin_precise(vec);
	}

	template <typename T, size_t N>
	TVector<T,N> atan_precise(TVector<T,N> vec) {
		TVector<T,N> r;
		for (size_t i = 0; i < N; ++i) {
			r[i] = std::atan(vec[i]);
		}
		return r;
	}
	
	template <typename T, size_t N>
	TVector<T,N> atan(TVector<T,N> vec) {
		// TODO: Optimize with approximation
		return atan_precise(vec);
	}
	
	template <typename A, typename B>
	inline constexpr auto max(A a, B b) -> typename std::common_type<A, B>::type {
		return a > b ? a : b;
	}
	template <typename A, typename B>
	inline constexpr auto min(A a, B b) -> typename std::common_type<A ,B>::type {
		return a < b ? a : b;
	}
	
	inline constexpr float32 floor(float32 f) {
		return (float32)((int32)f-0.5f);
	}
	inline constexpr float32 ceil(float32 f) {
		return (float32)((int32)f+0.5f);
	}
		
	template <typename T, size_t N>
	inline TVector<T,N> floor(TVector<T,N> v) {
		return v.floor();
	}
		
	template <typename T, size_t N>
	inline TVector<T, N> ceil(TVector<T,N> v) {
		return v.ceil();
	}
	
	template <typename T>
	constexpr T clamp_mod_down(T n, T min, T max) {
		return grace::min(n - (max-min) * grace::floor((max-n) / -(max-min)), min);
	}
	
	template <typename T>
	constexpr T clamp_mod_up(T n, T min, T max) {
		return grace::max(n + (max-min) * grace::ceil((min-n)/(max-min)), max);
	}
	
	/*
	 clamp_mod:
	 - if n > min && n < max, returns n.
	 - otherwise adds or subtracts (max-min) to n until the result is between min and max.
	 */
	template <typename T>
	constexpr T clamp_mod(T n, T min, T max) {
		return n > max ? clamp_mod_down(n, min, max) : (n < min ? clamp_mod_up(n, min, max) : n);
	}
	
	/*
	 clamp_mod_once:
	 - if n > min && n < max, returns n.
	 - otherwise adds or subtracts (max-min) to/from n just once.
	*/
	template <typename T>
	constexpr T clamp_mod_once(T n, T min, T max) {
		return n > max ? n-(max-min) : n+(max-min);
	}
	template <typename T, size_t N>
	inline TVector<T, N> clamp_mod_once(TVector<T,N> v, TVector<T,N> min, TVector<T,N> max) {
		const TVector<T,N> diff = max - min;
		auto r = v;
		r += select(v < min, diff, TVector<T,N>::zero());
		r -= select(v > max, diff, TVector<T,N>::zero());
		return r;
	}
	
	template <typename T>
	constexpr T clamp(T n, T min, T max) {
		return n > max ? max : (n < min ? min : n);
	}
	
	template <typename T>
	T abs(T value) {
		return value < 0 ? -value : value;
	}
}

#endif
