#pragma once
#ifndef STRUCT_TYPE_HPP_PTB31EJN
#define STRUCT_TYPE_HPP_PTB31EJN

#include "type/structured_type.hpp"
#include <memory>
#include <new>
#include "type/attribute.hpp"
#include "object/slot.hpp"
#include "object/universe.hpp"
#include "object/universe_base.hpp"
#include "object/object_type_base.hpp"

namespace grace {

template <typename T> struct AutoListRegistrarForObject;

template <typename T>
struct ObjectType : TypeFor<T, ObjectTypeBase> {
	ObjectType(IAllocator& alloc, const ObjectTypeBase* super, StringRef name, StringRef description)
		: TypeFor<T, ObjectTypeBase>(alloc, super, name, description)
		, properties_(alloc)
		, slots_(alloc)
		, lists_(alloc)
		{}
	
	void construct(byte* place, IUniverse& universe) const {
		TypeFor<T,ObjectTypeBase>::construct(place, universe);
		Object* p = reinterpret_cast<Object*>(place);
		p->set_object_type__(this);
		p->set_universe__(&universe);
	}
	
	ArrayRef<const IAttribute*> attributes() const {
		return ArrayRef<const IAttribute*>((IAttribute const **)properties_.data(), (IAttribute const **)properties_.data() + properties_.size());
	}
	ArrayRef<const ISlot* const> slots() const {
		return ArrayRef<const ISlot* const>((const ISlot* const*)slots_.data(), (const ISlot* const*)slots_.data() + slots_.size());
	}
	
	size_t num_elements() const { return properties_.size(); }
	const IType* type_of_element(size_t idx) const { return properties_[idx]->attribute_type(); }
	size_t offset_of_element(size_t idx) const { return 0; /* TODO */ }
	
	void deserialize(T& object, const DocumentNode&, IUniverse&) const;
	void serialize(const T& object, DocumentNode&, IUniverse&) const;
	
	const ISlot* find_slot_by_name(StringRef name) const {
		for (auto& it: slots_) {
			if (it->name() == name) return it;
		}
		return nullptr;
	}

	Array<IAttribute*> properties_;
	Array<ISlot*> slots_;
	Array<AutoListRegistrarForObject<T>*> lists_;
};

template <typename T>
struct AutoListRegistrarForObject {
	virtual void link_object_in_universe(T& object, IUniverse& universe) const = 0;
};


template <typename T>
void ObjectType<T>::deserialize(T& object, const DocumentNode& node, IUniverse& universe) const {
	auto s = this->super();
	if (s) s->deserialize_raw(reinterpret_cast<byte*>(&object), node, universe);
	
	for (auto& property: properties_) {
		ObjectPtr<> o = ObjectPtr<>(&object);
		const DocumentNode& serialized = node[property->name()];
		if (property->is_read_only())
			continue;
		if (property->deferred_instantiation()) {
			universe.defer_attribute_deserialization(o, property, &serialized);
		} else {
			property->deserialize_attribute(o.get(), serialized, universe);
		}
	}
	
	for (auto& list: lists_) {
		list->link_object_in_universe(object, universe);
	}
}

template <typename T>
void ObjectType<T>::serialize(const T& object, DocumentNode& node, IUniverse& universe) const {
	auto s = this->super();
	if (s) s->serialize_raw(reinterpret_cast<const byte*>(&object), node, universe);
	
	for (auto& property: properties_) {
		if (!property->is_opaque()) {
			property->serialize_attribute(&object, node[property->name()], universe);
		}
	}
	node["class"] << this->name();
}

}

#endif /* end of include guard: STRUCT_TYPE_HPP_PTB31EJN */
