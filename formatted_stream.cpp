//
//  formatted_stream.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/formatted_stream.hpp"
#include "io/formatters.hpp"

namespace falling {
	void FormattedStream::write_formatted(const std::string& str) {
		write((const byte*)str.c_str(), str.size());
	}
	
	void FormattedStream::write_formatted(uint8 n) {
		write_formatted(format("%u", n));
	}
	
	void FormattedStream::write_formatted(uint16 n) {
		write_formatted(format("%u", n));
	}
	
	void FormattedStream::write_formatted(uint32 n) {
		write_formatted(format("%u", n));
	}
	
	void FormattedStream::write_formatted(uint64 n) {
		write_formatted(format("%llu", n));
	}
	
	void FormattedStream::write_formatted(char c) {
		write((const byte*)&c, 1);
	}
	
	void FormattedStream::write_formatted(int8 n) {
		write_formatted(format("%d", n));
	}
	
	void FormattedStream::write_formatted(int16 n) {
		write_formatted(format("%d", n));
	}
	
	void FormattedStream::write_formatted(int32 n) {
		write_formatted(format("%d", n));
	}
	
	void FormattedStream::write_formatted(int64 n) {
		write_formatted(format("%lld", n));
	}
	
	void FormattedStream::write_formatted(float32 f) {
		write_formatted(format("%f", f));
	}
	
	void FormattedStream::write_formatted(float64 f) {
		write_formatted(format("%llf", f));
	}
	
	void FormattedStream::write_formatted(const Formatter& formatter) {
		formatter.write(*this);
	}
}
