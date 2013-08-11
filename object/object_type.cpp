#include "object/object_type.hpp"

namespace grace {

const StructuredType* ObjectTypeBase::super() const {
	if (super_ != nullptr) return super_;
	const ObjectTypeBase* object_type = get_type<Object>();
	return object_type != this ? object_type : nullptr;
}
	
}