#include "type/string_type.hpp"
#include "serialization/document_node.hpp"

namespace grace {

void StringType::deserialize(String& place, const DocumentNode& node, IUniverse&) const {
	node >> place;
}

void StringType::serialize(const String& place, DocumentNode& node, IUniverse&) const {
	node << place;
}

const StringType* StringType::get() {
	static const StringType type = StringType();
	return &type;
}

StringRef StringType::name() const {
	return "String";
}
	
const StringRefType* StringRefType::get() {
	static const StringRefType type = StringRefType();
	return &type;
}
	
void StringRefType::deserialize(StringRef& place, const DocumentNode& node, IUniverse&) const {
	node >> place;
}

void StringRefType::serialize(const StringRef &place, DocumentNode & node, IUniverse &) const {
	node << place;
}

StringRef StringRefType::name() const {
	return "StringRef";
}

}