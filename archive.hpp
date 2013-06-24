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
#include "serialization/archive_node.hpp"
#include "base/bag.hpp"

namespace grace {

	struct ArchiveNode;
	struct IUniverse;
	struct InputStream;
	struct OutputStream;
	
	struct IArchive {
		virtual void write(OutputStream& os) const = 0;
		virtual size_t read(InputStream& is, String& out_error) = 0;
		virtual IAllocator& allocator() const = 0;
		virtual ArchiveNode& root() = 0;
		virtual const ArchiveNode& root() const = 0;
		virtual const ArchiveNode& empty() const = 0;
		virtual ArchiveNode* make() = 0;
		virtual void clear() = 0;
	};

	struct Archive : IArchive {
		const ArchiveNode& operator[](StringRef key) const;
		ArchiveNode& operator[](StringRef key);
	
		IAllocator& allocator() const final { return allocator_; }
		ArchiveNode& root() final { return root_; }
		const ArchiveNode& root() const final { return root_; }
		const ArchiveNode& empty() const final { return empty_; }
		ArchiveNode* make() override;
		void clear() override;
	protected:
		Archive(IAllocator& alloc) : allocator_(alloc), root_(*this), empty_(*this) {}
		Archive(Archive&&) = delete;
	private:
		IAllocator& allocator_;
		ArchiveNode root_;
		const ArchiveNode empty_;
		ContainedBag<ArchiveNode> nodes_;
	};

}

#endif /* end of include guard: ARCHIVE_HPP_A0L9H8RE */
