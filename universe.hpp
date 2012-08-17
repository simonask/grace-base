#pragma once
#ifndef UNIVERSE_HPP_VHU9428R
#define UNIVERSE_HPP_VHU9428R

#include <string>
#include <map>
#include "object/object.hpp"
#include "object/objectptr.hpp"
#include "base/array_ref.hpp"

namespace falling {

struct DerivedType;
void error_category_already_initialized_with_different_type(const std::string& name);
void warn_attempt_to_get_objects_of_unindexed_type(const ObjectTypeBase*);

struct IUniverse {
	virtual ObjectPtr<> create_object(const DerivedType* type, std::string id) = 0;
	virtual ObjectPtr<> create_root(const DerivedType* type, std::string id) = 0;
	virtual ObjectPtr<> get_object(const std::string& id) const = 0;
	virtual const std::string& get_id(ObjectPtr<const Object> object) const = 0;
	virtual bool rename_object(ObjectPtr<> object, std::string new_id) = 0;
	virtual ObjectPtr<> root() const = 0;
	virtual void set_root(ObjectPtr<> root) = 0;
	virtual void initialize_all() = 0;
	virtual ~IUniverse() {}
	
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
	
	template <typename T>
	ArrayRef<ObjectPtr<T>> objects_of_type() const;
	
	template <typename T>
	void register_object_of_type(ObjectPtr<T> ptr);
private:
	struct ObjectIndexBase;
	template <typename T> struct ObjectIndex;
	std::map<const ObjectTypeBase*, std::unique_ptr<ObjectIndexBase>> object_indexes;
};

struct IUniverse::ObjectIndexBase {
	virtual ~ObjectIndexBase() {}
};

template <typename T>
struct IUniverse::ObjectIndex : ObjectIndexBase {
	Array<ObjectPtr<T>> objects;
};

template <typename T>
ArrayRef<ObjectPtr<T>> IUniverse::objects_of_type() const {
	const ObjectTypeBase* type = get_type<T>();
#if DEBUG
	warn_attempt_to_get_objects_of_unindexed_type(type);
#endif
	auto it = object_indexes.find(type);
	if (it != object_indexes.end()) {
		ObjectIndex<T>* index = dynamic_cast<ObjectIndex<T>*>(it->second.get());
		if (index != nullptr) {
			return index->objects;
		}
	}
	return Empty();
}

template <typename T>
void IUniverse::register_object_of_type(ObjectPtr<T> ptr) {
	const ObjectTypeBase* type = get_type<T>();
	auto it = object_indexes.find(type);
	ObjectIndex<T>* index;
	if (it == object_indexes.end()) {
		index = new ObjectIndex<T>;
		object_indexes.insert(std::make_pair(type, std::unique_ptr<ObjectIndexBase>(index)));
	} else {
		index = dynamic_cast<ObjectIndex<T>*>(it->second.get());
	}
	ASSERT(index != nullptr);
	index->objects.push_back(ptr);
}

struct BasicUniverse : IUniverse {
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
	
	BasicUniverse() : root_(nullptr) {}
	~BasicUniverse() { clear(); }
	void clear();
private:
	std::map<std::string, ObjectPtr<>> object_map_;
	std::map<ObjectPtr<const Object>, std::string> reverse_object_map_;
	Array<Object*> memory_map_;
	ObjectPtr<> root_;
	std::string empty_id_;
};
	
typedef BasicUniverse TestUniverse;

}

#endif /* end of include guard: UNIVERSE_HPP_VHU9428R */
