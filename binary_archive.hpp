//
//  binary_archive.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 02/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_binary_archive_hpp
#define falling_binary_archive_hpp

#include "serialization/archive.hpp"
#include "serialization/archive_node.hpp"

#include "base/bag.hpp"

namespace falling {
	struct BinaryArchive;
	
	struct BinaryArchiveNode : ArchiveNode {
		BinaryArchiveNode(BinaryArchive&);
		
		void write(OutputStream& os) const;
		bool read(const byte*& ptr, const byte* end, String& out_error);
	};
	
	struct BinaryArchive : Archive {
		explicit BinaryArchive(IAllocator& alloc = default_allocator());
		~BinaryArchive() { clear(); }
		ArchiveNode& root() override { return root_; }
		const ArchiveNode& root() const override { return root_; }
		void write(OutputStream& os) const override;
		size_t read(InputStream& is, String& out_error) override;
		bool can_parse(const byte* begin, const byte* end) const;
		ArchiveNode* make() override { return make_internal(); }
		const ArchiveNode& empty() const override { return empty_; }
		
		void clear();
	private:
		BinaryArchiveNode* make_internal();
		BinaryArchiveNode root_;
		const BinaryArchiveNode empty_;
		ContainedBag<BinaryArchiveNode> nodes_;
	};
	
	inline BinaryArchiveNode::BinaryArchiveNode(BinaryArchive& archive) : ArchiveNode(archive) {}
}

#endif
