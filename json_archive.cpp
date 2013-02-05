#include "serialization/json_archive.hpp"

namespace falling {

JSONArchive::JSONArchive(IAllocator& alloc) : Archive(alloc), root_(nullptr), nodes_(alloc) {
	empty_ = make_internal();
}

JSONArchiveNode* JSONArchive::make_internal() {
	return nodes_.allocate(*this);
}

ArchiveNode& JSONArchive::root() {
	if (root_ == nullptr) {
		root_ = make_internal();
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

static void print_string(OutputStream& oss, StringRef str) {
	FormattedStream os(oss);
	// TODO: Escape
	os << '"';
	os << str;
	os << '"';
}

void JSONArchiveNode::write(OutputStream& oss, bool print_inline, int indent) const {
	FormattedStream os(oss);
	if (is_empty()) { os << "null"; }
	else if (is_array()) {
		value_.when<ArrayType>([&](const ArrayType& arr) {
			os << '[';
			if (print_inline) {
				for (size_t i = 0; i < array_size(); ++i) {
					dynamic_cast<const JSONArchiveNode*>(arr[i])->write(os, true, indent);
					if (i+1 != arr.size()) {
						os << ", ";
					}
				}
			} else {
				for (size_t i = 0; i < array_size(); ++i) {
					os << '\n';
					print_indentation(os, indent+1);
					dynamic_cast<const JSONArchiveNode*>(arr[i])->write(os, indent > 2, indent+1);
					if (i+1 != arr.size()) {
						os << ',';
					}
				}
				os << '\n';
				print_indentation(os, indent);
			}
			os << ']';
		});
	} else if (is_map()) {
		value_.when<MapType>([&](const MapType& map) {
			os << '{';
			if (print_inline) {
				for (auto it = map.begin(); it != map.end();) {
					print_string(os, it->first);
					os << ": ";
					dynamic_cast<const JSONArchiveNode*>(it->second)->write(os, true, indent);
					++it;
					if (it != map.end()) {
						os << ", ";
					}
				}
			} else {
				for (auto it = map.begin(); it != map.end();) {
					os << '\n';
					print_indentation(os, indent+1);
					print_string(os, it->first);
					os << ": ";
					dynamic_cast<const JSONArchiveNode*>(it->second)->write(os, indent > 2, indent+1);
					++it;
					if (it != map.end()) {
						os << ',';
					}
				}
				os << '\n';
				print_indentation(os, indent);
			}
			os << '}';
		});
	} else if (is_integer()) {
		IntegerType n;
		(*this) >> n;
		os << n;
	} else if (is_float()) {
		FloatType f;
		(*this) >> f;
		os << f;
	} else if (is_string()) {
		StringRef str;
		(*this) >> str;
		print_string(os, str);
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
