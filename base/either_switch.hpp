#pragma once
#ifndef GRACE_EITHER_SWITCH_HPP_INCLUDED
#define GRACE_EITHER_SWITCH_HPP_INCLUDED

#include "base/either.hpp"
#include <tuple>
#include <array>

namespace grace {
	namespace detail {
		struct IEitherSwitchCaller {
			virtual void call() = 0;
		};

		template <typename T, typename F, typename EitherType>
		struct EitherSwitchCaller : IEitherSwitchCaller {
			EitherSwitchCaller(F& f, EitherType& e) : function(f), either(e) {}
			F& function;
			EitherType& either;
			void call() final {
				function(either.template get<T>());
			}
		};

		template <typename EitherType, typename CallersTuple, size_t... I>
		void either_switch_impl(EitherType& either, CallersTuple& callers_impls, Indices<I...>) {
			std::array<IEitherSwitchCaller*, sizeof...(I)> callers {{&std::get<I>(callers_impls)...}};
			if (either.which() < callers.size()) {
				callers[either.which()]->call();
			}
		}
	}

	template <typename... Types, typename... Functions>
	void either_switch(Either<Types...>& either, Functions... functions) {
		using Indices = typename MakeIndices<sizeof...(Functions)>::Type;
		using EitherType = Either<Types...>;
		auto caller_impls = std::make_tuple(detail::EitherSwitchCaller<Types, Functions, EitherType>(functions, either)...);
		detail::either_switch_impl(either, caller_impls, Indices());
	}

	template <typename... Types, typename... Functions>
	void either_switch(const Either<Types...>& either, Functions... functions) {
		using Indices = typename MakeIndices<sizeof...(Functions)>::Type;
		using EitherType = const Either<Types...>;
		auto caller_impls = std::make_tuple(detail::EitherSwitchCaller<Types, Functions, EitherType>(functions, either)...);
		detail::either_switch_impl(either, caller_impls, Indices());
	}
}

#endif
