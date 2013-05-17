//
//  type_traits.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 04/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_type_traits_hpp
#define falling_type_traits_hpp

#include <type_traits>
#include "base/math.hpp"

namespace falling {
	// === MaxSize ===
	template <typename... Types> struct MaxSize;
	template <> struct MaxSize<> {
		static const size_t Value = 0;
	};
	template <typename T, typename... Rest> struct MaxSize<T, Rest...> {
		static const size_t Value = max(sizeof(T), MaxSize<Rest...>::Value);
	};
	
	// === MaxAlignment ===
	template <typename... Types> struct MaxAlignment;
	template <> struct MaxAlignment<> {
		static const size_t Value = 0;
	};
	template <typename T, typename... Rest> struct MaxAlignment<T, Rest...> {
		static const size_t Value = max(alignof(T), MaxAlignment<Rest...>::Value);
	};
	
	// === IndexOfType ===
	template <typename T, size_t I, typename... Types> struct IndexOfTypeImpl;
	template <typename T, size_t I, typename Current, typename... Rest> struct IndexOfTypeImpl<T, I, Current, Rest...> {
		static const size_t Value = std::is_same<T, Current>::value ? I : IndexOfTypeImpl<T, I+1, Rest...>::Value;
	};
	template <typename T, size_t I> struct IndexOfTypeImpl<T, I> {
		static const size_t Value = SIZE_MAX;
	};
	template <typename T, typename... Types> struct IndexOfType {
		static const size_t Value = IndexOfTypeImpl<T, 0, Types...>::Value;
	};
	
	// === AlignedUnion ===
	// TODO: Use std::aligned_union once libc++ catches up with C++11.
	template <typename... Types>
	struct AlignedUnion {
		using Type = typename std::aligned_storage<MaxSize<Types...>::Value, MaxAlignment<Types...>::Value>::type;
	};
	
	// AllTypesSatisfyTrait_standard
	template <template <typename T> class Trait, typename... Types> struct AllTypesSatifyTrait_standard;
	template <template <typename T> class Trait, typename First, typename... Rest> struct AllTypesSatifyTrait_standard<Trait, First, Rest...> {
		static const bool Value = Trait<First>::value && AllTypesSatifyTrait_standard<Trait, Rest...>::Value;
	};
	template <template <typename T> class Trait> struct AllTypesSatifyTrait_standard<Trait> {
		static const bool Value = true;
	};
	// AnyTypesSatisfyTrait_standard
	template <template <typename T> class Trait, typename... Types> struct AnyTypesSatifyTrait_standard;
	template <template <typename T> class Trait, typename First, typename... Rest> struct AnyTypesSatifyTrait_standard<Trait, First, Rest...> {
		static const bool Value = Trait<First>::value || AllTypesSatifyTrait_standard<Trait, Rest...>::Value;
	};
	template <template <typename T> class Trait> struct AnyTypesSatifyTrait_standard<Trait> {
		static const bool Value = false;
	};
	// AllTypesSatisfyTrait_falling
	template <template <typename T> class Trait, typename... Types> struct AllTypesSatifyTrait_falling;
	template <template <typename T> class Trait, typename First, typename... Rest> struct AllTypesSatifyTrait_falling<Trait, First, Rest...> {
		static const bool Value = Trait<First>::Value && AllTypesSatifyTrait_falling<Trait, Rest...>::Value;
	};
	template <template <typename T> class Trait> struct AllTypesSatifyTrait_falling<Trait> {
		static const bool Value = true;
	};
	// AnyTypesSatisfyTrait_falling
	template <template <typename T> class Trait, typename... Types> struct AnyTypesSatifyTrait_falling;
	template <template <typename T> class Trait, typename First, typename... Rest> struct AnyTypesSatifyTrait_falling<Trait, First, Rest...> {
		static const bool Value = Trait<First>::Value || AllTypesSatifyTrait_falling<Trait, Rest...>::Value;
	};
	template <template <typename T> class Trait> struct AnyTypesSatifyTrait_falling<Trait> {
		static const bool Value = false;
	};
	
	// AreAllCopyConstructible
	template <typename... Types> struct AreAllCopyConstructible {
		static const bool Value = AllTypesSatifyTrait_standard<std::is_copy_constructible, Types...>::Value;
	};
	// AreAllCopyAssignable
	template <typename... Types> struct AreallCopyAssignable {
		static const bool Value = AllTypesSatifyTrait_standard<std::is_copy_assignable, Types...>::Value;
	};
	// AreAllMoveConstructible
	template <typename... Types> struct AreAllMoveConstructible {
		static const bool Value = AllTypesSatifyTrait_standard<std::is_move_constructible, Types...>::Value;
	};
	// AreAllMoveAssignable
	template <typename... Types> struct AreallMoveAssignable {
		static const bool Value = AllTypesSatifyTrait_standard<std::is_move_assignable, Types...>::Value;
	};
}

#endif
