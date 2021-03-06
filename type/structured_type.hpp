//
//  structured_type.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_structured_type_hpp
#define grace_structured_type_hpp

#include "type/type.hpp"

namespace grace {
	struct ISlot;
	struct IAttribute;
	struct ReferenceType;
	
	struct StructuredType : DerivedType {
		StructuredType(const TypeInfo& ti) : DerivedType(ti) {}
		virtual ArrayRef<const ISlot*> slots() const = 0;
		virtual ArrayRef<const IAttribute*> attributes() const = 0;
		virtual const StructuredType* super() const = 0;
		virtual const ReferenceType* reference_type() const;
		virtual bool wants_game_update() const = 0;
		
		const ISlot* find_slot_by_name(StringRef name) const;
		const IAttribute* find_attribute_by_name(StringRef name) const;
	};
}

#endif
