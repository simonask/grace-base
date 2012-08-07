#include "serialization/deserialize_object.hpp"
#include "serialization/archive_node.hpp"
#include "type/type_registry.hpp"
#include "object/composite_type.hpp"
#include "object/object_type.hpp"
#include "object/universe.hpp"
#include "object/object_template.hpp"
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
}

ObjectPtr<> deserialize_object(const ArchiveNode& node, IUniverse& universe) {
	if (!node.is_map()) {
		Error() << "Expected object, got non-map.";
		return nullptr;
	}
	
	MergedArchiveNode merged_node(node.archive());
	ResourceID template_rid;
	if (node["template"].get(template_rid)) {
		ResourcePtr<ObjectTemplate> templ = load_resource<ObjectTemplate>(template_rid);
		if (templ != nullptr) {
			merged_node.internal_map() = templ->archive.root().internal_map();
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
	
	type->deserialize(reinterpret_cast<byte*>(ptr.get()), merged_node, universe);
	
	return ptr;
}

}