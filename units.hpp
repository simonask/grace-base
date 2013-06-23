//
//  units.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 22/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef grace_units_hpp
#define grace_units_hpp

#include "type/type.hpp"
#include "base/math.hpp"

#define GRACE_STRONG_TYPEDEF(NEW_TYPE, UNDERLYING_TYPE) \
	struct NEW_TYPE { \
		UNDERLYING_TYPE value; \
		explicit constexpr NEW_TYPE(const UNDERLYING_TYPE v_) : value(v_) {} \
		constexpr NEW_TYPE() : value() {} \
		constexpr NEW_TYPE(const NEW_TYPE& other) : value(other.value) {} \
		NEW_TYPE& operator=(const NEW_TYPE& other) { value = other.value; return *this; } \
		constexpr operator const UNDERLYING_TYPE& () const { return value; } \
		operator UNDERLYING_TYPE&() { return value; } \
		constexpr bool operator==(const NEW_TYPE& other) const { return value == other.value; } \
		constexpr bool operator<(const NEW_TYPE& other) const { return value < other.value; } \
	}; \
	template <> struct BuildTypeInfo<NEW_TYPE> { \
		static auto build() -> decltype(get_type<UNDERLYING_TYPE>()) { \
			return get_type<UNDERLYING_TYPE>(); \
		} \
	}

#define GRACE_STRONG_ARITHMETIC_TYPEDEF_WITH_MODULUS(NEW_TYPE, UNDERLYING_TYPE, MIN, MAX) \
	struct NEW_TYPE { \
		UNDERLYING_TYPE value; \
		explicit constexpr NEW_TYPE(const UNDERLYING_TYPE v_) : value(grace::clamp_mod(v_, MIN, MAX)) {} \
		constexpr NEW_TYPE() : value() {} \
		constexpr NEW_TYPE(const NEW_TYPE& other) : value(other.value) {} \
		NEW_TYPE& operator=(const NEW_TYPE& other) { value = other.value; return *this; } \
		constexpr operator const UNDERLYING_TYPE& () const { return value; } \
		operator UNDERLYING_TYPE() { return value; } \
		constexpr bool operator==(const NEW_TYPE& other) const { return value == other.value; } \
		constexpr bool operator!=(const NEW_TYPE& other) const { return value != other.value; } \
		constexpr bool operator<(const NEW_TYPE& other) const { return value < other.value; } \
		constexpr bool operator>(const NEW_TYPE& other) const { return value > other.value; } \
		constexpr bool operator<=(const NEW_TYPE& other) const { return value <= other.value; } \
		constexpr bool operator>=(const NEW_TYPE& other) const { return value >= other.value; } \
		constexpr NEW_TYPE operator+(const NEW_TYPE& other) const { return NEW_TYPE(value + other.value); } \
		constexpr NEW_TYPE operator-(const NEW_TYPE& other) const { return NEW_TYPE(value - other.value); } \
		constexpr NEW_TYPE operator*(const UNDERLYING_TYPE& scalar) const { return NEW_TYPE(value * scalar); } \
		constexpr NEW_TYPE operator/(const UNDERLYING_TYPE& scalar) const { return NEW_TYPE(value / scalar); } \
		NEW_TYPE operator+=(const NEW_TYPE& other) { value = grace::clamp_mod(value + other.value, MIN, MAX); return *this; } \
		NEW_TYPE operator-=(const NEW_TYPE& other) { value = grace::clamp_mod(value - other.value, MIN, MAX); return *this; } \
		NEW_TYPE operator*=(const UNDERLYING_TYPE& scalar) { value = grace::clamp_mod(value * scalar, MIN, MAX); return *this; } \
		NEW_TYPE operator/=(const UNDERLYING_TYPE& scalar) { value = grace::clamp_mod(value / scalar, MIN, MAX); return *this; } \
	}; \
	template <> struct BuildTypeInfo<NEW_TYPE> { \
		static auto build() -> decltype(get_type<UNDERLYING_TYPE>()) { \
			return get_type<UNDERLYING_TYPE>(); \
		} \
	}

