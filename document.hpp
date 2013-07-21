#pragma once
#ifndef ARCHIVE_HPP_A0L9H8RE
#define ARCHIVE_HPP_A0L9H8RE

#include "base/basic.hpp"
#include "object/object.hpp"
#include "type/type.hpp"
#include "object/objectptr.hpp"
#include "base/stack_array.hpp"

#include "base/string.hpp"
#include "io/input_stream.hpp"
#include "io/file_stream.hpp"
#include "serialization/document_node.hpp"
#include "base/bag.hpp"

namespace grace {

	struct DocumentNode;
	struct IUniverse;
	struct InputStream;
	struct OutputStream;

	struct Document {
		const DocumentNode& operator[](StringRef key) const;
		DocumentNode& operator[](StringRef key);
	
		IAllocator& allocator() const { return allocator_; }
		DocumentNode& root() { return root_; }
		const DocumentNode& root() const { return root_; }
		const DocumentNode& empty() const { return empty_; }
		DocumentNode* make();
		void clear();
		
		Document(IAllocator& alloc) : allocator_(alloc), root_(*this), empty_(*this) {}
		Document(Document&&) = delete;
	private:
		IAllocator& allocator_;
		DocumentNode root_;
		const DocumentNode empty_;
		ContainedBag<DocumentNode> nodes_;
	};
}

#endif /* end of include guard: ARCHIVE_HPP_A0L9H8RE */
