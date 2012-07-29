#pragma once
#ifndef COMPOSITE_TYPE_HPP_K5R3HGBW
#define COMPOSITE_TYPE_HPP_K5R3HGBW

#include "type/type.hpp"
#include "serialization/archive.hpp"
#include "base/array.hpp"
#include <new>

namespace falling {

struct CompositeType : DerivedType {
	CompositeType(std::string name, const ObjectTypeBase* base_type = nullptr);
	
	const ObjectTypeBase* base_type() const;
	size_t base_size() const;
	void add_aspect(const DerivedType* aspect);
	void freeze() { frozen_ = true; }
	size_t num_aspects() const { return aspects_.size(); }
	Object* get_aspect_in_object(Object* object, size_t idx) const;
	
	// Type interface
	void construct(byte* place, IUniverse&) const override;
	void destruct(byte* place, IUniverse&) const override;
	void deserialize(byte* place, const ArchiveNode& node, IUniverse&) const override;
	void serialize(const byte* place, ArchiveNode& node, IUniverse&) const override;
	std::string name() const override { return name_; }
	size_t size() const override { return size_; }
	
	// DerivedType interface
	size_t num_elements() const { return aspects_.size(); }
	size_t offset_of_element(size_t idx) const;
	const Type* type_of_element(size_t idx) const { return aspects_[idx]; }
	
	Object* cast(const DerivedType* to, Object* o) const override;
	Object* find_instance_down(const DerivedType* of_type, Object* o, const DerivedType* avoid = nullptr) const;
	Object* find_instance_up(const DerivedType* of_type, Object* o, const DerivedType* avoid = nullptr) const;
	Object* find_self_up(Object* o) const;
private:
	Object* cast(const DerivedType* to, Object* o, const DerivedType* avoid) const;
	
	const ObjectTypeBase* base_type_;
	std::string name_;
	Array<const DerivedType*> aspects_;
	bool frozen_;
	size_t size_;
};

inline size_t CompositeType::offset_of_element(size_t idx) const {
	size_t offset = base_size();
	for (size_t i = 0; i < aspects_.size(); ++i) {
		if (i == idx) return offset;
		offset += aspects_[i]->size();
	}
	ASSERT(false); // unreachable
	return SIZE_T_MAX;
}
	
inline Object* CompositeType::get_aspect_in_object(Object *object, size_t idx) const {
	ASSERT(object->object_type() == this);
	byte* memory = reinterpret_cast<byte*>(object);
	size_t offset = offset_of_element(idx);
	return reinterpret_cast<Object*>(memory + offset);
}

}

#endif /* end of include guard: COMPOSITE_TYPE_HPP_K5R3HGBW */
