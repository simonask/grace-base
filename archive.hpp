#pragma once
#ifndef ARCHIVE_HPP_A0L9H8RE
#define ARCHIVE_HPP_A0L9H8RE

#include "base/basic.hpp"
#include "object/object.hpp"
#include "type/type.hpp"
#include "object/objectptr.hpp"
#include "serialization/archive_node_type.hpp"
#include "base/stack_array.hpp"

#include <string>
#include "io/input_stream.hpp"
#include "io/file_stream.hpp"

namespace falling {

struct DeserializeReferenceBase;
struct SerializeReferenceBase;
struct DeserializeSignalBase;
struct ArchiveNode;
struct IUniverse;

struct Archive {
	typedef ArchiveNodeType::Type NodeType;
	virtual ~Archive() {}
	virtual ArchiveNode& root() = 0;
	virtual const ArchiveNode& root() const = 0;
	virtual void write(OutputStream& os) const = 0;
	virtual size_t read(const byte* begin, const byte* end, std::string& out_error) = 0;
	size_t read_stream(InputFileStream& input, std::string& out_error);
	virtual ArchiveNode* make(NodeType type = NodeType::Empty) = 0;
	virtual const ArchiveNode& empty() const = 0;
	
	const ArchiveNode& operator[](const std::string& key) const;
	ArchiveNode& operator[](const std::string& key);
	
	void serialize(ObjectPtr<> object, IUniverse& universe);
	bool deserialize(IUniverse& universe, std::string& out_error);
	
	void register_reference_for_deserialization(DeserializeReferenceBase* ref) { deserialize_references.push_back(ref); }
	void register_reference_for_serialization(SerializeReferenceBase* ref) { serialize_references.push_back(ref); }
	void register_signal_for_deserialization(DeserializeSignalBase* sig) {
		deserialize_signals.push_back(sig);
	}
private:
	Array<DeserializeReferenceBase*> deserialize_references;
	Array<SerializeReferenceBase*> serialize_references;
	Array<DeserializeSignalBase*> deserialize_signals;
};
	
	// TODO: Rewrite serializers with streams as primary read API.
	inline size_t Archive::read_stream(InputFileStream &input, std::string &out_error) {
		size_t length = input.file_size();
		DEFINE_STACK_ARRAY(byte, buffer, length);
		input.read(buffer.begin(), length);
		return this->read(buffer.begin(), buffer.end(), out_error);
	}

}

#endif /* end of include guard: ARCHIVE_HPP_A0L9H8RE */
