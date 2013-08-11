//
//  grace_base.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 08/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "base/basic.hpp"
#include "io/resource_manager.hpp"
#include "type/type_registry.hpp"
#include "object/object_template.hpp"
#include "loaders/object_template_loader.hpp"
#include "base/config.hpp"

using namespace grace;

BEGIN_CONFIG_SCOPE(Debug)
	option<bool>("show_fps", "Whether or not to show FPS counter.");
	option<bool>("print_memory_usage", "Whether or not to print memory usage to the log.");
	option<bool>("track_memory_leaks", "Whether or not to keep track of memory allocations, and display them when printing memory usage.");
	option<float64>("print_memory_usage_interval", "Number of seconds between printing memory usage");
END_CONFIG_SCOPE()

GRACE_CAPI void grace_base_init() {
	ResourceManager::add_loader<ObjectTemplate, ObjectTemplateLoader>();
	
	REGISTER_CONFIG_SCOPE(Debug);
}
