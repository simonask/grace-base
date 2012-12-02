#include "type/array_type.hpp"
#include "io/string_stream.hpp"

namespace falling {
	
String build_variable_length_array_type_name(String base_name, const Type* element_type) {
	StringStream ss;
	ss << element_type->name() << "[]";
	return ss.str();
}

}