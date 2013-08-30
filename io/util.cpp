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
#include "io/string_stream.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <glob.h>

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
	
#if defined(WIN32)
	static const char PathSeparator = '\\';
#else
	static const char PathSeparator = '/';
#endif
	
	String path_join(ArrayRef<const StringRef> components, IAllocator& alloc) {
		StringStream ss(alloc);
		for (size_t i = 0; i < components.size(); ++i) {
			ss << path_chomp(components[i]);
			if (i+1 != components.size()) {
				ss << PathSeparator;
			}
		}
		return ss.string(alloc);
	}
	
	StringRef path_chomp(StringRef path) {
		if (path.size() && path[path.size()-1] == PathSeparator) {
			return substr(path, 0, -1);
		}
		return path;
	}
	
	String path_absolute(StringRef relpath, IAllocator& alloc) {
		char buffer[PATH_MAX];
		COPY_STRING_REF_TO_CSTR_BUFFER(relpath_cstr, relpath);
		char* str = realpath(relpath_cstr.data(), buffer);
		return String(str, alloc);
	}

	Array<String> path_glob(StringRef pattern, IAllocator& alloc) {
		Array<String> result(alloc);
		COPY_STRING_REF_TO_CSTR_BUFFER(pattern_cstr, pattern);
		// TODO: Invoke in a thread-safe manner!
		glob_t g;
		int r = ::glob(pattern_cstr.data(), GLOB_MARK | GLOB_NOSORT, nullptr, &g);
		if (r == 0) {
			result.reserve(g.gl_matchc);
			for (size_t i = 0; i < g.gl_matchc; ++i) {
				result.push_back(String(g.gl_pathv[i], alloc));
			}
		}
		return move(result);
	}

	Maybe<SystemTime> file_modification_time(StringRef path) {
		struct stat s;
		COPY_STRING_REF_TO_CSTR_BUFFER(path_cstr, path);
		int r = stat(path_cstr.data(), &s);
		if (r == 0 && !S_ISDIR(s.st_mode)) {
			auto mtime = s.st_mtimespec;
			return SystemTime(0) + SystemTime::seconds((int64)mtime.tv_sec) + SystemTime::nanoseconds((int64)mtime.tv_nsec);
		}
		return Nothing;
	}
}
