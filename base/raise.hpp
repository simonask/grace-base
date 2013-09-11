#pragma once
#ifndef GRACE_RAISE_HPP_INCLUDED
#define GRACE_RAISE_HPP_INCLUDED

#include "base/error.hpp"
#include "io/string_stream.hpp"
#include "io/formatters.hpp"
#include "base/string.hpp"
#include "io/printf.hpp"

namespace grace {
	template <typename ErrorType>
	void raise() {
		throw ErrorType();
	}

	template <typename ErrorType, typename... Args>
	void raise(StringRef format, Args&&... args) {
		StringStream ss;
		ss.printf(format, std::forward<Args>(args)...);
		size_t len = ss.buffer().size();
		IAllocator& alloc = default_allocator();
		char* description = (char*)alloc.allocate(len, alignof(char));
		ss.buffer().copy_to(description, description + len);
		ErrorType err;
		err._take_description(description, len, alloc);
		throw err;
	}
}

#endif
