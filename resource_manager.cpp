//
//  resource_manager.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/resource_manager.hpp"
#include "base/log.hpp"
#include "io/resource_loader.hpp"
#include "base/fiber.hpp"

#include <string>

#include "io/file_stream.hpp"

namespace falling {
	struct ResourceLoaderFiberManager : IFiberManager {
		ResourceLoaderFiberManager() {}
		ResourceLoaderFiberManager(const ResourceLoaderFiberManager&) = delete;
		
		GameTime now() const { return GameTime(); }
		void update(GameTime) {
			IFiberManager::set_current_manager(this);
			
			while (fibers_.size() != 0) {
				size_t i = fibers_.size()-1;
				current_ = fibers_[i].get();
				fibers_[i]->resume();
				current_ = nullptr;
				if (fibers_[i]->state() == FiberState::Unstarted) {
					// Fiber finished, so pop it. Since it didn't start any new fibers, it should be the last one.
					fibers_.erase(i);
				}
			}
			
			IFiberManager::set_current_manager(nullptr);
		}
		virtual void set_alarm_clock(Fiber* fiber, GameTime at) {};
		virtual void launch(std::function<void()> f) {
			fibers_.push_back(make_unique<Fiber>(*this, std::move(f), now()));
		}
		virtual void defer(std::function<void()> f, GameTime until) { launch(f); }
		virtual Fiber* current_fiber() const { return current_; }
		
		Array<std::unique_ptr<Fiber>> fibers_;
		Fiber* current_ = nullptr;
	};
	
	struct ResourceManager::Impl {
		std::string resource_path;
		std::map<ResourceID, Resource*> resource_cache;
		std::map<std::string, ResourceLoaderBase*> resource_loaders;
		bool is_in_resource_loader_fiber = false;
		ResourceLoaderFiberManager fiber_manager;
	};
	
	ResourceManager::Impl& ResourceManager::impl() {
		static ResourceManager::Impl* p = new ResourceManager::Impl;
		return *p;
	}
	
	void ResourceManager::initialize_with_path(const std::string &path_to_resources) {
		if (impl().resource_path != "") {
			Warning() << "Resource path already initialized!";
		}
		impl().resource_path = path_to_resources;
		
		if (impl().resource_path.back() != '/') { // TODO: Win32?
			impl().resource_path += '/';
		}
	}
	
	Resource* ResourceManager::load_resource_in_fiber(ResourceID rid) {
		auto it = impl().resource_cache.find(rid);
		if (it != impl().resource_cache.end()) {
			return it->second;
		}
		
		if (impl().is_in_resource_loader_fiber) {
			impl().fiber_manager.launch([=]() {
				load_resource_impl(rid);
			});
			Fiber::yield();
			auto loaded = impl().resource_cache.find(rid);
			if (loaded != impl().resource_cache.end()) {
				return loaded->second;
			}
			Error() << "Dependency '" << rid << "' failed to load.";
			return nullptr;
		} else {
			impl().is_in_resource_loader_fiber = true;
			impl().fiber_manager.launch([=]() {
				load_resource_impl(rid);
			});
			impl().fiber_manager.update(GameTime());
			impl().is_in_resource_loader_fiber = false;
			auto loaded = impl().resource_cache.find(rid);
			if (loaded != impl().resource_cache.end()) {
				return loaded->second;
			}
			Error() << "Failed to load resource: " << rid;
			return nullptr;
		}
	}
	
	Resource* ResourceManager::load_resource_impl(ResourceID rid) {
		ResourceLoaderBase* loader = get_loader_for_resource_id(rid);
		if (loader == nullptr) {
			Error() << "No loader for resource ID: " << rid;
			return nullptr;
		}
		
		// TODO: Consider derived resources.
		
		std::string path = path_for_resource(rid);
		InputFileStream f = InputFileStream::open(path);
		if (f.is_open()) {
			Resource* resource = loader->allocate();
			resource->id_ = rid;
			if (loader->load_resource(resource, f)) {
				impl().resource_cache[rid] = resource;
				Debug() << "Loaded resource: " << rid;
				return resource;
			}
			loader->free(resource);
		} else {
			Error() << "Could not open file: " << path;
		}
		return nullptr;
	}
	
	std::string ResourceManager::path_for_resource(ResourceID rid) {
		if (impl().resource_path == "") {
			Warning() << "No resource path has been set.";
		}
		return impl().resource_path + rid;
	}
	
	ResourceLoaderBase* ResourceManager::get_loader_for_resource_id(ResourceID rid) {
		std::string::size_type dot_pos = rid.rfind('.');
		if (dot_pos == std::string::npos) {
			return nullptr;
		}
		std::string file_extension = rid.substr(dot_pos+1);
		auto it = impl().resource_loaders.find(file_extension);
		if (it != impl().resource_loaders.end()) {
			return it->second;
		}
		return nullptr;
	}
	
	void ResourceManager::add_loader(std::string file_extension, ResourceLoaderBase *loader) {
		if (impl().resource_loaders.find(file_extension) != impl().resource_loaders.end()) {
			Warning() << "Loader already registered for extension '" << file_extension << "', overriding...";
		}
		impl().resource_loaders[file_extension] = loader;
	}
	
	void ResourceManager::garbage_collect() {
		for (auto it = impl().resource_cache.begin(); it != impl().resource_cache.end();) {
			if (it->second->refcount() == 0) {
				it->second->loader_->free(it->second);
				it = impl().resource_cache.erase(it);
			} else {
				if (it->second->refcount() < 0) {
					Error() << "Invalid resource refcount for resource: " << it->second->resource_id();
				}
				++it;
			}
		}
	}
}