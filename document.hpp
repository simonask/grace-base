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
	
	struct IDocument {
		virtual void write(OutputStream& os) const = 0;
		virtual size_t read(InputStream& is, String& out_error) = 0;
		virtual IAllocator& allocator() const = 0;
		virtual DocumentNode& root() = 0;
		virtual const DocumentNode& root() const = 0;
		virtual const DocumentNode& empty() const = 0;
		virtual DocumentNode* make() = 0;
		virtual void clear() = 0;
	};

	struct Document : IDocument {
		const DocumentNode& operator[](StringRef key) const;
		DocumentNode& operator[](StringRef key);
	
		IAllocator& allocator() const final { return allocator_; }
		DocumentNode& root() final { return root_; }
		const DocumentNode& root() const final { return root_; }
		const DocumentNode& empty() const final { return empty_; }
		DocumentNode* make() override;
		void clear() override;
		
		Document(IAllocator& alloc) : allocator_(alloc), root_(*this), empty_(*this) {}
		Document(Document&&) = delete;
	private:
		IAllocator& allocator_;
		DocumentNode root_;
		const DocumentNode empty_;
		ContainedBag<DocumentNode> nodes_;
	};
	
	struct MemoryDocument : Document {
	public:
		MemoryDocument(IAllocator& alloc) : Document(alloc) {}
		
		void write(OutputStream& os) const final { ASSERT(false); }
		size_t read(InputStream& is, String& out_error) final { ASSERT(false); }
	};

}

#endif /* end of include guard: ARCHIVE_HPP_A0L9H8RE */
