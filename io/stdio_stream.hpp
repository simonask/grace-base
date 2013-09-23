//
//  stdio_stream.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_stdio_stream_hpp
#define grace_stdio_stream_hpp

#include "io/formatted_stream.hpp"
#include "io/input_stream.hpp"

namespace grace {
	struct ConsoleStream : IInputStream, IInputStreamNonblocking, FormattedStream {
		ConsoleStream(const ConsoleStream&) = delete;
		ConsoleStream(ConsoleStream&&) = delete;
		ConsoleStream& operator=(const ConsoleStream&) = delete;
		ConsoleStream& operator=(ConsoleStream&&) = delete;
		static ConsoleStream& get();
		
		bool autoflush() const;
		void set_autoflush(bool b);

		// InputStream (stdin)
		bool is_readable() const final;
		size_t read(byte* buffer, size_t max) final;
		size_t tell_read() const final;
		bool seek_read(size_t position) final;
		bool has_length() const final;
		size_t length() const final;

		// IInputStreamNonblocking (stdin)
		size_t read_nonblocking(byte* buffer, size_t max, bool& out_would_block) final;
		bool is_read_nonblocking() const final;
		void set_read_nonblocking(bool) final;

		FormattedStream& stderr();
		
		struct Impl;
	private:
		ConsoleStream();
		Impl* impl = nullptr;
	};

	static ConsoleStream& Console = ConsoleStream::get();
	
	static FormattedStream& StdErr = ConsoleStream::get().stderr();
	static ConsoleStream& StdOut = ConsoleStream::get();
	static ConsoleStream& StdIn = ConsoleStream::get();
}

#endif
