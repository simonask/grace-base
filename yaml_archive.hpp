//
//  yaml_archive.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_yaml_archive_hpp
#define falling_yaml_archive_hpp

#include "serialization/archive.hpp"
#include "serialization/archive_node.hpp"

#include "base/bag.hpp"

namespace falling {
	struct YAMLArchive;
	
	struct YAMLArchiveNode : ArchiveNode {
		YAMLArchiveNode(YAMLArchive&, ArchiveNodeType::Type type);
	};
	
	struct YAMLArchive : public Archive {
		explicit YAMLArchive(IAllocator& alloc = default_allocator());
		~YAMLArchive() { clear(); }
		ArchiveNode& root() override;
		const ArchiveNode& root() const override { return root_ ? *root_ : empty_; }
		void write(OutputStream& os) const override;
		size_t read(InputStream& is, std::string& out_error) override;
		bool can_parse(const byte* begin, const byte* end) const;
		ArchiveNode* make(NodeType type = NodeType::Empty) override { return make_internal(type); }
		const ArchiveNode& empty() const override { return empty_; }
		
		void clear();
	private:
		YAMLArchiveNode* make_internal(NodeType type = NodeType::Empty);
		YAMLArchiveNode* root_;
		const YAMLArchiveNode empty_;
		ContainedBag<YAMLArchiveNode> nodes_;
		friend struct YAMLParserState;
	};
	
	inline YAMLArchiveNode::YAMLArchiveNode(YAMLArchive& archive, ArchiveNodeType::Type type) : ArchiveNode(archive, type) {}
	
	inline ArchiveNode& YAMLArchive::root() {
		if (root_ == nullptr) {
			root_ = make_internal(ArchiveNodeType::Map);
		}
		return *root_;
	}
}

#endif
