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
	void register_object_for_category(ObjectPtr<T> object, const std::string& category_name);
	template <typename T>
	void unregister_object_from_all_categories(ObjectPtr<T> object);
	template <typename T>
	ArrayRef<ObjectPtr<T>> objects_for_category(const std::string& category_name);
	
private:
	struct ObjectCategoryBase;
	template <typename T> struct ObjectCategory;
	
	typedef std::map<std::string, std::unique_ptr<ObjectCategoryBase>> CategoryForNameMap;
	typedef std::map<const ObjectTypeBase*, CategoryForNameMap> CategoriesForTypeMap;
	CategoriesForTypeMap categories;
};
	
	struct IUniverse::ObjectCategoryBase {
		virtual ~ObjectCategoryBase() {}
	};
	
	template <typename T>
	struct IUniverse::ObjectCategory : ObjectCategoryBase {
		Array<ObjectPtr<T>> objects;
	};
	
	template <typename T>
	void IUniverse::register_object_for_category(ObjectPtr<T> object, const std::string &category_name) {
		const ObjectTypeBase* type = get_type<T>();
		auto categories_for_type_it = categories.find(type);
		if (categories_for_type_it == categories.end()) {
			auto pair = categories.emplace(type);
			categories_for_type_it = pair.first;
		}
		
		CategoryForNameMap& map = categories_for_type_it->second;
		auto category_for_name_it = map.find(category_name);
		if (category_for_name_it == map.end()) {
			category_for_name_it = map.emplace(category_name, std::unique_ptr<ObjectCategoryBase>(new ObjectCategory<T>())).first;
		}
		
		ObjectCategoryBase* category_base = category_for_name_it->second.get();
		auto category = dynamic_cast<ObjectCategory<T>*>(category_base);
		if (category == nullptr) {
			error_category_already_initialized_with_different_type(category_name);
		} else {
			category->objects.push_back(object);
		}
	}
	
	template <typename T>
	void IUniverse::unregister_object_from_all_categories(ObjectPtr<T> object) {
		// TODO!!
	}
	
	template <typename T>
	ArrayRef<ObjectPtr<T>> IUniverse::objects_for_category(const std::string &category_name) {
		auto it1 = categories.find(get_type<T>());
		if (it1 != categories.end()) {
			auto it2 = it1->second.find(category_name);
			if (it2 != it1->second.end()) {
				auto category = dynamic_cast<ObjectCategory<T>*>(it2->second.get());
				return category->objects;
			}
		}
		return ArrayRef<ObjectPtr<T>>();
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
