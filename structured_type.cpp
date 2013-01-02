//
//  structured_type.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 02/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "type/structured_type.hpp"
#include "object/slot.hpp"
#include "type/attribute.hpp"

namespace falling {
	const ISlot* StructuredType::find_slot_by_name(StringRef name) const {
		for (auto s: slots()) {
			if (s->name() == name) return s;
		}
		return nullptr;
	}
	
	const IAttribute* StructuredType::find_attribute_by_name(StringRef name) const {
		for (auto a: attributes()) {
			if (a->name() == name) return a;
		}
		return nullptr;
	}
}
