//
//  structured_type.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_structured_type_hpp
#define falling_structured_type_hpp

#include "type/type.hpp"
#include "object/slot.hpp"
#include "type/attribute.hpp"

namespace falling {
	struct StructuredType : DerivedType {
		virtual ArrayRef<const SlotBase* const> slots() const = 0;
		virtual ArrayRef<const IAttribute*> attributes() const = 0;
		
		const SlotBase* find_slot_by_name(StringRef name) const;
		const IAttribute* find_attribute_by_name(StringRef name) const;
	};
	
	inline const SlotBase* StructuredType::find_slot_by_name(StringRef name) const {
		for (auto s: slots()) {
			if (s->name() == name) return s;
		}
		return nullptr;
	}
	
	inline const IAttribute* StructuredType::find_attribute_by_name(StringRef name) const {
		for (auto a: attributes()) {
			if (a->name() == name) return a;
		}
		return nullptr;
	}
}

#endif
