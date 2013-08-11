//
//  scene_loader.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 28/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_scene_loader_hpp
#define grace_scene_loader_hpp

#include "io/resource.hpp"
#include "io/resource_loader.hpp"
#include "object/object_template.hpp"

namespace grace {
	class ObjectTemplateLoader : public ResourceLoader<ObjectTemplate> {
		virtual bool load(ObjectTemplate& resource, InputStream& input) final;
	};
}

#endif
