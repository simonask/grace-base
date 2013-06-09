#include "reference_type.hpp"
#include "io/string_stream.hpp"

namespace grace {

String ReferenceType::build_reference_type_name(IAllocator& alloc, StringRef base_name, const Type* pointee) {
	StringStream ss;
	ss << base_name << '<' << pointee->name() << '>';
	return ss.string(alloc);
}

}