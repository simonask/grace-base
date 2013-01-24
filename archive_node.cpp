#include "serialization/archive_node.hpp"
#include "serialization/archive.hpp"
#include "object/universe.hpp"
#include "object/objectptr.hpp"
#include "type/structured_type.hpp"

namespace falling {
class IUniverse;

ArchiveNode& ArchiveNode::array_push() {
	if (type() != Type::Array) {
		clear(Type::Array);
	}
	ArchiveNode* n = archive_.make();
	array_.push_back(n);
	return *n;
}

const ArchiveNode& ArchiveNode::operator[](size_t idx) const {
	ASSERT(type() == Type::Array);
	if (idx >= array_.size()) {
		return archive_.empty();
	}
	return *array_[idx];
}

ArchiveNode& ArchiveNode::operator[](size_t idx) {
	if (type() != Type::Array) {
		clear(Type::Array);
	}
	if (idx < array_.size()) {
		return *array_[idx];
	} else {
		array_.reserve(idx+1);
		while (array_.size() < idx+1) { array_.push_back(archive_.make()); }
		return *array_[idx];
	}
}

const ArchiveNode& ArchiveNode::operator[](StringRef key) const {
	if (type() != Type::Map) return archive_.empty();
	return *find_or(map_, key, &archive_.empty());
}

ArchiveNode& ArchiveNode::operator[](StringRef key) {
	if (type() != Type::Map) {
		clear(Type::Map);
	}
	auto it = map_.find(String(key));
	if (it == map_.end()) {
		ArchiveNode* n = archive_.make();
		map_[String(key)] = n;
		return *n;
	} else {
		return *it->second;
	}
}
	
	const ArchiveNodeConstPtrType* BuildTypeInfo<const ArchiveNode*>::build() {
		static const ArchiveNodeConstPtrType type = ArchiveNodeConstPtrType();
		return &type;
	}
	
	void ArchiveNodeConstPtrType::deserialize(ArchiveNodeConstPtrType::T &place, const falling::ArchiveNode & node, falling::IUniverse &) const {
		place = &node;
	}
	
	void ArchiveNodeConstPtrType::serialize(const ArchiveNodeConstPtrType::T &place, falling::ArchiveNode &, falling::IUniverse &) const {
		ASSERT(false); // Cannot serialize a reference into another serialized tree.
	}

	StringRef ArchiveNodeConstPtrType::name() const {
		return "const ArchiveNode*";
	}
}