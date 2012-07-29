//
//  output_stream.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_output_stream_hpp
#define falling_output_stream_hpp

namespace falling {
	class OutputStream {
	public:
		virtual bool is_writable() const = 0;
		virtual size_t write(const byte* buffer, size_t max) = 0;
		virtual size_t tell_write() const = 0;
		virtual bool seek_write(size_t position) = 0;
	};
}

#endif
