#pragma once
#ifndef STRUCT_TYPE_HPP_PTB31EJN
#define STRUCT_TYPE_HPP_PTB31EJN

#include "type/type.hpp"
#include <memory>

#include <new>
#include "type/attribute.hpp"
#include "object/slot.hpp"
#include "object/universe.hpp"

namespace falling {

struct ObjectTypeBase : DerivedType {
	std::string name() const override { return name_; }
	const std::string& description() const { return description_; }
	const ObjectTypeBase* super() const;
	virtual Array<const AttributeBase*> attributes() const = 0;
	virtual size_t num_slots() const = 0;
	virtual const SlotBase* slot_at(size_t idx) const = 0;
	virtual const SlotBase* find_slot_by_name(const std::string& name) const = 0;
	
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
	
	ObjectTypeBase(const ObjectTypeBase* super, std::string name, std::string description) : super_(super), name_(std::move(name)), description_(std::move(description)) {}
	
	const ObjectTypeBase* super_;
	std::string name_;
	std::string description_;
};

template <typename T>
struct ObjectType : TypeFor<T, ObjectTypeBase> {
	ObjectType(const ObjectTypeBase* super, std::string name, std::string description) : TypeFor<T, ObjectTypeBase>(super, std::move(name), std::move(description)), is_abstract_(false) {}
	
	void construct(byte* place, IUniverse& universe) const {
		Object* p = ::new(place) T;
		p->set_object_type__(this);
		p->set_universe__(&universe);
	}
	
	Array<const AttributeBase*> attributes() const {
		Array<const AttributeBase*> result;
		result.reserve(properties_.size());
		for (auto it: properties_) {
			result.push_back(it);
		}
		return result;
	}
	size_t num_slots() const { return slots_.size(); }
	const SlotBase* slot_at(size_t idx) const { return slots_[idx]; }
	
	size_t num_elements() const { return properties_.size(); }
	const Type* type_of_element(size_t idx) const { return properties_[idx]->attribute_type(); }
	size_t offset_of_element(size_t idx) const { return 0; /* TODO */ }
	
	void deserialize(T& object, const ArchiveNode&, IUniverse&) const;
	void serialize(const T& object, ArchiveNode&, IUniverse&) const;
	
	void set_abstract(bool b) { is_abstract_ = b; }
	bool is_abstract() const { return is_abstract_; }
	
	const SlotBase* find_slot_by_name(const std::string& name) const {
		for (auto& it: slots_) {
			if (it->name() == name) return it;
		}
		return nullptr;
	}

	Array<AttributeForObject<T>*> properties_;
	Array<SlotForType<T>*> slots_;
	Array<std::string> categories_;
	bool is_abstract_;
};


template <typename T>
void ObjectType<T>::deserialize(T& object, const ArchiveNode& node, IUniverse& universe) const {
	auto s = this->super();
	if (s) s->deserialize(reinterpret_cast<byte*>(&object), node, universe);
	
	for (auto& property: properties_) {
		property->deserialize_attribute(&object, node[property->name()], universe);
	}
	
	for (auto category: categories_) {
		universe.register_object_for_category(ObjectPtr<T>(&object), category);
	}
}

template <typename T>
void ObjectType<T>::serialize(const T& object, ArchiveNode& node, IUniverse& universe) const {
	auto s = this->super();
	if (s) s->serialize(reinterpret_cast<const byte*>(&object), node, universe);
	
	for (auto& property: properties_) {
		property->serialize_attribute(&object, node[property->name()], universe);
	}
	node["class"] = this->name();
}

}

#endif /* end of include guard: STRUCT_TYPE_HPP_PTB31EJN */
