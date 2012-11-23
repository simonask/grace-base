#pragma once
#ifndef UNIVERSE_HPP_VHU9428R
#define UNIVERSE_HPP_VHU9428R

#include <string>
#include <map>
#include "object/object.hpp"
#include "object/objectptr.hpp"
#include "base/array_ref.hpp"
#include "base/intrusive_list.hpp"
#include "memory/allocator.hpp"

namespace falling {

struct DerivedType;
void error_category_already_initialized_with_different_type(const std::string& name);
void warn_attempt_to_get_objects_of_unindexed_type(const ObjectTypeBase*);

struct UniverseBase {
	virtual ObjectPtr<> create_object(const DerivedType* type, std::string id) = 0;
	virtual ObjectPtr<> create_root(const DerivedType* type, std::string id) = 0;
	virtual ObjectPtr<> get_object(const std::string& id) const = 0;
	virtual const std::string& get_id(ObjectPtr<const Object> object) const = 0;
	virtual bool rename_object(ObjectPtr<> object, std::string new_id) = 0;
	virtual ObjectPtr<> root() const = 0;
	virtual void set_root(ObjectPtr<> root) = 0;
	virtual void initialize_all() = 0;
	virtual ~UniverseBase() {}
	
	template <typename T>
	ObjectPtr<T> create(std::string id) {
		ObjectPtr<> o = this->create_object(get_type<T>(), std::move(id));
		ObjectPtr<T> ptr = o.cast<T>();
		ASSERT(ptr != nullptr); // create_object did not create an instance of T.
		return ptr;
	}
	
	template <typename T>
	ObjectPtr<T> create_root(std::string id) {
		ObjectPtr<> o = this->create_root(get_type<T>(), std::move(id));
		ObjectPtr<T> ptr = o.cast<T>();
		ASSERT(ptr != nullptr);
		return ptr;
	}
	
	template <typename T, size_t MemberOffset>
	IntrusiveList<T, MemberOffset>& get_intrusive_list();
	
	template <typename IntrusiveListType>
	IntrusiveListType& get_intrusive_list() {
		return get_intrusive_list<typename IntrusiveListType::ValueType, IntrusiveListType::LinkOffset>();
	}
private:
	std::map<const Type*, std::map<size_t, VirtualIntrusiveListBase*>> intrusive_lists;
};

template <typename T, size_t MemberOffset>
IntrusiveList<T, MemberOffset>& UniverseBase::get_intrusive_list() {
	VirtualIntrusiveListBase* base_ptr = nullptr;
	const Type* type = get_type<T>();
	auto it1 = intrusive_lists.find(type);
	if (it1 != intrusive_lists.end()) {
		auto& map2 = it1->second;
		auto it2 = map2.find(MemberOffset);
		if (it2 != map2.end()) {
			base_ptr = it2->second;
		} else {
			base_ptr = new VirtualIntrusiveList<T, MemberOffset>;
			map2[MemberOffset] = base_ptr;
		}
	} else {
		base_ptr = new VirtualIntrusiveList<T, MemberOffset>;
		std::map<size_t, VirtualIntrusiveListBase*> m = {{MemberOffset, base_ptr}};
		intrusive_lists[type] = std::move(m);
	}
	
	IntrusiveList<T, MemberOffset>* ptr = nullptr;
#if DEBUG
	ptr = dynamic_cast<VirtualIntrusiveList<T, MemberOffset>*>(base_ptr);
	ASSERT(ptr != nullptr);
#else
	ptr = static_cast<VirtualIntrusiveList<T, MemberOffset>*>(base_ptr);
#endif
	return *ptr;
}

struct BasicUniverse : UniverseBase {
	ObjectPtr<> create_object(const DerivedType* type, std::string) override;
	ObjectPtr<> create_root(const DerivedType* type, std::string) override;
	ObjectPtr<> get_object(const std::string& id) const override {
		return find_or(object_map_, id, nullptr);
	}
	const std::string& get_id(ObjectPtr<const Object> object) const override;
	bool rename_object(ObjectPtr<> object, std::string) override;
	ObjectPtr<> root() const override { return root_; }
	void set_root(ObjectPtr<> r) {
		ASSERT(r != nullptr && r->universe() == this);
		root_ = r;
	}
	void initialize_all();
	
	BasicUniverse(IAllocator& alloc = default_allocator()) : allocator_(alloc) {}
	~BasicUniverse() { clear(); }
	void clear();
private:
	IAllocator& allocator_;
	std::map<std::string, ObjectPtr<>> object_map_;
	std::map<ObjectPtr<const Object>, std::string> reverse_object_map_;
	Array<Object*> memory_map_;
	ObjectPtr<> root_;
	std::string empty_id_;
};
	
typedef BasicUniverse TestUniverse;

}

#endif /* end of include guard: UNIVERSE_HPP_VHU9428R */
