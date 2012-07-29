//
//  resource_loader.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 28/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_resource_loader_hpp
#define falling_resource_loader_hpp

namespace falling {
	class ResourceLoaderBase {
	public:
		virtual Resource* allocate() = 0;
		virtual void free(Resource*) = 0;
		virtual bool load_resource(Resource* resource, std::istream& input) = 0;
	};
	
	template <typename T>
	class ResourceLoader : public ResourceLoaderBase {
	public:
		virtual ~ResourceLoader() {}
		Resource* allocate() override { return new T; }
		void free(Resource* resource) override { delete resource; }
		virtual bool load_resource(T& resource, std::istream& input) = 0;
	private:
		bool load_resource(Resource* resource, std::istream& input) override {
			return load_resource(*(T*)resource, input);
		}
	};
};

#endif
