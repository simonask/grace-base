//
//  pointer_type.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "type/pointer_type.hpp"
#include "io/string_stream.hpp"

namespace grace {
	PointerType::PointerType(IAllocator& alloc, const Type* pointee_type, bool is_const) : name_(alloc), pointee_type_(pointee_type), is_const_(is_const) {
		StringStream ss;
		if (is_const) {
			ss << "const ";
		}
		ss << (pointee_type_ ? pointee_type_->name() : "Unknown") << '*';
		name_ = ss.string(alloc);
	}
	
	PointerType::PointerType(IAllocator& alloc, StringRef name, bool is_const) : name_(alloc), pointee_type_(nullptr), is_const_(is_const) {
		StringStream ss;
		if (is_const) {
			ss << "const ";
		}
		ss << name << '*';
		name_ = ss.string(alloc);
	}

	void PointerType::deserialize_raw(byte *place, const grace::ArchiveNode &, grace::IUniverse &) const {
		ASSERT(false); // Cannot deserialize pointer type.
	}
	
	void PointerType::serialize_raw(const byte *place, grace::ArchiveNode &, grace::IUniverse &) const {
		ASSERT(false); // Cannot serialize pointer type.
	}
	
	void PointerType::construct(byte *place, IUniverse &) const {
		memset(place, 0, size());
	}
	
	void PointerType::copy_construct(byte *to, const byte *from) const {
		memcpy(to, from, size());
	}
	
	void PointerType::move_construct(byte *to, byte *from) const {
		memcpy(to, from, size());
		memset(from, 0, size());
	}
	
	void PointerType::destruct(byte *place, IUniverse &) const {
		memset(place, 0, size());
	}
}
