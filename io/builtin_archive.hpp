//
//  builtin_archive.h
//  grace
//
//  Created by Simon Ask Ulsnes on 25/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef __grace__builtin_archive__
#define __grace__builtin_archive__

#include "io/archive.hpp"
#include "io/embedded_asset.hpp"

namespace grace {
	struct BuiltinArchive : IArchive {
		BuiltinArchive(const ArrayRef<const EmbeddedAsset* const> assets, IAllocator& alloc = default_allocator());
		~BuiltinArchive();
		
		bool contains(ResourceID) const final;
		UniquePtr<InputStream> open(ResourceID, IAllocator& alloc = default_allocator()) final;
		String debug_path(ResourceID) const;
		
		struct Impl;
		Impl* impl = nullptr;
	};
}

#endif /* defined(__grace__builtin_archive__) */
