#include "type/maybe_type.hpp"

namespace falling {
	
std::string build_maybe_type_name(const Type* inner_type) {
	StringStream ss;
	ss << "Maybe<" << inner_type->name() << '>';
	return ss.str();
}

}