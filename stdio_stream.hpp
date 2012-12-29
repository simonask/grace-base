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
	enum StandardOStreamType {
		StandardOutputStreamType,
		StandardErrorStreamType,
		NumStandardOutputStreamTypes
	};
	
	enum StandardInputStreamType {
		StandardInput,
	};
	
	class StdOutputStream : public FormattedStream {
	public:
		StdOutputStream(StandardOStreamType stream_type = StandardOutputStreamType);
		StdOutputStream(StdOutputStream&& other) = default;
		StdOutputStream& operator=(StdOutputStream&& other) = default;
		
		FORWARD_TO_MEMBER(sync, stream_, OutputFileStream);
		FORWARD_TO_MEMBER(set_sync, stream_, OutputFileStream);
	private:
		OutputFileStream stream_;
	};
	
	StdOutputStream& get_stdout_stream(StandardOStreamType type);
	
	template <typename T>
	StdOutputStream& operator<<(StandardOStreamType type, const T& value) {
		StdOutputStream& stream = get_stdout_stream(type);
		stream << value;
		return stream;
	}
	
	template <StandardOStreamType t>
	struct GetOutputStream {
		GetOutputStream() {}
		
		StdOutputStream& stream() const {
			return get_stdout_stream(t);
		}
		
		operator StdOutputStream&() const {
			return stream();
		}
	};
	
	static const GetOutputStream<StandardOutputStreamType> StdOut;
	static const GetOutputStream<StandardOutputStreamType> StandardOutput;
	static const GetOutputStream<StandardErrorStreamType> StdErr;
	static const GetOutputStream<StandardErrorStreamType> StandardError;
}

#endif
