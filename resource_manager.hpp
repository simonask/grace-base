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
	class ResourceManager {
	public:
		static void initialize_with_path(const std::string& path_to_resources);
		
		template <typename T>
		static ResourcePtr<T> load_resource(ResourceID rid);
		
		static ResourceLoaderBase* get_loader_for_resource_id(ResourceID rid);
		static std::string path_for_resource(ResourceID rid);
		
		template <typename T>
		static void add_loader(std::string file_extension) {
			add_loader(std::move(file_extension), new T);
		}
		
		static void garbage_collect();
	private:
		struct Impl;
		static Impl& impl();
		
		static Resource* load_resource_raw(ResourceID rid);
		static void add_loader(std::string file_extension, ResourceLoaderBase* loader);
	};
	
	template <typename T>
	ResourcePtr<T> load_resource(ResourceID rid) {
		return ResourceManager::load_resource<T>(rid);
	}
	
	template <typename T>
	ResourcePtr<T> ResourceManager::load_resource(ResourceID rid) {
		Resource* resource = load_resource_raw(rid);
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
