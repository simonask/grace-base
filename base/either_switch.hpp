#pragma once
#ifndef GRACE_EITHER_SWITCH_HPP_INCLUDED
#define GRACE_EITHER_SWITCH_HPP_INCLUDED

#include "base/either.hpp"
#include <tuple>
#include <array>

namespace grace {
	namespace detail {
		template <size_t I, typename... Types, typename EitherType>
		void either_switch_static_impl(EitherType& either) {
			// Base case for fewer functions than types.
		}

		template <size_t I, typename EitherType, typename Function, typename... FunctionsTail>
		void either_switch_static_impl(EitherType& either, Function function, FunctionsTail&&... tail) {
			// Base case for fewer types than functions.
		}

		template <size_t I, typename T, typename... Rest, typename EitherType, typename Function, typename... FunctionsTail>
		void either_switch_static_impl(EitherType& either, Function function, FunctionsTail&&... tail) {
			if (I == either.which()) {
				function(either.template get<T>());
			} else {
				either_switch_static_impl<I+1, Rest...>(either, std::forward<FunctionsTail>(tail)...);
			}
		}
	}

	template <typename... Types, typename... Functions>
	void either_switch(Either<Types...>& either, Functions&&... functions) {
		static_assert(!(sizeof...(Types) < sizeof...(Functions)), "Too many functions for either_switch.");
		static_assert(!(sizeof...(Types) > sizeof...(Functions)), "Unhandled case in either_switch.");
		detail::either_switch_static_impl<0, Types...>(either, std::forward<Functions>(functions)...);
	}

	template <typename... Types, typename... Functions>
	void either_switch(const Either<Types...>& either, Functions&&... functions) {
		static_assert(!(sizeof...(Types) < sizeof...(Functions)), "Too many functions for either_switch.");
		static_assert(!(sizeof...(Types) > sizeof...(Functions)), "Unhandled case in either_switch.");
		detail::either_switch_static_impl<0, Types...>(either, std::forward<Functions>(functions)...);
	}
}

#endif
