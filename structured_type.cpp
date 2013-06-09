//
//  structured_type.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "type/structured_type.hpp"
#include "object/slot.hpp"
#include "type/attribute.hpp"

namespace grace {
	const ISlot* StructuredType::find_slot_by_name(StringRef name) const {
		const StructuredType* t = this;
		while (t != nullptr) {
			for (auto s: t->slots()) {
				if (s->name() == name) return s;
			}
			t = t->super();
		}
		return nullptr;
	}
	
	const IAttribute* StructuredType::find_attribute_by_name(StringRef name) const {
		const StructuredType* t = this;
		while (t != nullptr) {
			for (auto a: t->attributes()) {
				if (a->name() == name) return a;
			}
			t = t->super();
		}
		return nullptr;
	}
}
