#pragma once
#ifndef GRACE_PROCESS_HPP_INCLUDED
#define GRACE_PROCESS_HPP_INCLUDED

#include "base/string_ref.hpp"
#include "base/array_ref.hpp"
#include "memory/unique_ptr.hpp"
#include "base/maybe.hpp"
#include "io/fd.hpp"
#include "io/pipe_stream.hpp"

namespace grace {
	struct IInputStream;
	class FormattedStream;

	class Process {
	public:
		static Process popen(StringRef executable, ArrayRef<StringRef> args);
		static Process popen(StringRef executable, std::initializer_list<StringRef> args);

		enum class Status {
			Running,
			Exited,
			Terminated,
			Stopped,
		};

		Status status();
		int exit_status();
		int termination_signal();
		int stop_signal();


		void wait();
		void close();
		void kill(int sig);
		void kill();

		InputPipeStream& stdout();
		InputPipeStream& stderr();
		FormattedStream& stdin();

		FileDescriptor stdout_fd() const;
		FileDescriptor stderr_fd() const;
		FileDescriptor stdin_fd() const;

		~Process();
		Process(Process&& other);
	private:
		Process();
		struct Impl;
		Impl* impl = nullptr;

		bool get_status(bool hang);
	};
}

#endif
