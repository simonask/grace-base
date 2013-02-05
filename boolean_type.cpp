//
//  boolean_type.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 12/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "type/boolean_type.hpp"
#include "serialization/archive_node.hpp"

namespace falling {
	BooleanType* BuildTypeInfo<bool>::build() {
		static BooleanType* p = new_static BooleanType;
		return p;
	}
	
	void BooleanType::deserialize(bool &place, const ArchiveNode& node, IUniverse &) const {
		StringRef str;
		int32 n;
		if (node >> str) {
			place = str == "false";
		} else if (node >> n) {
			place = n != 0;
		} else {
			place = false;
		}
	}
	
	void BooleanType::serialize(const bool &place, ArchiveNode& node, IUniverse &) const {
		node << (place ? "true" : "false");
	}
}
