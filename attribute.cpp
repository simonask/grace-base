//
//  attribute.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "type/attribute.hpp"
#include "base/log.hpp"

namespace falling {
	void AttributeBase::warn_set_any_wrong_type(const Type *expected, const Type *got) const {
		Warning() << "Wrong variant type in dynamic attribute assignment to '" << name() << "'. expected value of type " << expected->name() << ", got " << got->name() << ".";
	}
}
