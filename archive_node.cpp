#include "serialization/archive_node.hpp"
#include "serialization/archive.hpp"
#include "object/universe.hpp"
#include "object/objectptr.hpp"
#include "type/structured_type.hpp"
#include "io/formatters.hpp"
#include "io/stdio_stream.hpp"

namespace falling {
class IUniverse;

bool ArchiveNode::is_integer() const {
	return value_.is_a<IntegerType>();
}

bool ArchiveNode::is_float() const {
	return value_.is_a<FloatType>();
}

bool ArchiveNode::is_array() const {
	return value_.is_a<ArrayType>();
}

bool ArchiveNode::is_map() const {
	return value_.is_a<MapType>();
}

bool ArchiveNode::is_string() const {
	return value_.is_a<StringType>();
}

bool ArchiveNode::is_scalar() const {
	return !is_array() && !is_map();
}

bool ArchiveNode::is_empty() const {
	return value_.is_nothing();
}

IAllocator& ArchiveNode::allocator() const {
	return archive().allocator();
}

ArchiveNode& ArchiveNode::array_push() {
	if (!is_array()) {
		value_ = ArrayType(allocator());
	}
	ArchiveNode* n = archive_.make();
	value_.when<ArrayType>([&](ArrayType& array) {
		array.push_back(n);
	});
	return *n;
}

const ArchiveNode& ArchiveNode::operator[](size_t idx) const {
	if (!is_array()) return archive_.empty();
	const ArchiveNode* result = &archive_.empty();
	value_.when<ArrayType>([&](const ArrayType& array) {
		if (idx < array.size()) {
			result = array[idx];
		}
	});
	return *result;
}

ArchiveNode& ArchiveNode::operator[](size_t idx) {
	if (!is_array()) {
		value_ = ArrayType(allocator());
	}
	
	ArchiveNode* result;
	value_.when<ArrayType>([&](ArrayType& array) {
		if (idx < array.size()) {
			result = array[idx];
		} else {
			array.reserve(idx+1);
			while (array.size() < idx+1) { array.push_back(make_child()); }
			result = array[idx];
		}
	});
	return *result;
}

const ArchiveNode& ArchiveNode::operator[](StringRef key) const {
	const ArchiveNode* result;
	value_.when<MapType>([&](const MapType& map) {
		result = find_or(map, key, &archive_.empty());
	}).otherwise([&]() {
		result = &archive_.empty();
	});
	return *result;
}

ArchiveNode& ArchiveNode::operator[](StringRef key) {
	if (!is_map()) {
		value_ = MapType(allocator());
	}
	ArchiveNode* result;
	value_.when<MapType>([&](MapType& map) {
		auto it = map.find(key);
		if (it == map.end()) {
			result = make_child();
			map[String(key, allocator())] = result;
		} else {
			result = it->second;
		}
	});
	return *result;
}

ArchiveNode* ArchiveNode::make_child() {
	return archive_.make();
}

size_t ArchiveNode::array_size() const {
	size_t sz = 0;
	value_.when<ArrayType>([&](const ArrayType& array) {
		sz = array.size();
	});
	return sz;
}
	
	void ArchiveNode::dump(FormattedStream &os) const {
		dump(os, 0);
	}
	
	void ArchiveNode::dump(FormattedStream& os, int indent) const {
		value_.when<NothingType>([&](NothingType) {
			os << "(none)";
		}).when<StringType>([&](StringRef str) {
				os << "\"" << str << "\"";
		}).when<IntegerType>([&](IntegerType n) {
				os << n;
		}).when<FloatType>([&](FloatType f) {
				os << f;
		}).when<ArrayType>([&](const ArrayType& array) {
			bool all_are_scalars = true;
			for (auto child: array) {
				if (!child->is_scalar()) {
					all_are_scalars = false;
					break;
				}
			}
			
			if (all_are_scalars) {
				// dump inline array
				os << '[';
				for (auto child: array) {
					child->dump(os, -1);
					if (child != array.back()) {
						os << ", ";
					}
				}
				os << ']';
			} else {
				// dump multiline array
				os << "[\n" << repeat(' ', indent);
				for (auto child: array) {
					child->dump(os, indent+2);
					os << '\n' << repeat(' ', indent);
				}
				os << ']';
			}
		}).when<MapType>([&](const MapType& map) {
			bool all_are_scalars = true;
			for (auto pair: map) {
				if (pair.first.size() > 10 || !pair.second->is_scalar()) {
					all_are_scalars = false;
					break;
				}
			}
			
			auto keys = map.keys();
			auto values = map.values();
			if (all_are_scalars) {
				// dump inline map
				os << '{';
				for (size_t i = 0; i < keys.size(); ++i) {
					os << keys[i] << ": ";
					values[i]->dump(os, -1);
					if (i+1 != keys.size()) {
						os << ", ";
					}
				}
				os << '}';
			} else {
				os << "{\n" << repeat(' ', indent);
				for (size_t i = 0; i < keys.size(); ++i) {
					os << repeat(' ', 2);
					os << keys[i] << ": ";
					values[i]->dump(os, indent+4);
					if (i+1 != keys.size()) {
						os << ",";
					}
					os << "\n" << repeat(' ', indent);
				}
				os << "}";
			}
		});
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
	
	const ArchiveNodePtrType* BuildTypeInfo<ArchiveNode*>::build() {
		static const ArchiveNodePtrType type = ArchiveNodePtrType();
		return &type;
	}
	
	void ArchiveNodePtrType::deserialize(ArchiveNodePtrType::T &place, const falling::ArchiveNode & node, falling::IUniverse &) const {
		ASSERT(false); // Cannot deserialize a non-const pointer to an ArchiveNode.
	}
	
	void ArchiveNodePtrType::serialize(const ArchiveNodePtrType::T &place, falling::ArchiveNode &, falling::IUniverse &) const {
		ASSERT(false); // Cannot serialize a reference into another serialized tree.
	}
	
	StringRef ArchiveNodePtrType::name() const {
		return "ArchiveNode*";
	}
	
	void dump_archive_node_to_stdout(const ArchiveNode& node) {
		node.dump(StandardOutput);
	}
	
	void dump_archive_to_stdout(const Archive& archive) {
		dump_archive_node_to_stdout(archive.root());
	}
}