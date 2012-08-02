//
//  stdio_stream.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_stdio_stream_hpp
#define falling_stdio_stream_hpp

#include "io/formatted_stream.hpp"
#include "io/file_stream.hpp"

namespace falling {
	enum StandardOutputStreamType {
		StandardOutput,
		StdOut = StandardOutput,
		StandardError,
		StdErr = StandardError,
		NumStandardOutputStreamTypes
	};
	
	enum StandardInputStreamType {
		StandardInput,
	};
	
	class StdOutputStream : public FormattedStream {
	public:
		StdOutputStream(StandardOutputStreamType stream_type = StandardOutput);
		StdOutputStream(StdOutputStream&& other) = default;
		StdOutputStream& operator=(StdOutputStream&& other) = default;
	private:
		OutputFileStream stream_;
	};
	
	StdOutputStream& get_stdout_stream(StandardOutputStreamType type);
	
	template <typename T>
	StdOutputStream& operator<<(StandardOutputStreamType type, const T& value) {
		StdOutputStream& stream = get_stdout_stream(type);
		stream << value;
		return stream;
	}
}

#endif
