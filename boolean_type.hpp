//
//  boolean_type.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 12/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_boolean_type_hpp
#define falling_boolean_type_hpp

#include "type/type.hpp"
#include "memory/static_allocator.hpp"

namespace falling {
	struct BooleanType : TypeFor<bool> {
	public:
		void deserialize(bool& place, const ArchiveNode&, IUniverse&) const;
		void serialize(const bool& place, ArchiveNode&, IUniverse&) const;
		StringRef name() const { return "bool"; }
	};
	
	template <>
	struct BuildTypeInfo<bool> {
		static BooleanType* build();
	};
}

#endif
