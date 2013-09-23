//
//  archive.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 24/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "io/archive.hpp"
#include "io/util.hpp"
#include "io/file_stream.hpp"

namespace grace {
	bool PathArchive::contains(ResourceID rid) const {
		ScratchAllocator scratch;
		StringRef components[] = {path_, rid};
		auto path = path_join(components, scratch);
		return path_is_file(path);
	}
	
	UniquePtr<InputStream> PathArchive::open(ResourceID rid, IAllocator& alloc) {
		ScratchAllocator scratch;
		StringRef components[] = {path_, rid};
		auto path = path_join(components, scratch);
		FileStream fs = FileStream::open(path, FileMode::Read);
		if (fs.is_readable()) {
			return make_unique<FileStream>(alloc, move(fs));
		}
		return nullptr;
	}
	
	String PathArchive::debug_path(ResourceID rid) const {
		ScratchAllocator scratch;
		StringRef components[] = {path_, rid};
		return path_join(components);
	}
}
