//
//  util.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 09/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "io/util.hpp"
#include "base/string.hpp"
#include "base/stack_array.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace grace {
	bool path_is_directory(StringRef path) {
		struct stat s;
		COPY_STRING_REF_TO_CSTR_BUFFER(path_cstr, path);
		int r = stat(path_cstr.data(), &s);
		if (r == 0)
			return S_ISDIR(s.st_mode);
		return false;
	}
	
	bool path_is_file(StringRef path) {
		struct stat s;
		COPY_STRING_REF_TO_CSTR_BUFFER(path_cstr, path);
		int r = stat(path_cstr.data(), &s);
		if (r == 0)
			return !S_ISDIR(s.st_mode);
		return false;
	}
	
	bool path_exists(StringRef path) {
		struct stat s;
		COPY_STRING_REF_TO_CSTR_BUFFER(path_cstr, path);
		int r = stat(path_cstr.data(), &s);
		return r == 0;
	}
}
