//
//  string.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 02/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/string.hpp"

namespace falling {
	String String::take_ownership(IAllocator &alloc, const char *utf8, size_t size) {
		String s(alloc);
		s.data_ = utf8;
		s.size_ = size;
		return s;
	}
	
	String concatenate(StringRef a, StringRef b, IAllocator& alloc) {
		char* buffer = (char*)alloc.allocate(a.size() + b.size(), 1);
		std::copy(a.begin(), a.end(), buffer);
		std::copy(b.begin(), b.end(), buffer+a.size());
		return String::take_ownership(alloc, buffer, a.size() + b.size());
	}
}
