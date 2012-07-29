//
//  stream.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 21/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_stream_hpp
#define falling_stream_hpp

#include "base/basic.hpp"

namespace falling {
	class InputStream {
		virtual bool is_readable() const = 0;
		virtual size_t read(byte* buffer, size_t max) = 0;
		virtual size_t tell_read() const = 0;
		virtual bool seek_read(size_t position) = 0;
	};
}

#endif
