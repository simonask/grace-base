//
//  paths_apple.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 07/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "platform/paths.hpp"
#import <Foundation/Foundation.h>

namespace grace {
	String get_bundle_resource_path(IAllocator& alloc) {
		NSString* path = [[NSBundle mainBundle] resourcePath];
		String s(path.UTF8String, alloc);
		return move(s);
	}
	
	String get_executable_path(IAllocator& alloc) {
		NSString* path = [[[NSBundle mainBundle] executablePath] stringByDeletingLastPathComponent];
		String s(path.UTF8String, alloc);
		return move(s);
	}
}
