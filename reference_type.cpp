#include "reference_type.hpp"
#include "io/string_stream.hpp"

namespace falling {

std::string ReferenceType::build_reference_type_name(std::string base_name, const Type* pointee) {
	StringStream ss;
	ss << base_name << '<' << pointee->name() << '>';
	return ss.str();
}

}