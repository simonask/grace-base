#pragma once
#ifndef GRACE_PRINTF_HPP_INCLUDED
#define GRACE_PRINTF_HPP_INCLUDED

#include "io/formatted_stream.hpp"
#include "base/string.hpp"
#include "base/parse.hpp"
#include "base/regex.hpp"
#include "memory/static_allocator.hpp"

#include <array>

namespace grace {
	struct IWriteToFormattedStream { virtual void write(FormattedStream&) = 0; };
	template <typename T>
	struct WriteToFormattedStream : IWriteToFormattedStream {
		T& object;
		WriteToFormattedStream(T& x) : object(x) {}
		void write(FormattedStream& os) final {
			os << object;
		}
	};
	template <typename T>
	WriteToFormattedStream<T> make_write_to_formatted_stream(T& x) {
		return WriteToFormattedStream<T>{x};
	}

	template <typename ArgsWriterTuple, size_t... I>
	void formatted_stream_printf_impl(FormattedStream& os, StringRef format, ArgsWriterTuple& tuple, Indices<I...>) {
		std::array<IWriteToFormattedStream*, sizeof...(I)> arg_writers = {{ static_cast<IWriteToFormattedStream*>(&std::get<I>(tuple))... }};

		StringRef haystack = format;
		static const Regex interpolation("{(\\d+)}", static_allocator());
		replace_with_through(haystack, interpolation, os, [&](FormattedStream& s, ArrayRef<StringRef> groups) {
			if (groups.size() != 2) {
				s << groups[0];
			} else {
				Maybe<int> idx = parse<int>(groups[1]);
				if (idx && *idx < arg_writers.size()) {
					arg_writers[*idx]->write(s);
				} else {
					s << groups[0];
				}
			}
		});
	}

	template <typename... Args>
	FormattedStream& FormattedStream::printf(StringRef format, Args&&... args_pack) {
		// Instantiate a number of specialized classes that can turn the arguments into strings:
		auto arg_writers = std::make_tuple(make_write_to_formatted_stream(args_pack)...);
		using Indices = typename MakeIndices<sizeof...(Args)>::Type;
		formatted_stream_printf_impl(*this, format, arg_writers, Indices());
		return *this;
	}

}

#endif
