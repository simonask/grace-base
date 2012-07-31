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
	
	struct Formatter {
		virtual void write(FormattedStream&) const = 0;
	};
	
	class FormattedStream : public OutputStream {
	public:
		FormattedStream(OutputStream& stream) : stream_(stream) {}
		
		bool is_writable() const final { return stream_.is_writable(); }
		size_t write(const byte* buffer, size_t max) final { return stream_.write(buffer, max); }
		size_t tell_write() const final { return stream_.tell_write(); }
		bool seek_write(size_t position) final { return stream_.seek_write(position); }
		
		template <typename T>
		FormattedStream& operator<<(const T& object) {
			write_formatted(object);
			return *this;
		}
		
		void write_formatted(const char* cstr);
		void write_formatted(const std::string&);
		void write_formatted(bool b);
		void write_formatted(uint8);
		void write_formatted(uint16);
		void write_formatted(uint32);
		void write_formatted(uint64);
		void write_formatted(unsigned long n) { write_formatted((uint32)n); }
		void write_formatted(char);
		void write_formatted(int8);
		void write_formatted(int16);
		void write_formatted(int32);
		void write_formatted(int64);
		void write_formatted(float32);
		void write_formatted(float64);
		void write_formatted(const Formatter&);
		template <typename T, size_t N>
		void write_formatted(TVector<T, N> v);
	protected:
		OutputStream& stream_;
	};
	
	template <typename T, size_t N>
	void FormattedStream::write_formatted(TVector<T, N> v) {
		FormattedStream& self = *this;
		self << '{';
		for (size_t i = 0; i < N; ++i) {
			self << v[i];
			if (i != N-1) {
				self << ", ";
			}
		}
		self << '}';
	}
}

#endif
