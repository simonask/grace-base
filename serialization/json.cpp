#include "serialization/json.hpp"

namespace grace {

void JSON::write(IOutputStream& oss, const Document& doc) {
	FormattedStream os(oss);
	os << "{ \"root\": ";
	write_node(doc, os, false, 1);
	os << "\n}\n";
}

static void print_indentation(IOutputStream& oss, int level) {
	FormattedStream os(oss);
	for (int i = 0; i < level; ++i) {
		os << "  ";
	}
}

static void print_string(IOutputStream& oss, StringRef str) {
	FormattedStream os(oss);
	// TODO: Escape
	os << '"';
	os << str;
	os << '"';
}

void JSON::write_node(const DocumentNode& n, IOutputStream& oss, bool print_inline, int indent) const {
	FormattedStream os(oss);
	n.when<NothingType>([&](NothingType) {
		os << "null";
	}).when<DocumentNode::ArrayType>([&](const DocumentNode::ArrayType& arr) {
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
	}).when<DocumentNode::MapType>([&](const DocumentNode::MapType& map) {
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
	}).when<DocumentNode::IntegerType>([&](const DocumentNode::IntegerType& n) {
		os << n;
	}).when<DocumentNode::FloatType>([&](const DocumentNode::FloatType& f) {
		os << f;
	}).when<DocumentNode::StringType>([&](const DocumentNode::StringType& str) {
		print_string(os, str);
	});
}
	
bool JSON::read_node(DocumentNode& n, const byte *&p, const byte *end, String &out_error) {
	out_error = "JSONDocument::read not implemented yet.";
	return false;
}
	
size_t JSON::read(Document& doc, IInputStream&, String& out_error) {
	out_error = "JSONDocument::read not implemented.";
	return 0;
}

}
