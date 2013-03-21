//
//  resource_manager.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_resource_manager_hpp
#define falling_resource_manager_hpp

#include "base/basic.hpp"
#include "base/log.hpp"
#include "io/resource.hpp"
#include "io/resource_ptr.hpp"
#include "io/resource_loader.hpp"

namespace falling {
	typedef uintptr_t ResourceLoaderID;
	
	template <typename ResourceType>
	struct ResourceLoaderIDHolder {
	};
	
	template <typename ResourceType>
	__attribute__((noinline)) DLL_PUBLIC ResourceLoaderID get_loader_id_for_type() {
		static ResourceLoaderIDHolder<ResourceType>* holder = new_static ResourceLoaderIDHolder<ResourceType>;
		return reinterpret_cast<ResourceLoaderID>(holder);
	}
	
	class ResourceManager {
	public:
		static void initialize_with_path(StringRef path_to_resources);
		static IAllocator& allocator();
		
		template <typename T>
		static ResourcePtr<T> load_resource(ResourceID rid);
		
		static ResourceLoaderBase* get_loader_for_resource_id(ResourceID rid);
		static String path_for_resource(ResourceID rid);
		static InputStream* create_reader_for_resource_id(IAllocator& alloc, ResourceID rid);
		
		template <typename ResourceType, typename ResourceLoaderType>
		static void add_loader() {
			ResourceLoaderID lid = get_loader_id_for_type<ResourceType>();
			add_loader(lid, new_static ResourceLoaderType);
		}
		
		static void garbage_collect();
		static void clear();
	private:
		struct Impl;
		static Impl& impl();
		
		static Resource* load_resource_in_fiber(ResourceLoaderID lid, ResourceID rid);
		static Resource* load_resource_impl(ResourceLoaderID lid, ResourceID rid);
		static void add_loader(ResourceLoaderID lid, ResourceLoaderBase* loader);
	};
	
	template <typename T>
	ResourcePtr<T> load_resource(ResourceID rid) {
		return ResourceManager::load_resource<T>(rid);
	}
	
	template <typename T>
	ResourcePtr<T> ResourceManager::load_resource(ResourceID rid) {
		ResourceLoaderID lid = get_loader_id_for_type<T>();
		Resource* resource = load_resource_in_fiber(lid, rid);
		if (resource != nullptr) {
			T* typed = dynamic_cast<T*>(resource);
			if (typed != nullptr) {
				return ResourcePtr<T>(typed);
			} else {
				Error() << "ResourceManager: Loaded resource is not of the correct type (" << rid << ").";
			}
		}
		return nullptr;
	}
}

#endif
