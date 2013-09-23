#include "base/process.hpp"
#include "io/input_stream.hpp"
#include "io/output_stream.hpp"
#include "io/formatted_stream.hpp"
#include "base/stack_array.hpp"
#include "base/array.hpp"
#include "base/string.hpp"
#include "base/error.hpp"
#include "base/raise.hpp"

#include "io/stdio_stream.hpp"

#include <stdio.h>
#include <unistd.h> // execvp
#include <sys/wait.h> // waitpid()
#include <signal.h> // kill()
#include <errno.h>

namespace grace {
	struct PipeError : ErrorBase<PipeError> {};

	struct PipeStreamBase {
		PipeStreamBase() {}
		PipeStreamBase(PipeStreamBase&& other) : fd(other.fd), position(other.position) {
			other.fd = -1;
			other.position = 0;
		}
		PipeStreamBase(int fd) : fd(fd) {}
		~PipeStreamBase() {
			close();
		}

		PipeStreamBase& operator=(PipeStreamBase&& other) {
			close();
			fd = other.fd;
			position = other.position;
			other.fd = -1;
			other.position = 0;
			return *this;
		}

		int fd = -1;
		size_t position = 0;

		void close() {
			if (fd >= 0) {
				::close(fd);
				fd = -1;
			}
		}
	};

	struct InputPipeStream : PipeStreamBase, InputStream {
		InputPipeStream(int fd) : PipeStreamBase(fd) {}
		InputPipeStream() = default;
		InputPipeStream(InputPipeStream&& other) = default;
		InputPipeStream& operator=(InputPipeStream&& other) = default;

		bool is_readable() const final { return true; }

		size_t read(byte* buffer, size_t max) final {
			ssize_t n = ::read(fd, buffer, max);
			if (n < 0) {
				raise<PipeError>("read: {0}", ::strerror(errno));
			} else {
				position += n;
			}
			return (size_t)n;
		}

		size_t read_if_available(byte* buffer, size_t max, bool& out_would_block) final {
			ssize_t n = ::read(fd, buffer, max);
			if (n < 0) {
				if (errno == EAGAIN) {
					out_would_block = true;
				} else {
					raise<PipeError>("read: {0}", ::strerror(errno));
				}
			} else {
				out_would_block = false;
				position += n;
			}
			return (size_t)n;
		}

		size_t tell_read() const final {
			return position;
		}

		bool seek_read(size_t) final { return false; }
		bool has_length() const final { return false; }
		size_t length() const final { return SIZE_T_MAX; }
	};

	struct OutputPipeStream : PipeStreamBase, OutputStream {
		OutputPipeStream(int fd) : PipeStreamBase(fd) {}
		OutputPipeStream() = default;
		OutputPipeStream(OutputPipeStream&& other) = default;
		OutputPipeStream& operator=(OutputPipeStream&& other) = default;

		bool is_writable() const final { return true; }

		size_t write(const byte* buffer, size_t max) final {
			ssize_t n = ::write(fd, buffer, max);
			if (n < 0) {
				raise<PipeError>("write: {0}", ::strerror(errno));
			} else {
				position += n;
			}
			return (size_t)n;
		}

		size_t write_if_available(const byte* buffer, size_t max, bool& out_would_block) final {
			ssize_t n = ::write(fd, buffer, max);
			if (n < 0) {
				if (errno == EAGAIN) {
					out_would_block = true;
				} else {
					raise<PipeError>("write: {0}", ::strerror(errno));
				}
			} else {
				out_would_block = false;
				position += n;
			}
			return (size_t)n;
		}

		size_t tell_write() const final {
			return position;
		}

		bool seek_write(size_t) final { return false; }
		void flush() {}
	};

	struct Process::Impl {
		int pid = -1;
		Process::Status status;
		int exitcode_or_signal = -1;
		OutputPipeStream stdin_raw;
		FormattedStream stdin;
		InputPipeStream stdout;
		InputPipeStream stderr;

		Impl() : stdin(stdin_raw) {}
	};

