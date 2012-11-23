#include "serialization/archive.hpp"
#include "object/objectptr.hpp"
#include "serialization/serialize.hpp"
#include "type/type_registry.hpp"
#include "object/composite_type.hpp"
#include "object/universe.hpp"
#include "serialization/deserialize_object.hpp"

namespace falling {

void Archive::serialize(ObjectPtr<> object, UniverseBase& universe) {
	falling::serialize(*object, root(), universe);
	for (auto ref: serialize_references) {
		ref->perform(universe);
	}
	serialize_references.clear();
}

bool Archive::deserialize(UniverseBase& universe, std::string& out_error) {
	ObjectPtr<> ptr = deserialize_object(root(), universe);
	
	for (auto it: deserialize_references) {
		it->perform(universe);
	}
	for (auto it: deserialize_signals) {
		it->perform(universe);
	}
	
	universe.set_root(ptr);
	
	universe.initialize_all();
	
	return true;
}
	
	ArchiveNode& Archive::operator[](const std::string& key) {
		return root()[key];
	}
	
	const ArchiveNode& Archive::operator[](const std::string& key) const {
		return root()[key];
	}

}