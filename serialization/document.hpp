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
	struct IInputStream;
	struct IOutputStream;

	struct Document {
		const DocumentNode& operator[](StringRef key) const;
		DocumentNode& operator[](StringRef key);
		operator const DocumentNode&() const { return root(); }
		operator DocumentNode&() { return root(); }
	
		IAllocator& allocator() const { return allocator_; }
		DocumentNode& root() { return root_; }
		const DocumentNode& root() const { return root_; }
		const DocumentNode& empty() const { return empty_; }
		DocumentNode* make();
		void clear();
		
		explicit Document(IAllocator& alloc = default_allocator()) : allocator_(alloc), root_(*this), empty_(*this) {}
		Document(Document&&) = delete; // deleted because the tree may contain pointers to root_ and empty_.
	private:
		IAllocator& allocator_;
		DocumentNode root_;
		const DocumentNode empty_;
		ContainedBag<DocumentNode> nodes_;
	};
	
	struct IDocumentReader {
		virtual size_t read(Document& out_doc, IInputStream& is, String& out_error) = 0;
	};
	
	struct IDocumentWriter {
		virtual void write(IOutputStream& os, const Document& doc) = 0;
	};
}

#endif /* end of include guard: ARCHIVE_HPP_A0L9H8RE */
