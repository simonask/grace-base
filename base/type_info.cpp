#include "base/type_info.hpp"
#include "base/error.hpp"
#include "base/raise.hpp"

namespace grace {
	struct UnsupportedTypeOperationError : ErrorBase<UnsupportedTypeOperationError> {};

	void TypeInfo::unsupported(const char* op) const {
		raise<UnsupportedTypeOperationError>("Unsupported operation: {0}", op);
	}
}