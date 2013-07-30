//
//  random.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/07/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef grace_random_hpp
#define grace_random_hpp

#include "base/basic.hpp"

#include <random>
#include <limits>

namespace grace {
	struct Random {
		Random();
		explicit Random(uint32 seed) : engine_(seed) {}
		
		template <typename T>
		T between(T min, T max);
		
		template <typename T>
		T normal(T mean, T standard_deviation);
	private:
		std::default_random_engine engine_;
	};
	
	template <typename T, bool IsFloat> struct UniformDistributionForTypeImpl;
	template <typename T> struct UniformDistributionForTypeImpl<T, true> {
		using Type = std::uniform_real_distribution<T>;
	};
	template <typename T> struct UniformDistributionForTypeImpl<T, false> {
		using Type = std::uniform_int_distribution<T>;
	};
	template <typename T> struct UniformDistributionForType {
		using Type = typename UniformDistributionForTypeImpl<T, IsFloatingPoint<T>::Value>::Type;
	};
	
	template <typename T>
	T Random::between(T min, T max) {
		using Distribution = typename UniformDistributionForType<T>::Type;
		Distribution dist(min, max);
		return dist(engine_);
	}
	
	template <typename T>
	T Random::normal(T mean, T standard_deviation) {
		std::normal_distribution<T> dist;
		return dist(mean, standard_deviation);
	}
}

#endif
