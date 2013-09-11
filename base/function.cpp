#include "base/function.hpp"
#include "base/function_error.hpp"
#include "base/raise.hpp"

namespace grace {
	namespace detail {
		void empty_function_call_error() {
			raise<EmptyFunctionCallError>();
		}
	}
}