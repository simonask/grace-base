//
//  yaml_archive.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_yaml_archive_hpp
#define grace_yaml_archive_hpp

#include "serialization/archive.hpp"
#include "serialization/archive_node.hpp"

#include "base/bag.hpp"

namespace grace {
	struct YAMLArchive;
	
	struct YAMLArchiveNode : ArchiveNode {
		YAMLArchiveNode(YAMLArchive&);
	};
	
	struct YAMLArchive : public Archive {
		explicit YAMLArchive(IAllocator& alloc = default_allocator());
		~YAMLArchive() { clear(); }
		ArchiveNode& root() override;
		const ArchiveNode& root() const override { return root_ ? *root_ : empty_; }
		void write(OutputStream& os) const override;
		size_t read(InputStream& is, String& out_error) override;
		bool can_parse(const byte* begin, const byte* end) const;
		ArchiveNode* make() override { return make_internal(); }
		const ArchiveNode& empty() const override { return empty_; }
		
		void clear();
	private:
		YAMLArchiveNode* make_internal();
		YAMLArchiveNode* root_;
		const YAMLArchiveNode empty_;
		ContainedBag<YAMLArchiveNode> nodes_;
		friend struct YAMLParserState;
	};
	
	inline YAMLArchiveNode::YAMLArchiveNode(YAMLArchive& archive) : ArchiveNode(archive) {}
	
	inline ArchiveNode& YAMLArchive::root() {
		if (root_ == nullptr) {
			root_ = make_internal();
		}
		return *root_;
	}
}

#endif
