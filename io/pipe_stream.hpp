#pragma once
#ifndef GRACE_PIPE_STREAM_HPP_INCLUDED
#define GRACE_PIPE_STREAM_HPP_INCLUDED

#include "io/fd.hpp"
#include "io/input_stream.hpp"
#include "io/output_stream.hpp"

namespace grace {
	struct PipeError : ErrorBase<PipeError> {};

	struct PipeStreamBase {
		~PipeStreamBase();
		bool is_nonblocking() const;
		void set_nonblocking(bool b);
		bool is_open() const;
		void close();
		FileDescriptor descriptor() const { return fd; }
	protected:
		PipeStreamBase() {}
		explicit PipeStreamBase(FileDescriptor fd) : fd(fd) {}
		PipeStreamBase(PipeStreamBase&& other);
		PipeStreamBase& operator=(PipeStreamBase&& other);
		void swap(PipeStreamBase& other);
		FileDescriptor fd = -1;
		size_t position = 0;
	};

	struct InputPipeStream : PipeStreamBase, IInputStream {
		explicit InputPipeStream(FileDescriptor fd) : PipeStreamBase(fd) {}
		InputPipeStream() {}
		virtual ~InputPipeStream() {}
		InputPipeStream(InputPipeStream&& other) = default;
		InputPipeStream& operator=(InputPipeStream&& other) = default;

		// IInputStream
		bool is_readable() const final { return is_open(); }
		bool is_read_nonblocking() const final { return is_nonblocking(); }
		Either<size_t, IOEvent> read(byte* buffer, size_t max) final;
		size_t tell_read() const final { return position; }
		bool seek_read(size_t) final { return false; }
		bool has_length() const final { return false; }
		size_t length() const final { return SIZE_T_MAX; }
	};

	struct OutputPipeStream : PipeStreamBase, IOutputStream {
		explicit OutputPipeStream(FileDescriptor fd) : PipeStreamBase(fd) {}
		OutputPipeStream() {}
		virtual ~OutputPipeStream() {}
		OutputPipeStream(OutputPipeStream&& other) = default;
		OutputPipeStream& operator=(OutputPipeStream&& other) = default;

		// IOutputStream
		bool is_writable() const final { return is_open(); }
		bool is_write_nonblocking() const final { return is_nonblocking(); }
		Either<size_t, IOEvent> write(const byte* buffer, size_t max) final;
		size_t tell_write() const final { return position; }
		bool seek_write(size_t) final { return false; }
		void flush() final {}
	};
}

#endif
