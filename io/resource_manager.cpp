//
//  resource_manager.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/resource_manager.hpp"
#include "base/log.hpp"
#include "io/resource_loader.hpp"
#include "base/fiber.hpp"
#include "io/archive.hpp"

#include "base/string.hpp"

#include "io/file_stream.hpp"

namespace grace {
	static const size_t RESOURCE_ARENA_SIZE = 0x2000000; // 32 Mb

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
		virtual void launch(Function<void()> f) {
			fibers_.push_back(make_unique<Fiber>(default_allocator(), *this, std::move(f), now()));
		}
		virtual void defer(Function<void()> f, GameTime until) { launch(f); }
		virtual Fiber* current_fiber() const { return current_; }
		
		Array<UniquePtr<Fiber>> fibers_;
		Fiber* current_ = nullptr;
	};
	
	struct ResourceManager::Impl {
		String resource_path;
		Array<UniquePtr<IArchive>> archives;
		Map<ResourceID, Resource*> resource_cache;
		Map<ResourceLoaderID, ResourceLoaderBase*> resource_loaders;
		bool is_in_resource_loader_fiber = false;
		ResourceLoaderFiberManager fiber_manager;
		LinearAllocator allocator;
		
		Impl() : allocator(RESOURCE_ARENA_SIZE) {}
	};
	
	ResourceManager::Impl& ResourceManager::impl() {
		static ResourceManager::Impl* p = new_static ResourceManager::Impl;
		return *p;
	}
	
	void ResourceManager::initialize_with_path(StringRef path_to_resources) {
		if (impl().resource_path != "") {
			Warning() << "Resource path already initialized!";
		}
		impl().resource_path = path_to_resources;
		add_archive(make_unique<PathArchive>(static_allocator(), path_to_resources));
	}
	
	Resource* ResourceManager::load_resource_in_fiber(ResourceLoaderID lid, ResourceID rid) {
		auto it = impl().resource_cache.find(rid);
		if (it != impl().resource_cache.end()) {
			return it->second;
		}
		
		if (impl().is_in_resource_loader_fiber) {
			impl().fiber_manager.launch([=]() {
				load_resource_impl(lid, rid);
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
				load_resource_impl(lid, rid);
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
	
	Resource* ResourceManager::load_resource_impl(ResourceLoaderID lid, ResourceID rid) {
		auto loader_it = impl().resource_loaders.find(lid);
		ResourceLoaderBase* loader = nullptr;
		if (loader_it == impl().resource_loaders.end()) {
			Error() << "No loader for resource ID: " << rid;
			return nullptr;
		}
		loader = loader_it->second;
		
		// TODO: Consider derived resources.
		
		String archive_debug_path;
		UniquePtr<IInputStream> is;
		for (auto& archive: impl().archives) {
			if (archive->contains(rid)) {
				archive_debug_path = archive->debug_path(rid);
				is = archive->open(rid);
				break;
			}
		}
		
		if (is) {
			Resource* resource = loader->allocate();
			resource->id_ = rid;
			if (loader->load_resource(resource, *is)) {
				impl().resource_cache[rid] = resource;
				Debug() << "Loaded resource: " << rid;
				return resource;
			}
			loader->free(resource);
		} else {
			Error() << "Could not open file: " << rid;
		}
		return nullptr;
	}
	
	String ResourceManager::path_for_resource(ResourceID rid) {
		if (impl().resource_path == "") {
			Warning() << "No resource path has been set.";
		}
		for (auto& archive: impl().archives) {
			if (archive->contains(rid)) {
				return archive->debug_path(rid);
			}
		}
		return rid;
	}
	
	void ResourceManager::add_loader(ResourceLoaderID lid, ResourceLoaderBase *loader) {
#if defined(DEBUG)
		auto it = impl().resource_loaders.find(lid);
		if (it != impl().resource_loaders.end()) {
			Warning() << "Loader already registered for this resource type. Overriding.";
		}
#endif
		impl().resource_loaders[lid] = loader;
	}
	
	void ResourceManager::add_archive(UniquePtr<IArchive> archive) {
		impl().archives.push_back(move(archive));
	}
	
	IInputStream* ResourceManager::create_reader_for_resource_id(IAllocator& alloc, ResourceID rid) {
		// TODO: Consider virtual resources?
		String path = path_for_resource(rid);
		FileStream of = FileStream::open(path, FileMode::Read);
		if (of.is_open() && of.is_readable()) {
			return new(alloc) FileStream(std::move(of));
		}
		Error() << "Could not create reader for resource id '" << rid << "' (path: " << path << ").";
		return nullptr;
	}
	
	void ResourceManager::garbage_collect() {
		bool any_leaked = false;
		for (auto it = impl().resource_cache.begin(); it != impl().resource_cache.end();) {
			if (it->second->refcount() == 0) {
				it->second->loader_->free(it->second);
				it = impl().resource_cache.erase(it);
			} else {
				if (it->second->refcount() < 0) {
					Error() << "Invalid resource refcount for resource: " << it->second->resource_id();
				} else {
					Error() << "Resource leaked: " << it->second->resource_id();
				}
				any_leaked = true;
				++it;
			}
		}
		if (!any_leaked) {
			impl().allocator.reset(impl().allocator.begin());
		}
	}
	
	void ResourceManager::clear() {
		for (auto pair: impl().resource_cache) {
			destroy(pair.second, allocator());
		}
		impl().resource_cache.clear();
		impl().resource_loaders.clear();
		impl().resource_path = "";
		impl().fiber_manager.fibers_.clear();
	}
	
	IAllocator& ResourceManager::allocator() {
		return impl().allocator;
	}
}