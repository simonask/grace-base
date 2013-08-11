//
//  stdio_stream.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/stdio_stream.hpp"
#include <stdio.h>

namespace grace {
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
	
	static byte stdout_stream_memory[sizeof(StdOutputStream)];
	static byte stderr_stream_memory[sizeof(StdOutputStream)];
	
	StdOutputStream& get_stdout_stream(StandardOStreamType type) {
		static StdOutputStream* stdout_ = nullptr;
		static StdOutputStream* stderr_ = nullptr;
		switch (type) {
			case StandardOutputStreamType: {
				if (stdout_ == nullptr) {
					stdout_ = new(stdout_stream_memory) StdOutputStream(type);
				}
				return *stdout_;
			}
			case StandardErrorStreamType: {
				if (stderr_ == nullptr) {
					stderr_ = new(stderr_stream_memory) StdOutputStream(type);
				}
				return *stderr_;
			}
			default: UNREACHABLE();
		}
	}
	
	StdOutputStream::StdOutputStream(StandardOStreamType type) : FormattedStream(stream_) {
		stream_ = OutputFileStream::wrap_file_pointer(file_pointer_for_stream_type(type));
		stream_.set_sync(true);
	}
}
