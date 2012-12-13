#pragma once
#ifndef STRUCT_TYPE_HPP_PTB31EJN
#define STRUCT_TYPE_HPP_PTB31EJN

#include "type/type.hpp"
#include <memory>

#include <new>
#include "type/attribute.hpp"
#include "object/slot.hpp"
#include "object/universe.hpp"
#include "base/link_list.hpp"

namespace falling {

struct ObjectTypeBase : DerivedType {
	String name() const override { return name_; }
	const String& description() const { return description_; }
	const ObjectTypeBase* super() const;
	virtual ArrayRef<const AttributeBase* const> attributes() const = 0;
	virtual size_t num_slots() const = 0;
	virtual const SlotBase* slot_at(size_t idx) const = 0;
	virtual const SlotBase* find_slot_by_name(const String& name) const = 0;
	
	template <typename T, typename R, typename... Args>
	const SlotForTypeWithSignature<T,R,Args...>* find_slot_for_method(typename GetMemberFunctionPointerType<T, R, Args...>::Type method) const {
		size_t n = num_slots();
		for (size_t i = 0; i < n; ++i) {
			auto s = slot_at(i);
			auto slot = dynamic_cast<const SlotForTypeWithSignature<T,R,Args...>*>(s);
			if (slot != nullptr) {
				if (slot->method() == method) {
					return slot;
				}
			}
		}
		return nullptr;
	}
	
	ObjectTypeBase(const ObjectTypeBase* super, String name, String description) : super_(super), name_(std::move(name)), description_(std::move(description)), is_abstract_(false) {}
	
	const ObjectTypeBase* super_;
	String name_;
	String description_;
	bool is_abstract_;
	
	void set_abstract(bool b) { this->is_abstract_ = b; }
	bool is_abstract() const { return this->is_abstract_; }};

template <typename T> struct AutoListRegistrarForObject;

template <typename T>
struct ObjectType : TypeFor<T, ObjectTypeBase> {
	ObjectType(const ObjectTypeBase* super, String name, String description)
		: TypeFor<T, ObjectTypeBase>(super, std::move(name), std::move(description))
		, properties_(static_allocator())
		, slots_(static_allocator())
		, lists_(static_allocator())
		{}
	
	void construct(byte* place, UniverseBase& universe) const {
		TypeFor<T,ObjectTypeBase>::construct(place, universe);
		Object* p = reinterpret_cast<Object*>(place);
		p->set_object_type__(this);
		p->set_universe__(&universe);
	}
	
	ArrayRef<const AttributeBase* const> attributes() const {
		return ArrayRef<const AttributeBase* const>((const AttributeBase* const*)properties_.data(), (const AttributeBase* const*)properties_.data() + properties_.size());
	}
	size_t num_slots() const { return slots_.size(); }
	const SlotBase* slot_at(size_t idx) const { return slots_[idx]; }
	
	size_t num_elements() const { return properties_.size(); }
	const Type* type_of_element(size_t idx) const { return properties_[idx]->attribute_type(); }
	size_t offset_of_element(size_t idx) const { return 0; /* TODO */ }
	
	void deserialize(T& object, const ArchiveNode&, UniverseBase&) const;
	void serialize(const T& object, ArchiveNode&, UniverseBase&) const;
	
	const SlotBase* find_slot_by_name(const String& name) const {
		for (auto& it: slots_) {
			if (it->name() == name) return it;
		}
		return nullptr;
	}

	Array<AttributeForObject<T>*> properties_;
	Array<SlotForType<T>*> slots_;
	Array<AutoListRegistrarForObject<T>*> lists_;
};

template <typename T>
struct AutoListRegistrarForObject {
	virtual void link_object_in_universe(T& object, UniverseBase& universe) const = 0;
};

template <typename T, typename ObjectType, size_t MemberOffset>
struct AutoListRegistrarImpl : AutoListRegistrarForObject<T> {
	typedef AutoListLink<ObjectType> ObjectType::* LinkMemberType;
	LinkMemberType link_;
	
	AutoListRegistrarImpl(LinkMemberType link) : link_(link) {}
	
	void link_object_in_universe(T& object, UniverseBase& universe) const {
		auto& list = universe.get_auto_list<ObjectType, MemberOffset>();
		AutoListLink<ObjectType>* link = &(object.*link_);
		list.link_head(link);
	}
};


template <typename T>
void ObjectType<T>::deserialize(T& object, const ArchiveNode& node, UniverseBase& universe) const {
	auto s = this->super();
	if (s) s->deserialize_raw(reinterpret_cast<byte*>(&object), node, universe);
	
	for (auto& property: properties_) {
		property->deserialize_attribute(&object, node[property->name()], universe);
	}
	
	for (auto& list: lists_) {
		list->link_object_in_universe(object, universe);
	}
}

template <typename T>
void ObjectType<T>::serialize(const T& object, ArchiveNode& node, UniverseBase& universe) const {
	auto s = this->super();
	if (s) s->serialize_raw(reinterpret_cast<const byte*>(&object), node, universe);
	
	for (auto& property: properties_) {
		property->serialize_attribute(&object, node[property->name()], universe);
	}
	node["class"] = this->name();
}

}

#endif /* end of include guard: STRUCT_TYPE_HPP_PTB31EJN */
