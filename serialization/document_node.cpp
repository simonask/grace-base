#include "serialization/document_node.hpp"
#include "serialization/document.hpp"
#include "object/universe.hpp"
#include "object/objectptr.hpp"
#include "type/structured_type.hpp"
#include "io/formatters.hpp"
#include "io/stdio_stream.hpp"

namespace grace {
struct IUniverse;

bool DocumentNode::is_integer() const {
	return value_.is_a<IntegerType>();
}

bool DocumentNode::is_float() const {
	return value_.is_a<FloatType>();
}

bool DocumentNode::is_array() const {
	return value_.is_a<ArrayType>();
}

bool DocumentNode::is_map() const {
	return value_.is_a<MapType>();
}

bool DocumentNode::is_string() const {
	return value_.is_a<StringType>();
}

bool DocumentNode::is_scalar() const {
	return !is_array() && !is_map();
}

bool DocumentNode::is_empty() const {
	return value_.is_nothing();
}

IAllocator& DocumentNode::allocator() const {
	return document().allocator();
}

DocumentNode& DocumentNode::array_push() {
	if (!is_array()) {
		value_ = ArrayType(allocator());
	}
	DocumentNode* n = document_.make();
	value_.when<ArrayType>([&](ArrayType& array) {
		array.push_back(n);
	});
	return *n;
}

const DocumentNode& DocumentNode::operator[](size_t idx) const {
	if (!is_array()) return document_.empty();
	const DocumentNode* result = &document_.empty();
	value_.when<ArrayType>([&](const ArrayType& array) {
		if (idx < array.size()) {
			result = array[idx];
		}
	});
	return *result;
}

DocumentNode& DocumentNode::operator[](size_t idx) {
	if (!is_array()) {
		value_ = ArrayType(allocator());
	}
	
	DocumentNode* result;
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

const DocumentNode& DocumentNode::operator[](StringRef key) const {
	const DocumentNode* result;
	value_.when<MapType>([&](const MapType& map) {
		result = find_or(map, key, &document_.empty());
	}).otherwise([&]() {
		result = &document_.empty();
	});
	return *result;
}

DocumentNode& DocumentNode::operator[](StringRef key) {
	if (!is_map()) {
		value_ = MapType(allocator());
	}
	DocumentNode* result;
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

DocumentNode* DocumentNode::make_child() {
	return document_.make();
}

size_t DocumentNode::array_size() const {
	size_t sz = 0;
	value_.when<ArrayType>([&](const ArrayType& array) {
		sz = array.size();
	});
	return sz;
}
	
	void DocumentNode::dump(FormattedStream &os) const {
		dump(os, 0);
	}
	
	void DocumentNode::dump(FormattedStream& os, int indent) const {
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
	
	const DocumentNodeConstPtrType* BuildTypeInfo<const DocumentNode*>::build() {
		static const DocumentNodeConstPtrType type = DocumentNodeConstPtrType();
		return &type;
	}
	
	void DocumentNodeConstPtrType::deserialize(DocumentNodeConstPtrType::T &place, const grace::DocumentNode & node, grace::IUniverse &) const {
		place = &node;
	}
	
	void DocumentNodeConstPtrType::serialize(const DocumentNodeConstPtrType::T &place, grace::DocumentNode &, grace::IUniverse &) const {
		ASSERT(false); // Cannot serialize a reference into another serialized tree.
	}

	StringRef DocumentNodeConstPtrType::name() const {
		return "const DocumentNode*";
	}
	
	const DocumentNodePtrType* BuildTypeInfo<DocumentNode*>::build() {
		static const DocumentNodePtrType type = DocumentNodePtrType();
		return &type;
	}
	
	void DocumentNodePtrType::deserialize(DocumentNodePtrType::T &place, const grace::DocumentNode & node, grace::IUniverse &) const {
		ASSERT(false); // Cannot deserialize a non-const pointer to an DocumentNode.
	}
	
	void DocumentNodePtrType::serialize(const DocumentNodePtrType::T &place, grace::DocumentNode &, grace::IUniverse &) const {
		ASSERT(false); // Cannot serialize a reference into another serialized tree.
	}
	
	StringRef DocumentNodePtrType::name() const {
		return "DocumentNode*";
	}
	
	void dump_document_node_to_stdout(const DocumentNode& node) {
		node.dump(StdOut);
	}
	
	void dump_document_to_stdout(const Document& document) {
		dump_document_node_to_stdout(document.root());
	}
}