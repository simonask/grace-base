#include "object/composite_type.hpp"
#include "object/object_type.hpp"
#include "base/log.hpp"

namespace falling {

CompositeType::CompositeType(IAllocator& alloc, StringRef name, const ObjectTypeBase* base_type) : base_type_(base_type), name_(std::move(name), alloc), aspects_(alloc), exposed_attributes_(alloc), exposed_slots_(alloc), frozen_(false) {
	size_ = this->base_type()->size();
}
	
	CompositeType::~CompositeType() {
		for (auto p: exposed_attributes_) {
			destroy(p, allocator());
		}
	}
	
	IAllocator& CompositeType::allocator() const {
		return name_.allocator();
	}
	
size_t CompositeType::base_size() const {
	return base_type()->size();
}

void CompositeType::add_aspect(const StructuredType* aspect) {
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
	
	void CompositeType::expose_attribute(size_t aspect_idx, StringRef attr_name) {
		ASSERT(!frozen_);
		ASSERT(aspect_idx < aspects_.size());
		const StructuredType* aspect_type = aspects_[aspect_idx];
		const IAttribute* attr = aspect_type->find_attribute_by_name(attr_name);
		if (attr == nullptr) {
			Error() << "Cannot expose attribute '" << attr_name << "', because it doesn't exist on aspect '" << aspect_type->name() << "' in composite.";
			return;
		}
		
#if DEBUG
		for (auto p: exposed_attributes_) {
			if (p->aspect() == aspect_idx && p->attribute() == attr) {
				Warning() << "Attribute '" << attr_name << "' from aspect '" << aspect_type->name() << "' has already been exposed on composite.";
				return;
			}
		}
#endif
		
		exposed_attributes_.push_back(new(allocator()) ExposedAttribute(aspect_idx, attr));
	}

	ArrayRef<const IAttribute*> CompositeType::attributes() const {
		auto p = (IAttribute const**)exposed_attributes_.data();
		return ArrayRef<const IAttribute*>(p, p + exposed_attributes_.size());
	}
	
	ArrayRef<const ISlot* const> CompositeType::slots() const {
		auto p = (ISlot const**)exposed_slots_.data();
		return ArrayRef<const ISlot* const>(p, p + exposed_slots_.size());
	}
	
	Any ExposedAttribute::get_any(const Object* object) const {
		const StructuredType* st = object->object_type();
		const CompositeType* ct = dynamic_cast<const CompositeType*>(st);
		ASSERT(st != nullptr); // type mismatch
		return attribute_->get_any(ct->get_aspect_in_object(object, aspect_idx_));
	}
	
	Any ExposedAttribute::get_any(Object* object) const {
		const StructuredType* st = object->object_type();
		const CompositeType* ct = dynamic_cast<const CompositeType*>(st);
		ASSERT(st != nullptr); // type mismatch
		return attribute_->get_any(ct->get_aspect_in_object(object, aspect_idx_));
	}
	
	bool ExposedAttribute::set_any(Object* object, const Any& value) const {
		const StructuredType* st = object->object_type();
		const CompositeType* ct = dynamic_cast<const CompositeType*>(st);
		ASSERT(st != nullptr); // type mismatch
		return attribute_->set_any(ct->get_aspect_in_object(object, aspect_idx_), value);
	}
	
	void ExposedAttribute::deserialize_attribute(Object* object, const ArchiveNode &, UniverseBase & u) const {
		ASSERT(false); // Composite objects must be deserialized with the specialized algorithm.
	}
	
	void ExposedAttribute::serialize_attribute(const Object *object, ArchiveNode &, UniverseBase &) const {
		ASSERT(false); // Composite objects must be serialized with the specialized algorithm.
	}
	
	void resolve_exposed_attribute(const IAttribute*& inout_attr, ObjectPtr<>& inout_object) {
		bool is_real = false;
		while (true) {
			const ExposedAttribute* exattr = dynamic_cast<const ExposedAttribute*>(exattr);
			if (exattr != nullptr) {
				const CompositeType* ct = dynamic_cast<const CompositeType*>(inout_object->object_type());
				ASSERT(ct != nullptr); // ExposedAttribute on non-composite object.
				inout_object = ct->get_aspect_in_object(inout_object.get(), exattr->aspect());
				inout_attr = exattr->attribute();
			} else {
				break;
			}
		}
	}
}