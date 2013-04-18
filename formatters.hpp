//
//  formatters.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_formatters_hpp
#define falling_formatters_hpp

#include "io/formatted_stream.hpp"
#include "io/string_stream.hpp"
#include "base/array.hpp"
#include "base/matrix.hpp"
#include "base/array_list.hpp"
#include "base/color.hpp"
#include "base/function.hpp"
#include "base/time.hpp"

namespace falling {
	template <typename T>
	struct PrintfFormatter : Formatter {
		String v;
		PrintfFormatter(const char* format_string, T value) {
			char* ret;
			asprintf(&ret, format_string, value);
			v = ret;
			free(ret);
		}
		void write(FormattedStream& stream) const override { stream << v; }
	};
	
	template <typename T>
	PrintfFormatter<T> format(const char* format_string, T value) {
		return PrintfFormatter<T>(format_string, value);
	}
	
	template <typename T>
	struct RepeatFormatter : Formatter {
		T object;
		uint32 times;
		RepeatFormatter(T object, uint32 times) : object(move(object)), times(times) {}
		void write(FormattedStream& stream) const final {
			for (uint32 i = 0; i < times; ++i) {
				stream << object;
			}
		}
	};
	template <typename T>
	RepeatFormatter<T> repeat(T object, uint32 times) {
		return RepeatFormatter<T>(move(object), times);
	}
	
	template <typename T>
	struct TruncateFormatter : Formatter {
		const T& value;
		uint32 width;
		String ellipsis;
	};
	
	struct ClosureFormatter : Formatter {
		Function<void(FormattedStream&)> closure;
		void write(FormattedStream& stream) const {
			closure(stream);
		};
	};
	
	inline ClosureFormatter closure_formatter(Function<void(FormattedStream&)> closure) {
		ClosureFormatter formatter;
		formatter.closure = closure;
		return formatter;
	}
	
	template <typename T>
	ClosureFormatter truncate(const T& value, uint32 width, StringRef ellipsis = "") {
		return closure_formatter([=](FormattedStream& stream) {
			StringStream ss;
			ss << value;
			auto s = ss.str();
			if (s.size() > width) {
				if (ellipsis.size() && width > ellipsis.size()) {
					stream << s.substr(0, -ellipsis.size()) << ellipsis;
				} else {
					stream << s.substr(0, width);
				}
			} else {
				stream << s;
			}
		});
	}
	
	template <typename T>
	struct PadOrTruncateFormatter : Formatter {
		const T& value;
		uint32 width;
		char padding;
		bool pad_left;
		PadOrTruncateFormatter(const T& input, uint32 width, char padding, bool pad_left) : value(input), width(width), padding(padding), pad_left(pad_left) {}
		void write(FormattedStream& stream) const {
			StringStream ss;
			ss << value;
			String s = ss.str();
			if (s.size() > width) {
				stream << s.substr(0, width);
			} else {
				size_t missing = width - s.size();
				if (pad_left) {
					for (uint32 i = 0; i < missing; ++i) {
						stream << padding;
					}
				}
				stream << s;
				if (!pad_left) {
					for (uint32 i = 0; i < missing; ++i) {
						stream << padding;
					}
				}
			}
		}
	};
	
	template <typename T>
	PadOrTruncateFormatter<T> pad_or_truncate(const T& value, uint32 width, char padding = ' ', bool pad_left = false) {
		return PadOrTruncateFormatter<T>(value, width, padding, pad_left);
	}
	
	template <typename Container>
	struct JoinFormatter : Formatter {
		const Container& container;
		const char* delimiter;
		JoinFormatter(const Container& c, const char* delimiter) : container(c), delimiter(delimiter) {}
		void write(FormattedStream& stream) const {
			for (auto it = container.begin(); it != container.end();) {
				stream << *it;
				++it;
				if (it != container.end()) {
					stream << delimiter;
				}
			}
		}
	};
	
