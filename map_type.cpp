//
//  map_type.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 10/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "type/map_type.hpp"
#include "io/string_stream.hpp"

namespace falling {
	String MapType::build_map_type_name() {
		StringStream ss;
		ss << "Map<" << key_type()->name() << ", " << value_type()->name() << '>';
		return ss.string();
	}
}
