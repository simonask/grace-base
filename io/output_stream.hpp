//
//  output_stream.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_output_stream_hpp
#define grace_output_stream_hpp

namespace grace {
	struct IOutputStream {
		virtual bool is_writable() const = 0;
		virtual size_t write(const byte* buffer, size_t max) = 0;
		virtual size_t tell_write() const = 0;
		virtual bool seek_write(size_t position) = 0;
		virtual void flush() = 0;
	};
	
	struct IOutputStreamNonblocking {
		virtual size_t write_nonblocking(const byte* buffer, size_t max, bool& out_would_block) = 0;
		virtual bool is_write_nonblocking() const = 0;
		virtual void set_write_nonblocking(bool b) = 0;
	};
}

#endif
