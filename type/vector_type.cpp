#include "type/vector_type.hpp"
#include "io/string_stream.hpp"

namespace grace {
	namespace detail {
		String build_vector_type_name(const SimpleType* element_type, size_t n, IAllocator& alloc) {
			ScratchAllocator scratch;
			StringStream ss(scratch);
			if (!element_type->is_float()) {
				if (element_type->is_signed()) {
					ss << 'i';
				} else {
					ss << 'u';
				}
			}
			ss << "vec" << n;
			return ss.string(alloc);
		}
	}
}