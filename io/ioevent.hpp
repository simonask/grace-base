#pragma once
#ifndef GRACE_IOERROR_HPP_INCLUDED
#define GRACE_IOERROR_HPP_INCLUDED

#include "base/error.hpp"

namespace grace {
	struct IOError : ErrorBase<IOError> {};

	enum class IOEvent {
		EndOfStream,
		WouldBlock,
	};
}

#endif