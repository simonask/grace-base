#pragma once
#ifndef GRACE_BASE_PROCESS_STREAM_HPP
#define GRACE_BASE_PROCESS_STREAM_HPP

#include "io/input_stream.hpp"
#include "io/output_stream.hpp"
#include "base/string.hpp"

namespace grace {
	enum class ProcessStreamMode : uint8 {
		Read = 1,
		Write = 2,
		ReadWrite = 3,
	};

	class ProcessStream : public InputStream, public OutputStream {
	public:
		static ProcessStream open(StringRef cmd, ProcessStreamMode mode = ProcessStreamMode::ReadWrite);

		ProcessStream(ProcessStream&& other);
		ProcessStream& operator=(ProcessStream&& other);
		virtual ~ProcessStream();

		int wait();
		void close();

		// InputStream interface
		bool is_readable() const final;
		size_t read(byte* buffer, size_t max) final;
		size_t read_if_available(byte* buffer, size_t max, bool& would_block) final;
		size_t tell_read() const final;
		bool seek_read(size_t position) final;
		bool has_length() const final { return false; }
		size_t length() const final { return SIZE_MAX; }

		// OutputStream interface
		bool is_writable() const final;
		size_t write(const byte* buffer, size_t max) final;
		size_t write_if_available(const byte* buffer, size_t max, bool& would_block) final;
		size_t tell_write() const final;
		bool seek_write(size_t position) final;
		void flush() final;
	private:
		ProcessStream(void* os_fp, ProcessStreamMode mode) : fp_(os_fp), mode_(mode) {}
		void* fp_;
		ProcessStreamMode mode_;
		int exitcode_ = -1;
	};
}

#endif