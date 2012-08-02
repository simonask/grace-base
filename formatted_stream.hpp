//
//  formatted_stream.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_formatted_stream_hpp
#define falling_formatted_stream_hpp

#include "base/basic.hpp"
#include "io/output_stream.hpp"
#include "base/vector.hpp"
#include <stdio.h>
#include <string>

namespace falling {
	class FormattedStream;
	
	class FormattedStream : public OutputStream {
	public:
		FormattedStream(OutputStream& stream) : stream_(stream) {}
		
		bool is_writable() const final { return stream_.is_writable(); }
		size_t write(const byte* buffer, size_t max) final { return stream_.write(buffer, max); }
		size_t tell_write() const final { return stream_.tell_write(); }
		bool seek_write(size_t position) final { return stream_.seek_write(position); }
	protected:
		OutputStream& stream_;
	};

	FormattedStream& operator<<(FormattedStream& stream, const char* cstr);
	FormattedStream& operator<<(FormattedStream& stream, const std::string&);
	FormattedStream& operator<<(FormattedStream& stream, bool b);
	FormattedStream& operator<<(FormattedStream& stream, uint8);
	FormattedStream& operator<<(FormattedStream& stream, uint16);
	FormattedStream& operator<<(FormattedStream& stream, uint32);
	FormattedStream& operator<<(FormattedStream& stream, uint64);
	FormattedStream& operator<<(FormattedStream& stream, char);
	FormattedStream& operator<<(FormattedStream& stream, int8);
	FormattedStream& operator<<(FormattedStream& stream, int16);
	FormattedStream& operator<<(FormattedStream& stream, int32);
	FormattedStream& operator<<(FormattedStream& stream, int64);
	FormattedStream& operator<<(FormattedStream& stream, float32);
	FormattedStream& operator<<(FormattedStream& stream, float64);
	inline FormattedStream& operator<<(FormattedStream& stream, unsigned long n) { return stream << (uint32)n; }
	
	template <size_t N>
	FormattedStream& operator<<(FormattedStream& stream, const char str[N]) {
		stream.write((const byte*)str, N);
	}
	
	// One-shot streams overload.
	template <typename T>
	void operator<<(FormattedStream&& stream, T value) {
		stream << value;
	}

	struct Formatter {
		virtual void write(FormattedStream&) const = 0;
	};

	inline FormattedStream& operator<<(FormattedStream& stream, const Formatter& formatter) {
		formatter.write(stream);
		return stream;
	}

	template <typename T, size_t N>
	inline FormattedStream& operator<<(FormattedStream& stream, TVector<T, N> v) {
		stream << '{';
		for (size_t i = 0; i < N; ++i) {
			stream << v[i];
			if (i != N-1) {
				stream << ", ";
			}
		}
		stream << '}';
		return stream;
	}
}

#endif
