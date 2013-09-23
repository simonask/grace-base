//
//  resource.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_resource_hpp
#define grace_resource_hpp

#include "base/basic.hpp"
#include "object/object.hpp"

#include "base/string.hpp"

namespace grace {
	typedef String ResourceID;
	
	class ResourceLoaderBase;
	
	class Resource {
	public:
		Resource(IAllocator& alloc = default_allocator()) : refcount_(0), loader_(nullptr) {}
		virtual ~Resource() {}
		
		const ResourceID& resource_id() const { return id_; }
		void retain();
		void release();
		int64 refcount() { return refcount_; }
	private:
		ResourceID id_;
		int64 refcount_;
		ResourceLoaderBase* loader_;
		friend class ResourceManager;
	};
	
	struct IInputStream;
	
	class StreamingResource : public Resource {
	public:
		virtual IInputStream* create_reader(IAllocator& alloc) const;
	};
	
	inline void Resource::retain() {
		++refcount_;
	}
	
	inline void Resource::release() {
		--refcount_;
	}
}

#endif
