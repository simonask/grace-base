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
	namespace detail {
		void warn_set_any_wrong_type(StringRef property_name, const IType *expected, const IType *got) {
			Warning() << "Wrong variant type in dynamic attribute assignment to '" << property_name << "'. expected value of type " << expected->name() << ", got " << got->name() << ".";
		}
	}
}
