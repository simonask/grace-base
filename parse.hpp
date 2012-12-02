//
//  parse.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 02/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_parse_hpp
#define falling_parse_hpp

#include "base/maybe.hpp"
#include "base/string.hpp"
#include "base/stack_array.hpp"

#include <type_traits>

namespace falling {
	template <typename T>
	struct Parser;

	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value
		&& std::is_unsigned<T>::value,
		Maybe<T>
	>::type
	parse(StringRef input, int8 base = 10) {
		ASSERT(base >= 2 && base <= 36);
		if (input.size() == 0) return Nothing;
		unsigned long long n;
		COPY_STRING_REF_TO_CSTR_BUFFER(buffer, input);
		char* endp;
		n = strtoull(buffer.data(), &endp, base);
		if (endp != buffer.data()) {
			return (T)n;
		}
		return Nothing;
	}
	
	template <typename T>
	typename std::enable_if<
	std::is_integral<T>::value
	&& std::is_signed<T>::value,
	Maybe<T>
	>::type
	parse(StringRef input, int8 base = 10) {
		ASSERT(base >= 2 && base <= 36);
		if (input.size() == 0) return Nothing;
		long long n;
		COPY_STRING_REF_TO_CSTR_BUFFER(buffer, input);
		char* endp;
		n = strtoll(buffer.data(), &endp, base);
		if (endp != buffer.data()) {
			return (T)n;
		}
		return Nothing;
	}
}

#endif
