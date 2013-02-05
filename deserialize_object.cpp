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
	const StructuredType* get_type_from_map(const ArchiveNode& node, String& out_error);
	
	const ObjectTypeBase* get_class_from_map(const ArchiveNode& node, String& out_error) {
		String clsname;
		const ArchiveNode& cls_node = node["class"];
		if (!(cls_node >> clsname)) {
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
	
	const StructuredType* transform_if_composite_type(const ArchiveNode& node, const ObjectTypeBase* base_type, String& out_error) {
		const ArchiveNode& aspects = node["aspects"];
		if (!aspects.is_array()) return base_type;
		if (aspects.array_size() == 0) return base_type;
		
		CompositeType* type = new CompositeType(default_allocator(), "Composite", base_type); // TODO: Use universe allocator
		for (size_t i = 0; i < aspects.array_size(); ++i) {
			const ArchiveNode& aspect = aspects[i];
			const StructuredType* aspect_type = get_type_from_map(aspect, out_error);
			if (aspect_type == nullptr) {
				return nullptr;
			}
			type->add_aspect(aspect_type);
		}
		type->freeze();
		return type;
	}
	
	const StructuredType* get_type_from_map(const ArchiveNode& node, String& out_error) {
		const ObjectTypeBase* struct_type = get_class_from_map(node, out_error);
		if (struct_type != nullptr) {
			return transform_if_composite_type(node, struct_type, out_error);
		}
		return nullptr;
	}
	
	
	struct MergedArchiveNode : public ArchiveNode {
	public:
		MergedArchiveNode(Archive& archive) : ArchiveNode(archive) {}
	};
	
	void merge_archive_node_map(ArchiveNode& into, const ArchiveNode& from) {
		from.value().when<ArchiveNode::MapType>([&](const ArchiveNode::MapType& from_map) {
			if (!into.is_map()) {
				into.value() = ArchiveNode::MapType(into.allocator());
			}
			into.value().when<ArchiveNode::MapType>([&](ArchiveNode::MapType& into_map) {
				for (auto pair: from_map) {
					into_map[pair.first] = pair.second;
				}
			});
		});
	}
	
	void copy_archive_node(ArchiveNode& to, const ArchiveNode& from) {
		if (from.is_scalar()) {
			to.value() = from.value();
		} else if (from.is_array()) {
			for (size_t i = 0; i < from.array_size(); ++i) {
				ArchiveNode& n = to.array_push();
				copy_archive_node(n, from[i]);
			}
		} else if (from.is_map()) {
			from.map_each_pair([&](StringRef key, const ArchiveNode* child) {
				ArchiveNode& n = to[key];
				copy_archive_node(n, *child);
			});
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
	if (node["template"] >> template_rid) {
		ResourcePtr<ObjectTemplate> templ = load_resource<ObjectTemplate>(template_rid);
		if (templ != nullptr) {
			copy_archive_node(merged_node, templ->archive.root());
		}
	}
	merge_archive_node_map(merged_node, node);
	
	String error;
	const StructuredType* type = get_type_from_map(merged_node, error);
	if (type == nullptr) {
		Error() << error;
		return nullptr;
	}
	
	String id;
	if (!(merged_node["id"] >> id)) {
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