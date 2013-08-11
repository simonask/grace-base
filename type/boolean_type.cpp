//
//  boolean_type.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 12/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "type/boolean_type.hpp"
#include "serialization/document_node.hpp"

namespace grace {
	BooleanType* BuildTypeInfo<bool>::build() {
		static BooleanType* p = new_static BooleanType;
		return p;
	}
	
	void BooleanType::deserialize(bool &place, const DocumentNode& node, IUniverse &) const {
		node >> place;
	}
	
	void BooleanType::serialize(const bool &place, DocumentNode& node, IUniverse &) const {
		node << place;
	}
}