	template <typename Container>
	JoinFormatter<Container> join(const Container& c, const char* delimiter = ", ") {
		return JoinFormatter<Container>(c, delimiter);
	}
	
	struct DataSizeFormatter : Formatter {
		size_t bytes;
		const uint32 digits;
		DataSizeFormatter(size_t bytes, uint32 digits = 2) : bytes(bytes), digits(digits) {}
		void write(FormattedStream& stream) const override {
			if (bytes == SIZE_MAX) {
				stream << "∞";
			} else if (bytes >= 1024*1024) {
				size_t mbs = bytes / (1024*1024);
				size_t fraction = bytes % (1024*1024);
				stream << mbs << '.' << pad_or_truncate(fraction, 2, '0', false) << "M";
			} else if (bytes >= 1024) {
				size_t kbs = bytes / 1024;
				size_t fraction = bytes % 1024;
				stream << kbs << '.' << pad_or_truncate(fraction, 2, '0', false) << "K";
			} else {
				stream << bytes << "B";
			}
		}
	};
	
	inline DataSizeFormatter format_data_size(size_t sz, uint32 digits = 2) {
		return DataSizeFormatter(sz, digits);
	}
	
	template <Timeline T>
	struct TimeDeltaFormatter : Formatter {
		TimeDelta<T> delta;
		TimeDeltaFormatter(TimeDelta<T> delta) : delta(delta) {}
		void write(FormattedStream& os) const override {
			uint64 ns = delta.nanoseconds();
			if (ns > 1000000000) {
				uint64 seconds = ns / 1000000000;
				uint64 rem = (ns % 1000000000) / 1000;
				uint64 rem_ms = rem / 1000000;
				os << seconds << "." << pad_or_truncate(rem_ms, 3, '0', true) << " s";
			} else if (ns > 1000000) {
				uint64 milliseconds = ns / 1000000;
				uint64 rem = (ns % 1000000) / 1000;
				os << milliseconds << '.' << pad_or_truncate(rem, 3, '0', true) << " ms";
			} else if (ns > 1000) {
				uint64 microseconds = ns / 1000;
				uint64 rem = ns % 1000;
				os << microseconds << '.' << pad_or_truncate(rem, 3, '0', true) << " µs";
			} else {
				os << ns << " ns";
			}
		}
	};
	
	template <Timeline T>
	TimeDeltaFormatter<T> format_time_delta(TimeDelta<T> delta) {
		return TimeDeltaFormatter<T>(delta);
	}
	
	template <typename T>
	FormattedStream& operator<<(FormattedStream& stream, const Array<T>& array) {
		stream << '[';
		stream << join(array, ", ");
		stream << ']';
		return stream;
	}
	
	template <typename T>
	FormattedStream& operator<<(FormattedStream& stream, const ArrayRef<T>& array) {
		stream << '[';
		stream << join(array, ", ");
		stream << ']';
		return stream;
	}
	
	template <typename T, bool C>
	FormattedStream& operator<<(FormattedStream& stream, const LinearMemoryIterator<T, C>& it) {
		stream << it.get();
		return stream;
	}
	
	template <typename T, size_t N, size_t M>
	FormattedStream& operator<<(FormattedStream& os, const TMatrix<T, N, M>& mat) {
		os << '{';
		for (size_t row = 0; row < M; ++row) {
			os << '{';
			for (size_t col = 0; col < N; ++col) {
				os << mat.row_at(row)[col];
				if (col != N-1) {
					os << ", ";
				}
			}
			os << '}';
		}
		os << '}';
		return os;
	}
	
	template <typename T>
	FormattedStream& operator<<(FormattedStream& stream, const ArrayList<T>& array) {
		stream << "@[";
		stream << join(array, ", ");
		stream << ']';
		return stream;
	}
	
	inline FormattedStream& operator<<(FormattedStream& stream, CompactColor color) {
		stream << "rgba(" << color.red() << ", " << color.green() << ", " << color.blue() << ", " << color.alpha() << ")";
		return stream;
	}
}

#endif
