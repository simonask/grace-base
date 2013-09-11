//
//  string.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/string.hpp"
#include "base/exceptions.hpp"
#include "base/raise.hpp"

namespace grace {
	namespace detail {
		void string_index_of_out_bounds_exception(size_t idx, size_t sz) {
			raise<IndexOutOfBoundsException>("Requested character {0} from a string of length {1}.", idx, sz);
		}
	}

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
	
	StringRef substr(StringRef str, ssize_t pos, ssize_t len) {
		size_t begin;
		if (pos >= 0) {
			begin = pos;
		} else {
			// pos < 0
			if (-pos > str.size()) {
				begin = 0;
			} else {
				begin = str.size() + pos;
			}
		}
		if (begin > str.size()) begin = str.size();
		size_t end;
		if (len >= 0) {
			if (len > str.size() - begin) {
				len = str.size() - begin;
			}
			end = begin + len;
		} else {
			// len < 0
			if (-len > str.size() - begin) {
				end = begin;
			} else {
				end = str.size() + len;
			}
		}
		return StringRef(str.data() + begin, str.data() + end);
	}
	
	size_t find(StringRef str, const StringRef& needle) {
		if (needle.size() == 0) return str.size();
		if (str.size() == 0) return String::NPos;
		if (needle.size() > str.size()) return String::NPos;
		
		for (size_t i = 0; i < str.size() - needle.size(); ++i) {
			if (substr(str, i, needle.size()) == needle) return i;
		}
		return String::NPos;
	}
	
	size_t find(StringRef str, char needle) {
		if (str.size() == 0) return String::NPos;
		for (ssize_t i = 0; i < str.size(); ++i) {
			if (str[i] == needle) {
				return i;
			}
		}
		return String::NPos;
	}
	
	size_t rfind(StringRef str, const StringRef& needle) {
		if (needle.size() == 0) return str.size();
		if (str.size() == 0) return String::NPos;
		if (needle.size() > str.size()) return String::NPos;
		
		for (size_t i = str.size() - needle.size(); i > 0; --i) {
			if (substr(str, i, needle.size()) == needle) return i;
		}
		return String::NPos;
	}
	
	size_t rfind(StringRef str, char needle) {
		if (str.size() == 0) return String::NPos;
		for (ssize_t i = str.size()-1; i >= 0; --i) {
			if (str[i] == needle) {
				return i;
			}
		}
		return String::NPos;
	}

	StringRef lstrip(StringRef input, char c) {
		const char* p = input.data();
		const char* end = input.data() + input.size();
		while (*p == c && p != end) {
			++p;
		}
		return StringRef(p, end);
	}

	StringRef rstrip(StringRef input, char c) {
		const char* p = input.data();
		const char* end = input.data() + input.size();
		while (end != p && *(end-1) == c) {
			--end;
		}
		return StringRef(p, end);
	}
	
	StringRef strip(StringRef input, char c) {
		return rstrip(lstrip(input, c), c);
	}
}
