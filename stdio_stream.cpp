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
		FILE* file_pointer_for_stream_type(StandardOutputStreamType type) {
			switch (type) {
				case StandardOutput: return stdout;
				case StandardError: return stderr;
			}
		}
		
		FILE* file_pointer_for_stream_type(StandardInputStreamType type) {
			return stdin;
		}
	}
	
	StdOutputStream::StdOutputStream(StandardOutputStreamType type) : FormattedStream(stream_) {
		stream_ = OutputFileStream::wrap_file_pointer(file_pointer_for_stream_type(type));
		stream_.set_sync(true);
	}
}
