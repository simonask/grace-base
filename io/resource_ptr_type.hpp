//
//  resource_ptr_type.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_resource_ptr_type_hpp
#define grace_resource_ptr_type_hpp

#include "type/type.hpp"
#include "io/resource_ptr.hpp"
#include "io/resource_manager.hpp"

namespace grace {
	class ResourcePtrType : public Type {
	public:
		ResourcePtrType(const TypeInfo& ti) : Type(ti) {}
		StringRef name() const { return "ResourcePtr"; }
	private:
		String name_;
	};
	
	template <typename T>
	class ResourcePtrTypeImpl : public TypeFor<ResourcePtr<T>, ResourcePtrType> {
	public:
		virtual void deserialize(ResourcePtr<T>& place, const DocumentNode& node, IUniverse&) const final;
		virtual void serialize(const ResourcePtr<T>& place, DocumentNode& node, IUniverse&) const final;
	};
	
	template <typename T>
	struct BuildTypeInfo<ResourcePtr<T>> {
		static const ResourcePtrType* build() {
			static const ResourcePtrTypeImpl<T>* type = new_static ResourcePtrTypeImpl<T>();
			return type;
		}
	};
	
	template <typename T>
	void ResourcePtrTypeImpl<T>::deserialize(ResourcePtr<T>& place, const DocumentNode& node, IUniverse&) const {
		ResourceID rid;
		if (node >> rid) {
			place = load_resource<T>(rid);
		}
	}
	
	template <typename T>
	void ResourcePtrTypeImpl<T>::serialize(const ResourcePtr<T>& place, DocumentNode& node, IUniverse&) const {
		if (place != nullptr) {
			node << place->resource_id();
		} else {
			node.clear();
		}
	}
}

#endif
