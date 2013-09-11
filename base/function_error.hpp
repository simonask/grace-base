#pragma once
#ifndef GRACE_FUNCTION_ERROR_HPP_INCLUDED
#define GRACE_FUNCTION_ERROR_HPP_INCLUDED

#include "base/error.hpp"

namespace grace {
	struct EmptyFunctionCallError : ErrorBase<EmptyFunctionCallError> {};
}

#endif