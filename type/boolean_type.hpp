//
//  boolean_type.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 12/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_boolean_type_hpp
#define grace_boolean_type_hpp

#include "type/type.hpp"
#include "memory/static_allocator.hpp"

namespace grace {
	struct BooleanType : TypeFor<bool> {
	public:
		void deserialize(bool& place, const DocumentNode&, IUniverse&) const;
		void serialize(const bool& place, DocumentNode&, IUniverse&) const;
		StringRef name() const { return "bool"; }
	};
	
	template <>
	struct BuildTypeInfo<bool> {
		static BooleanType* build();
	};
}

#endif
