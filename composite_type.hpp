#pragma once
#ifndef COMPOSITE_TYPE_HPP_K5R3HGBW
#define COMPOSITE_TYPE_HPP_K5R3HGBW

#include "type/structured_type.hpp"
#include "serialization/archive.hpp"
#include "base/array.hpp"
#include "object/object_type_base.hpp"
#include "type/attribute.hpp"
#include <new>

namespace grace {
	
struct ExposedAttribute : public IAttribute {
	// IAttribute interface
	const IType* type() const final { return attribute_->type(); }
	StringRef name() const final { return attribute_->name(); }
	StringRef description() const final { return attribute_->description(); }
	Any get_any(const Object* object) const final;
	Any get_any(Object* object) const final;
	bool set_any(Object* object, const Any& value) const final;
	void deserialize_attribute(Object* object, const ArchiveNode&, IUniverse&) const final;
	void serialize_attribute(const Object* object, ArchiveNode&, IUniverse&) const final;
	bool deferred_instantiation() const final { return attribute_->deferred_instantiation(); }
	bool is_read_only() const final { return attribute_->is_read_only(); }
	bool is_opaque() const final { return attribute_->is_opaque(); }
	
	// ExposedAttribute interface
	size_t aspect() const { return aspect_idx_; }
	const IAttribute* attribute() const { return attribute_; }
private:
	size_t aspect_idx_;
	const IAttribute* attribute_;
	friend class CompositeType;
	ExposedAttribute(size_t aspect_idx, const IAttribute* attribute) : aspect_idx_(aspect_idx), attribute_(attribute) {}
};
	
	struct ExposedSlot : public ISlot {
		// ISlot interface
		StringRef name() const final;
		StringRef description() const final;
		Array<const Type*> signature(IAllocator& alloc) const final;
		bool invoke(ObjectPtr<> receiver, ArrayRef<Any> args) const final;
		
		// Deprecated:
		void invoke_with_serialized_arguments(ObjectPtr<> receiver, const ArchiveNode& arg_list, IUniverse& universe) const final;
		String signature_description(IAllocator& alloc) const final;
		
		// ExposedSlot interface
		size_t aspect() const { return aspect_idx_; }
		const ISlot* slot() const { return slot_; }
	private:
		size_t aspect_idx_;
		const ISlot* slot_;
		friend class CompositeType;
		ExposedSlot(size_t aspect_idx, const ISlot* slot) : aspect_idx_(aspect_idx), slot_(slot) {}
	};

void resolve_exposed_attribute(const IAttribute*& inout_attr, ObjectPtr<>& inout_object);

struct CompositeType : StructuredType {
	CompositeType(IAllocator& alloc, StringRef name, const ObjectTypeBase* base_type = nullptr);
	~CompositeType();
	
	// CompositeType interface
	IAllocator& allocator() const;
	const ObjectTypeBase* base_type() const;
	size_t base_size() const;
	void add_aspect(const StructuredType* aspect);
	void freeze() { frozen_ = true; }
	size_t num_aspects() const { return aspects_.size(); }
	Object* get_aspect_in_object(Object* object, size_t idx) const;
	const Object* get_aspect_in_object(const Object* object, size_t idx) const;
	Object* find_aspect_of_type(Object* composite_object, const StructuredType* aspect, const StructuredType* skip_in_search = nullptr) const;
	void expose_attribute(size_t aspect_idx, StringRef attr_name);
	void unexpose_attribute(size_t aspect_idx, StringRef attr_name);
	void expose_slot(size_t aspect_idx, StringRef slot_name);
	void unexpose_slot(size_t aspect_idx, StringRef slot_name);
	ArrayRef<const ExposedAttribute*> exposed_attributes() const;
	const StructuredType* get_aspect(size_t aspect_idx) const;
	
	// Type interface
	void construct(byte* place, IUniverse&) const override;
	void destruct(byte* place, IUniverse&) const override;
	void copy_construct(byte* place, const byte* original) const override { ASSERT(false); }
	void move_construct(byte* place, byte* original) const override { ASSERT(false); }
	void deserialize_raw(byte* place, const ArchiveNode& node, IUniverse&) const override;
	void serialize_raw(const byte* place, ArchiveNode& node, IUniverse&) const override;
	StringRef name() const override { return name_; }
	size_t size() const override { return size_; }
	size_t alignment() const override { return alignof(Object); }
	
	// DerivedType interface
	size_t num_elements() const { return aspects_.size(); }
	size_t offset_of_element(size_t idx) const;
	const Type* type_of_element(size_t idx) const { return aspects_[idx]; }
	
	// StructuredType interface
	ArrayRef<const IAttribute*> attributes() const override;
	ArrayRef<const ISlot* const> slots() const override;
	const StructuredType* super() const final;
private:
	const ObjectTypeBase* base_type_;
	String name_;
	Array<const StructuredType*> aspects_; // TODO: Consider ownership
	bool frozen_;
	size_t size_;
	Array<ExposedAttribute*> exposed_attributes_;
	Array<ExposedSlot*> exposed_slots_;
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
	
	inline const Object* CompositeType::get_aspect_in_object(const Object *object, size_t idx) const {
		ASSERT(object->object_type() == this);
		const byte* memory = reinterpret_cast<const byte*>(object);
		size_t offset = offset_of_element(idx);
		return reinterpret_cast<const Object*>(memory + offset);
	}
}

#endif /* end of include guard: COMPOSITE_TYPE_HPP_K5R3HGBW */
