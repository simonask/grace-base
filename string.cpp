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
	
	StringRef substr(StringRef str, size_t pos, size_t len) {
		size_t begin = pos;
		if (begin > str.size()) begin = str.size();
		if (len > str.size() - begin) {
			len = str.size() - begin;
		}
		size_t end = begin + len;
		return StringRef(str.data() + begin, str.data() + end);
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
	
	StringRef strip(StringRef input) {
		const char* p = input.data();
		const char* end = input.data() + input.size();
		while (*p == ' ' && p != end) {
			++p;
		}
		if (p != end) {
			do {
				--end;
			} while (*end == ' ' && end != p);
			++end;
		}
		return StringRef(p, end);
	}
}
