//
//  util.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 25/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_util_hpp
#define falling_util_hpp

#include "io/input_stream.hpp"
#include "base/array.hpp"

namespace falling {
	inline size_t read_all(InputStream& is, Array<byte>& buffer) {
		if (is.has_length()) {
			buffer.reserve(is.length());
		}
		size_t n;
		do {
			byte b[1024];
			n = is.read(b, 1024);
			buffer.insert(b, b + n);
		} while (n > 0);
		return buffer.size();
	}
	
	inline Array<byte> read_all(InputStream& is, IAllocator& alloc = default_allocator()) {
		Array<byte> buffer(alloc);
		read_all(is, buffer);
		return buffer;
	}
}


#endif
