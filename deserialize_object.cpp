#include "serialization/deserialize_object.hpp"
#include "serialization/archive_node.hpp"
#include "type/type_registry.hpp"
#include "object/composite_type.hpp"
#include "object/object_type.hpp"
#include "object/universe.hpp"
#include "object/object_template.hpp"
#include "base/log.hpp"
#include "io/resource_ptr.hpp"
#include "io/resource_manager.hpp"
#include <memory>

namespace falling {

namespace {
	const DerivedType* get_type_from_map(const ArchiveNode& node, std::string& out_error);
	
	const ObjectTypeBase* get_class_from_map(const ArchiveNode& node, std::string& out_error) {
		std::string clsname;
		if (!node["class"].get(clsname)) {
			out_error = "Class not specified.";
			return nullptr;
		}
		const ObjectTypeBase* struct_type = TypeRegistry::get(clsname);
		if (struct_type == nullptr) {
			out_error = "Class '" + clsname + "' not registered.";
			return nullptr;
		}
		return struct_type;
	}
	
	const DerivedType* transform_if_composite_type(const ArchiveNode& node, const ObjectTypeBase* base_type, std::string& out_error) {
		const ArchiveNode& aspects = node["aspects"];
		if (!aspects.is_array()) return base_type;
		if (aspects.array_size() == 0) return base_type;
		
		CompositeType* type = new CompositeType("Composite", base_type);
		for (size_t i = 0; i < aspects.array_size(); ++i) {
			const ArchiveNode& aspect = aspects[i];
			const DerivedType* aspect_type = get_type_from_map(aspect, out_error);
			if (aspect_type == nullptr) {
				return nullptr;
			}
			type->add_aspect(aspect_type);
		}
		type->freeze();
		return type;
	}
	
	const DerivedType* get_type_from_map(const ArchiveNode& node, std::string& out_error) {
		const ObjectTypeBase* struct_type = get_class_from_map(node, out_error);
		if (struct_type != nullptr) {
			return transform_if_composite_type(node, struct_type, out_error);
		}
		return nullptr;
	}
	
	
	struct MergedArchiveNode : public ArchiveNode {
	public:
		MergedArchiveNode(Archive& archive) : ArchiveNode(archive, ArchiveNodeType::Map) {}
	};
	
	void merge_archive_node_map(ArchiveNode& into, const ArchiveNode& from) {
		for (auto& pair: from.internal_map()) {
			into.internal_map()[pair.first] = pair.second;
		}
	}
	
	void copy_archive_node(ArchiveNode& to, const ArchiveNode& from) {
		switch (from.type()) {
			case ArchiveNodeType::Empty: to.clear(); break;
			case ArchiveNodeType::Float:
				double f;
				from.get(f);
				to.set(f);
				break;
			case ArchiveNodeType::Integer:
				int64 n;
				from.get(n);
				to.set(n);
				break;
			case ArchiveNodeType::String: {
				std::string s;
				from.get(s);
				to.set(std::move(s));
				break;
			}
			case ArchiveNodeType::Array: {
				for (size_t i = 0; i < from.array_size(); ++i) {
					ArchiveNode& n = to.array_push();
					copy_archive_node(n, from[i]);
				}
				break;
			}
			case ArchiveNodeType::Map: {
				for (auto& it: from.internal_map()) {
					ArchiveNode& n = to[it.first];
					copy_archive_node(n, *it.second);
				}
				break;
			}
		}
	}
}

ObjectPtr<> deserialize_object(const ArchiveNode& node, UniverseBase& universe) {
	if (!node.is_map()) {
		Error() << "Expected object, got non-map.";
		return nullptr;
	}
	
	MergedArchiveNode merged_node(node.archive());
	ResourceID template_rid;
	if (node["template"].get(template_rid)) {
		ResourcePtr<ObjectTemplate> templ = load_resource<ObjectTemplate>(template_rid);
		if (templ != nullptr) {
			copy_archive_node(merged_node, templ->archive.root());
		}
	}
	merge_archive_node_map(merged_node, node);
	
	std::string error;
	const DerivedType* type = get_type_from_map(merged_node, error);
	if (type == nullptr) {
		Error() << error;
		return nullptr;
	}
	
	std::string id;
	if (!merged_node["id"].get(id)) {
		Warning() << "Object without id.";
	}
	
	ObjectPtr<> ptr = universe.create_object(type, id);
	if (ptr->object_id() != id) {
		Warning() << "Object '" << id << "' was renamed to '" << ptr->object_id() << "' because of a collision.\n";
	}
	
	type->deserialize_raw(reinterpret_cast<byte*>(ptr.get()), merged_node, universe);
	
	return ptr;
}

}