#include "serialization/json_archive.hpp"

namespace falling {

JSONArchive::JSONArchive(IAllocator& alloc) : Archive(alloc), root_(nullptr), nodes_(alloc) {
	empty_ = make_internal();
}

JSONArchiveNode* JSONArchive::make_internal(ArchiveNode::Type node_type) {
	return nodes_.allocate(*this, node_type);
}

ArchiveNode& JSONArchive::root() {
	if (root_ == nullptr) {
		root_ = make_internal(ArchiveNodeType::Map);
	}
	return *root_;
}

const ArchiveNode& JSONArchive::root() const {
	ASSERT(root_ != nullptr);
	return *root_;
}

void JSONArchive::write(OutputStream& oss) const {
	FormattedStream os(oss);
	os << "{ \"root\": ";
	if (root_ != nullptr)
		root_->write(os, false, 1);
	os << "\n}\n";
}

static void print_indentation(OutputStream& oss, int level) {
	FormattedStream os(oss);
	for (int i = 0; i < level; ++i) {
		os << "  ";
	}
}

static void print_string(OutputStream& oss, const String& str) {
	FormattedStream os(oss);
	// TODO: Escape
	os << '"';
	os << str;
	os << '"';
}

void JSONArchiveNode::write(OutputStream& oss, bool print_inline, int indent) const {
	FormattedStream os(oss);
	switch (type()) {
		case ArchiveNodeType::Empty: os << "null"; break;
		case ArchiveNodeType::Array: {
			os << '[';
			if (print_inline) {
				for (size_t i = 0; i < array_.size(); ++i) {
					dynamic_cast<const JSONArchiveNode*>(array_[i])->write(os, true, indent);
					if (i+1 != array_.size()) {
						os << ", ";
					}
				}
			} else {
				for (size_t i = 0; i < array_.size(); ++i) {
					os << '\n';
					print_indentation(os, indent+1);
					dynamic_cast<const JSONArchiveNode*>(array_[i])->write(os, indent > 2, indent+1);
					if (i+1 != array_.size()) {
						os << ',';
					}
				}
				os << '\n';
				print_indentation(os, indent);
			}
			os << ']';
			break;
		}
		case ArchiveNodeType::Map: {
			os << '{';
			if (print_inline) {
				for (auto it = map_.begin(); it != map_.end();) {
					print_string(os, it->first);
					os << ": ";
					dynamic_cast<const JSONArchiveNode*>(it->second)->write(os, true, indent);
					++it;
					if (it != map_.end()) {
						os << ", ";
					}
				}
			} else {
				for (auto it = map_.begin(); it != map_.end();) {
					os << '\n';
					print_indentation(os, indent+1);
					print_string(os, it->first);
					os << ": ";
					dynamic_cast<const JSONArchiveNode*>(it->second)->write(os, indent > 2, indent+1);
					++it;
					if (it != map_.end()) {
						os << ',';
					}
				}
				os << '\n';
				print_indentation(os, indent);
			}
			os << '}';
			break;
		}
		case ArchiveNodeType::Integer: os << integer_value; break;
		case ArchiveNodeType::Float: os << float_value; break;
		case ArchiveNodeType::String: print_string(os, string_value); break;
	}
}
	
bool JSONArchiveNode::read(const byte *&p, const byte *end, String &out_error) {
	out_error = "JSONArchive::read not implemented yet.";
	return false;
}
	
size_t JSONArchive::read(InputStream&, String& out_error) {
	out_error = "JSONArchive::read not implemented.";
	return 0;
}

}
