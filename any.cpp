//
//  any.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/any.hpp"
#include "serialization/document_node.hpp"
#include "type/array_type.hpp"
#include "type/map_type.hpp"

namespace grace {
	Any Any::take_memory(IAllocator& alloc, const IType* type, void *memory) {
		if (memory == nullptr) return Nothing;
		Any result(alloc);
		result.stored_type_ = type;
		size_t sz = type->size();
		if (sz > Size) {
			byte** ptrptr = reinterpret_cast<byte**>(&result.memory_);
			*ptrptr = (byte*)memory;
		} else {
			byte* ptr = reinterpret_cast<byte*>(&result.memory_);
			type->copy_construct(ptr, (const byte*)memory);
			type->destruct((byte*)memory, *(IUniverse*)nullptr);
			alloc.free(memory, sz);
		}
		return move(result);
	}
	}
}
