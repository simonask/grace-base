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
#include "io/fd.hpp"
#include "io/pipe_stream.hpp"

#include <stdio.h>
#include <unistd.h> // execvp()
#include <sys/wait.h> // waitpid()
#include <signal.h> // kill()
#include <errno.h>

namespace grace {
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
			raise<PipeError>("fork: {0}", ::strerror(errno));
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

	InputPipeStream& Process::stdout() {
		ASSERT(impl);
		return impl->stdout;
	}

	InputPipeStream& Process::stderr() {
		ASSERT(impl);
		return impl->stderr;
	}

	FormattedStream& Process::stdin() {
		ASSERT(impl);
		return impl->stdin;
	}

	FileDescriptor Process::stdout_fd() const {
		return impl->stdout.descriptor();
	}

	FileDescriptor Process::stderr_fd() const {
		return impl->stderr.descriptor();
	}

	FileDescriptor Process::stdin_fd() const {
		return impl->stdin_raw.descriptor();
	}
}