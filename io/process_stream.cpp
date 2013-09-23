#include "io/process_stream.hpp"
#include "base/stack_array.hpp"

namespace grace {
		ProcessStream ProcessStream::open(StringRef cmd, ProcessStreamMode mode) {
			const char* m;
			switch (mode) {
				case ProcessStreamMode::Read: m = "r"; break;
				case ProcessStreamMode::Write: m = "w"; break;
				case ProcessStreamMode::ReadWrite: m = "r+"; break;
			}
			COPY_STRING_REF_TO_CSTR_BUFFER(cmd_buffer, cmd);
			FILE* fp = ::popen(cmd_buffer.data(), m);
			ProcessStream ps(fp, mode);
			if (fp == nullptr) {
				ps.exitcode_ = 127;
			}
			return move(ps);
		}

		ProcessStream::~ProcessStream() {
			close();
		}

		ProcessStream::ProcessStream(ProcessStream&& other) : fp_(other.fp_), mode_(other.mode_), exitcode_(other.exitcode_) {
			other.fp_ = nullptr;
			other.exitcode_ = -1;
		}

		ProcessStream& ProcessStream::operator=(ProcessStream&& other) {
			close();
			fp_ = other.fp_;
			mode_ = other.mode_;
			exitcode_ = other.exitcode_;
			other.fp_ = nullptr;
			other.exitcode_ = -1;
			return *this;
		}

		void ProcessStream::close() {
			if (fp_) {
				exitcode_ = ::pclose((FILE*)fp_);
				fp_ = nullptr;
			}
		}

		int ProcessStream::wait() {
			close();
			return exitcode_;
		}

		// InputStream interface
		bool ProcessStream::is_readable() const {
			return mode_ == ProcessStreamMode::Read || mode_ == ProcessStreamMode::ReadWrite;
		}

		size_t ProcessStream::read(byte* buffer, size_t max) {
			ASSERT(is_readable());
			if (fp_) {
				return ::fread((void*)buffer, max, 1, (FILE*)fp_);
			}
			return 0;
		}
		
		size_t ProcessStream::read_if_available(byte* buffer, size_t max, bool &would_block) {
			ASSERT(is_readable());
			would_block = true;
			return 0;
		}

		size_t ProcessStream::tell_read() const {
			if (fp_) {
				return ::ftell((FILE*)fp_);
			}
			return 0;
		}

		bool ProcessStream::seek_read(size_t position) {
			return false;
		}

		// OutputStream interface
		bool ProcessStream::is_writable() const {
			return mode_ == ProcessStreamMode::Write || mode_ == ProcessStreamMode::ReadWrite;
		}

		size_t ProcessStream::write(const byte* buffer, size_t max) {
			ASSERT(is_writable());
			if (fp_) {
				return ::fwrite((const void*)buffer, max, 1, (FILE*)fp_);
			}
			return 0;
		}
		
		size_t ProcessStream::write_if_available(const byte *buffer, size_t max, bool &would_block) {
			ASSERT(is_writable());
			would_block = true;
			return 0;
		}

		size_t ProcessStream::tell_write() const {
			return 0;
		}

		bool ProcessStream::seek_write(size_t position) {
			return false;
		}

		void ProcessStream::flush() {
			if (fp_) {
				::fflush((FILE*)fp_);
			}
		}
}