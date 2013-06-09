//
//  util.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 25/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_util_hpp
#define grace_util_hpp

#include "io/input_stream.hpp"
#include "base/array.hpp"

namespace grace {
	template <typename ContainerType>
	inline size_t read_all(InputStream& is, ContainerType& buffer) {
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
	
	template <typename ContainerType>
	inline ContainerType read_all(InputStream& is, IAllocator& alloc = default_allocator()) {
		ContainerType buffer(alloc);
		read_all(is, buffer);
		return move(buffer);
	}
	
	bool path_exists(StringRef path);
	bool path_is_file(StringRef path);
	bool path_is_directory(StringRef path);
}


#endif
