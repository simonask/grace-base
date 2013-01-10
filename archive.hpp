#pragma once
#ifndef ARCHIVE_HPP_A0L9H8RE
#define ARCHIVE_HPP_A0L9H8RE

#include "base/basic.hpp"
#include "object/object.hpp"
#include "type/type.hpp"
#include "object/objectptr.hpp"
#include "serialization/archive_node_type.hpp"
#include "base/stack_array.hpp"

#include "base/string.hpp"
#include "io/input_stream.hpp"
#include "io/file_stream.hpp"

namespace falling {

struct ArchiveNode;
struct IUniverse;

struct Archive {
	typedef ArchiveNodeType::Type NodeType;
	virtual ~Archive() {}
	virtual ArchiveNode& root() = 0;
	virtual const ArchiveNode& root() const = 0;
	virtual void write(OutputStream& os) const = 0;
	virtual size_t read(InputStream& is, String& out_error) = 0;
	virtual ArchiveNode* make(NodeType type = NodeType::Empty) = 0;
	virtual const ArchiveNode& empty() const = 0;
	
	const ArchiveNode& operator[](StringRef key) const;
	ArchiveNode& operator[](StringRef key);
	
	IAllocator& allocator() const { return allocator_; }
protected:
	Archive(IAllocator& alloc) : allocator_(alloc) {}
private:
	IAllocator& allocator_;
};

}

#endif /* end of include guard: ARCHIVE_HPP_A0L9H8RE */
