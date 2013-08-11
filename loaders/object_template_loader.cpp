//
//  scene_loader.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 23/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "loaders/object_template_loader.hpp"
#include "base/log.hpp"
#include "serialization/yaml.hpp"

namespace grace {
	bool ObjectTemplateLoader::load(ObjectTemplate& resource, InputStream& input) {
		String error;
		// TODO: Get rid of reinterpret_cast!
		YAML yaml;
		if (yaml.read(resource.document, input, error) == 0) {
			Error() << "YAML: " << error;
			return false;
		}
		return true;
	}
}