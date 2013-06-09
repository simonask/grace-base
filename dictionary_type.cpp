//
//  dictionary_type.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 06/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "type/dictionary_type.hpp"
#include "io/string_stream.hpp"

namespace grace {
	String build_dictionary_type_name(IAllocator& alloc, const Type* value_type) {
		StringStream ss;
		ss << "Dictionary<" << value_type->name() << ">";
		return ss.string(alloc);
	}
}
