//
//  stream.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 21/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_stream_hpp
#define grace_stream_hpp

#include "base/basic.hpp"

namespace grace {
	template <typename...> class Either;
	enum class IOEvent;

	struct IInputStream {
		virtual bool is_readable() const = 0;
		virtual bool is_read_nonblocking() const = 0;
		virtual Either<size_t, IOEvent> read(byte* buffer, size_t max) = 0;
		virtual size_t tell_read() const = 0;
		virtual bool seek_read(size_t position) = 0;
		virtual bool has_length() const = 0;
		virtual size_t length() const = 0;
	};
}

#endif
