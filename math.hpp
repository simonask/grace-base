//
//  math.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 31/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_math_hpp
#define falling_math_hpp

#include <cmath>
#include "base/vector.hpp"

namespace falling {
	static const float32 E = M_E;
	static const float32 Log2E = M_LOG2E;
	static const float32 Log10E = M_LOG10E;
	static const float32 Ln2 = M_LN2;
	static const float32 Ln10 = M_LN10;
	static const float32 Pi = M_PI;
	static const float32 PiOver2 = M_PI_2;
	static const float32 PiOver4 = M_PI_4;
	static const float32 OneOverPi = M_1_PI;
	static const float32 TwoOverPi = M_2_PI;
	static const float32 TwoOverSqrtPi = M_2_SQRTPI;
	static const float32 Sqrt2 = M_SQRT2;
	static const float32 OneOverSqrt2 = M_SQRT1_2;
	
	inline float32 rad2deg(float32 rad) {
		return rad * 180.f / Pi;
	}
	
	inline float32 deg2rad(float32 deg) {
		return deg * Pi / 180.f;
	}

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

		auto add_mask = vec < LowerBound;
		auto sub_mask = vec > UpperBound;
		vec += ClampAdd & add_mask;
		vec -= ClampAdd & sub_mask;

		static const auto C1 = TVector<T,N>::replicate(1.27323954);
		static const auto C2 = TVector<T,N>::replicate(0.405284735);
		static const auto C3 = TVector<T,N>::replicate(0.225);
		static const auto Zero = TVector<T,N>::zero();
		
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
	constexpr auto max(A a, B b) -> typename std::common_type<A, B>::type {
		return a > b ? a : b;
	}
}

#endif