	Process Process::popen(StringRef exe, ArrayRef<StringRef> arguments) {
		Process p;

		int in[2];
		int out[2];
		int err[2];
		int pid;

		pipe(in);
		pipe(out);
		pipe(err);

		pid = fork();
		if (pid > 0) {
			// parent: set up pipes
			::close(in[0]);
			::close(out[1]);
			::close(err[1]);
			p.impl->pid = pid;
			p.impl->stdin_raw = OutputPipeStream(in[1]);
			p.impl->stdout = InputPipeStream(out[0]);
			p.impl->stderr = InputPipeStream(err[0]);
			p.impl->status = Status::Running;
		} else if (pid == 0) {
			// transform arguments into a format that execvp can understand
			COPY_STRING_REF_TO_CSTR_BUFFER(exe_cstr, exe);
			Array<String> args;
			Array<const char*> argv;
			args.reserve(arguments.size()+1);
			argv.reserve(arguments.size()+2);
			argv.push_back(exe_cstr.data());
			for (auto& arg: arguments) {
				args.emplace_back(String(arg) + '\0');
				argv.emplace_back(args.back().data());
			}
			argv.push_back(nullptr);

			// child: replace stdin/stdout/stderr
			::close(in[1]);
			::close(out[0]);
			::close(err[0]);

			::close(0);
			dup(in[0]);
			::close(1);
			dup(out[1]);
			::close(2);
			dup(err[1]);

			::execvp(exe_cstr.data(), (char* const*)argv.data());
			::perror("execvp");
			exit(1);
		} else {
			// error!
		}

		return std::move(p);
	}

	Process Process::popen(StringRef executable, std::initializer_list<StringRef> args) {
		DEFINE_STACK_ARRAY(StringRef, argv, args.size());
		auto it = args.begin();
		for (size_t i = 0; i < args.size(); ++i, ++it) {
			argv[i] = *it;
		}
		return popen(executable, argv);
	}

	Process::Process() {
		impl = new(default_allocator()) Impl;
	}

	Process::Process(Process&& other) : impl(other.impl) {
		other.impl = nullptr;
	}

	Process::~Process() {
		destroy(impl, default_allocator());
	}

	bool Process::get_status(bool hang) {
		ASSERT(impl);
		if (impl->pid < 0)
			return false;

		int status;
		int options = WUNTRACED;
		if (!hang)
			options |= WNOHANG;
		int rc = ::wait4(impl->pid, &status, options, nullptr);
		ASSERT(rc >= 0);

		if (WIFEXITED(status)) {
			impl->status = Status::Exited;
			impl->exitcode_or_signal = WEXITSTATUS(status);
			impl->pid = -1;
			return false;
		}
		if (WIFSIGNALED(status)) {
			impl->status = Status::Terminated;
			impl->exitcode_or_signal = WTERMSIG(status);
			impl->pid = -1;
			return false;
		}
		if (WIFSTOPPED(status)) {
			impl->status = Status::Stopped;
			impl->exitcode_or_signal = WSTOPSIG(status);
			impl->pid = -1;
			return false;
		}
		impl->status = Status::Running;
		return true; // it's still running
	}

	int Process::exit_status() {
		if (impl->status == Status::Exited) {
			return impl->exitcode_or_signal;
		}
		return -1;
	}

	int Process::termination_signal() {
		if (impl->status == Status::Terminated) {
			return impl->exitcode_or_signal;
		}
		return -1;
	}

	int Process::stop_signal() {
		if (impl->status == Status::Stopped) {
			return impl->exitcode_or_signal;
		}
		return -1;
	}

	void Process::wait() {
		ASSERT(impl);
		get_status(true);
	}

	void Process::close() {
		kill(1);
	}

	void Process::kill(int sig) {
		ASSERT(impl);
		if (impl->status == Status::Running) {
			::kill(impl->pid, sig);
			wait();
		}
	}

	void Process::kill() {
		kill(9);
	}

	InputStream& Process::stdout() {
		ASSERT(impl);
		return impl->stdout;
	}

	InputStream& Process::stderr() {
		ASSERT(impl);
		return impl->stderr;
	}

	FormattedStream& Process::stdin() {
		ASSERT(impl);
		return impl->stdin;
	}
}