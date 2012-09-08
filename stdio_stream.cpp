//
//  stdio_stream.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/stdio_stream.hpp"
#include <stdio.h>

namespace falling {
	namespace {
		FILE* file_pointer_for_stream_type(StandardOStreamType type) {
			switch (type) {
				case StandardOutputStreamType: return stdout;
				case StandardErrorStreamType: return stderr;
				default: ASSERT(false); return nullptr;
			}
		}
		
		FILE* file_pointer_for_stream_type(StandardInputStreamType type) {
			return stdin;
		}
	}
	
	StdOutputStream& get_stdout_stream(StandardOStreamType type) {
		static StdOutputStream* stream[NumStandardOutputStreamTypes] = {nullptr};
		StdOutputStream*& ptr = stream[type];
		if (ptr == nullptr) {
			ptr = new StdOutputStream(type);
		}
		return *ptr;
	}
	
	StdOutputStream::StdOutputStream(StandardOStreamType type) : FormattedStream(stream_) {
		stream_ = OutputFileStream::wrap_file_pointer(file_pointer_for_stream_type(type));
		stream_.set_sync(true);
	}
}
