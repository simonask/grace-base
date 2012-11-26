//
//  resource_loader.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 28/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_resource_loader_hpp
#define falling_resource_loader_hpp

#include "io/input_stream.hpp"
#include "io/resource.hpp"

namespace falling {
	
	class ResourceLoaderBase {
	public:
		IAllocator& allocator() const;
		virtual Resource* allocate() = 0;
		virtual void free(Resource*) = 0;
		virtual bool load_resource(Resource* resource, InputStream& input) = 0;
	};
	
	template <typename T>
	class ResourceLoader : public ResourceLoaderBase {
	public:
		virtual ~ResourceLoader() {}
		Resource* allocate() override { return new(allocator()) T(allocator()); }
		void free(Resource* resource) override { destroy(resource, allocator()); }
		virtual bool load(T& resource, InputStream& input) = 0;
	private:
		bool load_resource(Resource* resource, InputStream& input) override {
			return load(*(T*)resource, input);
		}
	};
};

#endif
