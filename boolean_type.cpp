//
//  boolean_type.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 12/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "type/boolean_type.hpp"

namespace falling {
	BooleanType* BuildTypeInfo<bool>::build() {
		static BooleanType* p = new_static BooleanType;
		return p;
	}
	
	void BooleanType::deserialize(bool &place, const ArchiveNode& node, UniverseBase &) const {
		bool is_false = node.is_empty() || (node.is_scalar() && (node.string_value == "false" || node.string_value == "0"));
		place = !is_false;
	}
	
	void BooleanType::serialize(const bool &place, ArchiveNode& node, UniverseBase &) const {
		node = place ? "true" : "false";
	}
}
