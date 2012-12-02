#include "object/composite_type.hpp"
#include "object/object_type.hpp"

namespace falling {

CompositeType::CompositeType(String name, const ObjectTypeBase* base_type) : base_type_(base_type), name_(std::move(name)), frozen_(false) {
	size_ = this->base_type()->size();
}
	
size_t CompositeType::base_size() const {
	return base_type()->size();
}

void CompositeType::add_aspect(const DerivedType* aspect) {
	ASSERT(!frozen_);
	ASSERT(!aspect->is_abstract());
	aspects_.push_back(aspect); // TODO: Check for circular dependencies.
	size_ += aspect->size();
}
	
Object* CompositeType::find_aspect_of_type(Object* composite_object, const DerivedType* aspect_type, const DerivedType* skip_in_search) const {
	ASSERT(composite_object->object_type() == this);
	
	// Breadth:
	for (size_t i = 0; i < aspects_.size(); ++i) {
		if (aspects_[i] == aspect_type) {
			return get_aspect_in_object(composite_object, i);
		}
	}
	
	// Depth:
	for (size_t i = 0; i < aspects_.size(); ++i) {
		if (aspects_[i] != skip_in_search) {
			const CompositeType* comp = dynamic_cast<const CompositeType*>(aspects_[i]);
			if (comp != nullptr) {
				Object* subcomp = get_aspect_in_object(composite_object, i);
				Object* r = comp->find_aspect_of_type(subcomp, aspect_type);
				if (r != nullptr) {
					return r;
				}
			}
		}
	}
	
	return nullptr;
}

const ObjectTypeBase* CompositeType::base_type() const {
	return base_type_ ? base_type_ : get_type<Object>();
}


void CompositeType::construct(byte* place, UniverseBase& universe) const {
	base_type()->construct(place, universe);
	Object* obj = reinterpret_cast<Object*>(place);
	obj->set_object_type__(this);
	size_t offset = base_type()->size();
	for (auto aspect: aspects_) {
		aspect->construct(place + offset, universe);
		Object* subobject = reinterpret_cast<Object*>(place + offset);
		subobject->set_object_offset__((uint32)offset);
		subobject->set_object_id(aspect->name()); // might be renamed later by deserialization
		offset += aspect->size();
	}
	ASSERT(offset == size_);
}

void CompositeType::destruct(byte* place, UniverseBase& universe) const {
	size_t offset = base_type()->size();
	for (auto aspect: aspects_) { // TODO: Consider doing this backwards?
		aspect->destruct(place + offset, universe);
		offset += aspect->size();
	}
	base_type()->destruct(place, universe);
	ASSERT(offset == size_);
}

void CompositeType::deserialize_raw(byte* place, const ArchiveNode& node, UniverseBase& universe) const {
	ASSERT(frozen_);
	base_type()->deserialize_raw(place, node, universe);
	
	const ArchiveNode& aspect_array = node["aspects"];
	if (aspect_array.is_array()) {
		size_t offset = base_type()->size();
		size_t sz = aspect_array.array_size();
		for (size_t i = 0; i < sz; ++i) {
			const ArchiveNode& aspect_node = aspect_array[i];
			aspects_[i]->deserialize_raw(place + offset, aspect_node, universe);
			Object* subobject = reinterpret_cast<Object*>(place + offset);
			subobject->set_object_offset__((uint32)offset);
			offset += aspects_[i]->size();
		}
		ASSERT(offset == size_);
	}
}

void CompositeType::serialize_raw(const byte* place, ArchiveNode& node, UniverseBase& universe) const {
	ASSERT(frozen_);
	base_type()->serialize_raw(place, node, universe);
	node["class"] = base_type()->name();
	
	size_t offset = base_type()->size();
	ArchiveNode& aspect_array = node["aspects"];
	for (auto aspect: aspects_) {
		ArchiveNode& aspect_node = aspect_array.array_push();
		aspect->serialize_raw(place + offset, aspect_node, universe);
		offset += aspect->size();
	}
	ASSERT(offset == size_);
}

}