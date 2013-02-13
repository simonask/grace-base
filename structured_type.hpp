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

namespace falling {
	struct ISlot;
	struct IAttribute;
	
	struct StructuredType : DerivedType {
		virtual ArrayRef<const ISlot* const> slots() const = 0;
		virtual ArrayRef<const IAttribute*> attributes() const = 0;
		virtual const StructuredType* super() const = 0;
		
		const ISlot* find_slot_by_name(StringRef name) const;
		const IAttribute* find_attribute_by_name(StringRef name) const;
	};
}

#endif
