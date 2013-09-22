#pragma once
#ifndef GRACE_RAISE_HPP_INCLUDED
#define GRACE_RAISE_HPP_INCLUDED

#include "base/error.hpp"
#include "io/string_stream.hpp"
#include "base/string.hpp"
#include "io/printf.hpp"

namespace grace {
	template <typename ErrorType>
	__attribute__((noreturn))
	void raise() {
		throw ErrorType();
	}

	template <typename ErrorType, typename... Args>
	ErrorType make_error(StringRef format, Args&&... args) {
		StringStream ss;
		ss.printf(format, std::forward<Args>(args)...);
		size_t len = ss.buffer().size();
		IAllocator& alloc = default_allocator();
		char* description = (char*)alloc.allocate(len, alignof(char));
		ss.buffer().copy_to(description, description + len);
		ErrorType err;
		err._take_description(description, len, alloc);
		return std::move(err);
	}

	template <typename ErrorType, typename... Args>
	__attribute__((noreturn))
	void raise(StringRef format, Args&&... args) {
		ErrorType err = make_error<ErrorType>(format, std::forward<Args>(args)...);
		throw ErrorType(std::move(err));
	}
}

#endif
