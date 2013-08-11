//
//  builtin_archive.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 25/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "io/builtin_archive.hpp"
#include "base/dictionary.hpp"
#include "io/memory_stream.hpp"

namespace grace {
	struct BuiltinArchive::Impl {
		Dictionary<ArrayRef<const byte>> assets;
		
		Impl(IAllocator& alloc) : assets(alloc) {}
	};
	
	BuiltinArchive::BuiltinArchive(const ArrayRef<const EmbeddedAsset* const> assets, IAllocator& alloc) {
		impl = new(alloc) Impl(alloc);
		impl->assets.reserve(assets.size());
		for (auto a: assets) {
			auto p = a->data;
			ArrayRef<const byte> ref(p, p + a->size);
			impl->assets[a->rid] = ref;
		}
	}
	
	BuiltinArchive::~BuiltinArchive() {
		destroy(impl, impl->assets.allocator());
	}
	
	bool BuiltinArchive::contains(ResourceID rid) const {
		return impl->assets.find(rid) != impl->assets.end();
	}
	
	UniquePtr<InputStream> BuiltinArchive::open(ResourceID rid, IAllocator& alloc) {
		auto it = impl->assets.find(rid);
		if (it == impl->assets.end())
			return nullptr;
		auto stream = make_unique<MemoryStream>(alloc, it->second.data(), it->second.data() + it->second.size());
		return move(stream);
	}
	
	String BuiltinArchive::debug_path(ResourceID rid) const {
		return concatenate("[builtin]/", rid);
	}
}