#define GRACE_STRONG_ARITHMETIC_TYPEDEF_WITH_CLAMP(NEW_TYPE, UNDERLYING_TYPE, MIN, MAX) \
	struct NEW_TYPE { \
		UNDERLYING_TYPE value; \
		explicit constexpr NEW_TYPE(const UNDERLYING_TYPE v_) : value(grace::clamp(v_, MIN, MAX)) {} \
		constexpr NEW_TYPE() : value() {} \
		constexpr NEW_TYPE(const NEW_TYPE& other) : value(other.value) {} \
		NEW_TYPE& operator=(const NEW_TYPE& other) { value = other.value; return *this; } \
		constexpr operator const UNDERLYING_TYPE& () const { return value; } \
		operator UNDERLYING_TYPE() { return value; } \
		constexpr bool operator==(const NEW_TYPE& other) const { return value == other.value; } \
		constexpr bool operator!=(const NEW_TYPE& other) const { return value != other.value; } \
		constexpr bool operator<(const NEW_TYPE& other) const { return value < other.value; } \
		constexpr bool operator>(const NEW_TYPE& other) const { return value > other.value; } \
		constexpr bool operator<=(const NEW_TYPE& other) const { return value <= other.value; } \
		constexpr bool operator>=(const NEW_TYPE& other) const { return value >= other.value; } \
		constexpr NEW_TYPE operator+(const NEW_TYPE& other) const { return NEW_TYPE(value + other.value); } \
		constexpr NEW_TYPE operator-(const NEW_TYPE& other) const { return NEW_TYPE(value - other.value); } \
		constexpr NEW_TYPE operator*(const UNDERLYING_TYPE& scalar) const { return NEW_TYPE(value * scalar); } \
		constexpr NEW_TYPE operator/(const UNDERLYING_TYPE& scalar) const { return NEW_TYPE(value / scalar); } \
		NEW_TYPE operator+=(const NEW_TYPE& other) { value = grace::clamp(value + other.value, MIN, MAX); return *this; } \
		NEW_TYPE operator-=(const NEW_TYPE& other) { value = grace::clamp(value - other.value, MIN, MAX); return *this; } \
		NEW_TYPE operator*=(const UNDERLYING_TYPE& scalar) { value = grace::clamp(value * scalar, MIN, MAX); return *this; } \
		NEW_TYPE operator/=(const UNDERLYING_TYPE& scalar) { value = grace::clamp(value / scalar, MIN, MAX); return *this; } \
	}; \
	template <> struct BuildTypeInfo<NEW_TYPE> { \
		static auto build() -> decltype(get_type<UNDERLYING_TYPE>()) { \
			return get_type<UNDERLYING_TYPE>(); \
		} \
	}

namespace grace {
	GRACE_STRONG_ARITHMETIC_TYPEDEF_WITH_MODULUS(Degrees, float32, -180.f, 180.f);
	GRACE_STRONG_ARITHMETIC_TYPEDEF_WITH_MODULUS(Radians, float32, -Tau, Tau);
	
	inline constexpr Degrees operator"" _deg(long double d) {
		return Degrees((float32)d);
	}
	inline constexpr Degrees operator"" _deg(unsigned long long d) {
		return Degrees((float32)d);
	}
	
	inline constexpr Radians operator"" _rad(long double r) {
		return Radians((float32)r);
	}
	inline constexpr Radians operator"" _rad(unsigned long long r) {
		return Radians((float32)r);
	}
	
	inline constexpr Degrees rad2deg(Radians rad) {
		return Degrees(rad.value * 180.f / Pi);
	}
	
	inline constexpr Radians deg2rad(Degrees deg) {
		return Radians(deg.value * Pi / 180.f);
	}
}

#endif
