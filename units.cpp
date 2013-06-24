//
//  units.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 24/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "base/units.hpp"

namespace grace {
	DegreesType::DegreesType(IAllocator& alloc) : FloatType(alloc, GetTypeInfo<Degrees>::Value, "Degrees", sizeof(Degrees)) {}
	
	void DegreesType::deserialize_raw(byte *place, const DocumentNode &node, IUniverse& u) const {
		float32 fval;
		FloatType::deserialize_raw(reinterpret_cast<byte*>(&fval), node, u);
		Degrees* d = reinterpret_cast<Degrees*>(place);
		*d = Degrees(fval);
	}
	
	const DegreesType* BuildTypeInfo<Degrees>::build() {
		static const DegreesType* p = new_static DegreesType(static_allocator());
		return p;
	}
	
	RadiansType::RadiansType(IAllocator& alloc) : FloatType(alloc, GetTypeInfo<Radians>::Value, "Radians", sizeof(Radians)) {}
	
	void RadiansType::deserialize_raw(byte *place, const DocumentNode &node, IUniverse& u) const {
		float32 fval;
		FloatType::deserialize_raw(reinterpret_cast<byte*>(&fval), node, u);
		Radians* d = reinterpret_cast<Radians*>(place);
		*d = Radians(fval);
	}
	
	const RadiansType* BuildTypeInfo<Radians>::build() {
		static const RadiansType* p = new_static RadiansType(static_allocator());
		return p;
	}
}
