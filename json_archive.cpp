#include "serialization/json_archive.hpp"

namespace grace {

void JSONArchive::write(OutputStream& oss) const {
	FormattedStream os(oss);
	os << "{ \"root\": ";
	write_node(root(), os, false, 1);
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

void JSONArchive::write_node(const ArchiveNode& n, OutputStream& oss, bool print_inline, int indent) const {
	FormattedStream os(oss);
	n.when<NothingType>([&](NothingType) {
		os << "null";
	}).when<ArchiveNode::ArrayType>([&](const ArchiveNode::ArrayType& arr) {
		os << '[';
		if (print_inline) {
			for (size_t i = 0; i < n.array_size(); ++i) {
				write_node(*arr[i], os, true, indent);
				if (i+1 != arr.size()) {
					os << ", ";
				}
			}
		} else {
			for (size_t i = 0; i < n.array_size(); ++i) {
				os << '\n';
				print_indentation(os, indent+1);
				write_node(*arr[i], os, indent > 2, indent+1);
				if (i+1 != arr.size()) {
					os << ',';
				}
			}
			os << '\n';
			print_indentation(os, indent);
		}
		os << ']';
	}).when<ArchiveNode::MapType>([&](const ArchiveNode::MapType& map) {
		os << '{';
		if (print_inline) {
			for (auto it = map.begin(); it != map.end();) {
				print_string(os, it->first);
				os << ": ";
				write_node(*it->second, os, true, indent);
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
				write_node(*it->second, os, indent > 2, indent + 1);
				++it;
				if (it != map.end()) {
					os << ',';
				}
			}
			os << '\n';
			print_indentation(os, indent);
		}
		os << '}';
	}).when<ArchiveNode::IntegerType>([&](const ArchiveNode::IntegerType& n) {
		os << n;
	}).when<ArchiveNode::FloatType>([&](const ArchiveNode::FloatType& f) {
		os << f;
	}).when<ArchiveNode::StringType>([&](const ArchiveNode::StringType& str) {
		print_string(os, str);
	});
}
	
bool JSONArchive::read_node(ArchiveNode& n, const byte *&p, const byte *end, String &out_error) {
	out_error = "JSONArchive::read not implemented yet.";
	return false;
}
	
size_t JSONArchive::read(InputStream&, String& out_error) {
	out_error = "JSONArchive::read not implemented.";
	return 0;
}

}
