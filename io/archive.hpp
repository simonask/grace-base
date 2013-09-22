//
//  archive.h
//  grace
//
//  Created by Simon Ask Ulsnes on 24/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef __grace__archive__
#define __grace__archive__

#include "base/basic.hpp"
#include "io/resource.hpp"
#include "memory/allocator.hpp"
#include "memory/unique_ptr.hpp"

namespace grace {
	class InputStream;
	
	struct IArchive {
		virtual bool contains(ResourceID) const = 0;
		virtual UniquePtr<InputStream> open(ResourceID, IAllocator& alloc = default_allocator()) = 0;
		virtual String debug_path(ResourceID) const = 0;
	};
	
	struct PathArchive : IArchive {
		PathArchive(StringRef path) : path_(path) {}
		
		bool contains(ResourceID) const final;
		UniquePtr<InputStream> open(ResourceID, IAllocator& alloc = default_allocator()) final;
		String debug_path(ResourceID) const;
	private:
		StringRef path_;
	};
}

#endif /* defined(__grace__archive__) */
