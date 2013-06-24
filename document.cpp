#include "serialization/document.hpp"
#include "object/objectptr.hpp"
#include "serialization/serialize.hpp"
#include "type/type_registry.hpp"
#include "object/composite_type.hpp"
#include "object/universe.hpp"
#include "serialization/deserialize_object.hpp"

namespace grace {
	DocumentNode& Document::operator[](StringRef key) {
		return root()[key];
	}
	
	const DocumentNode& Document::operator[](StringRef key) const {
		return root()[key];
	}
	
	DocumentNode* Document::make() {
		return nodes_.allocate(*this);
	}
	
	void Document::clear() {
		nodes_.clear();
		root_.clear();
	}
}