//
//  binary_archive.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_binary_archive_hpp
#define grace_binary_archive_hpp

#include "serialization/archive.hpp"
#include "serialization/archive_node.hpp"

#include "base/bag.hpp"

namespace grace {
	struct BinaryArchive : Archive {
		explicit BinaryArchive(IAllocator& alloc = default_allocator()) : Archive(alloc) {}
		BinaryArchive(BinaryArchive&& other) = delete;
		void write(OutputStream& os) const override;
		size_t read(InputStream& is, String& out_error) override;
		bool can_parse(const byte* begin, const byte* end) const;
	private:
		void write_node(const ArchiveNode&, OutputStream& os) const;
		bool read_node(ArchiveNode&, const byte*& ptr, const byte* end, String& out_error);
	};
}

#endif
