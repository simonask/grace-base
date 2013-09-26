#include "platform/event_loop_libevent.hpp"
#include "io/file_stream.hpp"
#include "io/network_stream.hpp"
#include "io/reactor.hpp"
#include "io/fd.hpp"
#include "io/stdio_stream.hpp"
#include "base/string.hpp"
#include "base/string_ref.hpp"
#include "base/array.hpp"
#include "base/process.hpp"

#include <errno.h>
#include <stdio.h>

namespace grace {
	namespace {
		using namespace ::grace;
		struct timeval system_time_delta_to_timeval(SystemTimeDelta delta) {
			struct timeval tv;
			int64 us = delta.microseconds();
			tv.tv_sec = us / 1000000;
			tv.tv_usec = us % 1000000;
			return tv;
		}

		struct LibEventHandle : IEventHandle {
			SystemTimeDelta timeout = SystemTimeDelta::forever();
			void set_timeout(SystemTimeDelta t) final {
				timeout = t;
				if (is_active()) {
					activate();
				}
			}
			virtual ~LibEventHandle() {}
		};

		struct LibEventTimer : LibEventHandle {
			event* ev;
			Function<void()> callback;
			virtual ~LibEventTimer() {
				cancel();
				event_free(ev);
			}

			void invoke() {
				callback();
			}

			bool is_repeating() const final {
				return (event_get_events(ev) & EV_PERSIST) != 0;
			}

			bool is_active() const final {
				return event_pending(ev, EV_TIMEOUT|EV_READ|EV_WRITE|EV_SIGNAL, nullptr) != 0;
			}

			void activate() final {
				struct timeval tv = system_time_delta_to_timeval(timeout);
				event_add(ev, &tv);
			}

			void cancel() final {
				event_del(ev);
			}
		};

		void timer_callback(int fd, short ev, void* timer) {
			((LibEventTimer*)timer)->invoke();
		}

		struct LibEventFileDescriptor : LibEventHandle {
			event* ev;
			virtual ~LibEventFileDescriptor() {
				cancel();
				int fd = event_get_fd(ev);
				if (fd >= 0)
					::close(fd);
				event_free(ev);
			}

			virtual void invoke(int fd, short ev) = 0;

			bool is_repeating() const final {
				return true;
			}

			bool is_active() const final {
				return event_pending(ev, EV_TIMEOUT|EV_READ|EV_WRITE|EV_SIGNAL, nullptr) != 0;
			}

			void activate() final {
				struct timeval tv = system_time_delta_to_timeval(timeout);
				event_add(ev, &tv);
			}

			void cancel() final {
				event_del(ev);
			}
		};

		void file_descriptor_callback(int fd, short ev, void* handler) {
			((LibEventFileDescriptor*)handler)->invoke(fd, ev);
		}

		struct LibEventStdInHandle : LibEventFileDescriptor {
			Function<void(StdInEvent, ConsoleStream&)> callback;

			void invoke(int fd, short ev) final {
				switch (ev) {
					case EV_READ:    callback(StdInEvent::Read, Console); return;
					case EV_TIMEOUT: callback(StdInEvent::Timeout, Console); return;
					default: ASSERT(false);
				}
			}
		};

		void process_callback(int fd, short ev, void* handler);

		struct ProcessHandle : LibEventHandle {
			event_base* base_ = nullptr;
			String command;
			Array<String> arguments;
			UniquePtr<Process> process;
			Function<void(ProcessEvent, Process&)> callback;
			event* stdout_ev = nullptr;
			event* stderr_ev = nullptr;

			virtual ~ProcessHandle() {
				cancel();
			}

			bool is_active() const final {
				return process != nullptr;
			}

			bool is_repeating() const final {
				return true;
			}

			void activate() final {
				if (!process) {
					ScratchAllocator scratch;
					Array<StringRef> args(scratch);
					args.reserve(arguments.size());
					for (auto& s: arguments) {
						args.push_back(s);
					}
					process = make_unique<Process>(default_allocator(), Process::popen(command, args));
					
					set_nonblocking(process->stdout_fd(), true);
					stdout_ev = event_new(base_, process->stdout_fd(), EV_READ|EV_TIMEOUT|EV_PERSIST, process_callback, this);
					
					set_nonblocking(process->stderr_fd(), true);
					stderr_ev = event_new(base_, process->stderr_fd(), EV_READ|EV_TIMEOUT|EV_PERSIST, process_callback, this);

					struct timeval tv = system_time_delta_to_timeval(timeout);
					event_add(stdout_ev, &tv);
					event_add(stderr_ev, &tv);
					
					callback(ProcessEvent::Ready, *process);
				}
			}

			void cancel() final {
				if (process) {
					process->close();
					process = nullptr;
				}

				if (stdout_ev) {
					event_del(stdout_ev);
					event_free(stdout_ev);
					stdout_ev = nullptr;
				}

				if (stderr_ev) {
					event_del(stderr_ev);
					event_free(stderr_ev);
					stderr_ev = nullptr;
				}
			}

			void invoke(int fd, short ev) {
				if (ev == EV_READ) {
					ProcessEvent pev;
					if (fd == process->stdout_fd()) {
						callback(ProcessEvent::StdOut, *process);
						if (!process->stdout().is_open()) {
							callback(ProcessEvent::Closed, *process);
							cancel();
						}
					} else if (fd == process->stderr_fd()) {
						callback(ProcessEvent::StdErr, *process);
						if (!process->stderr().is_open()) {
							callback(ProcessEvent::Closed, *process);
							cancel();
						}
					} else {
						ASSERT(false); // Invalid fd/ev combo
					}
					callback(pev, *process);
				} else if (ev == EV_TIMEOUT) {
					callback(ProcessEvent::Timeout, *process);
				} else if (ev == EV_SIGNAL) {
					//callback(ProcessEvent::Closed, *process);
					//cancel(); // TODO: Consider this.
				}
			}
		};

		void process_callback(int fd, short ev, void* handler) {
			((ProcessHandle*)handler)->invoke(fd, ev);
		}
	}

	EventLoop_libevent::EventLoop_libevent() {
		base_ = event_base_new();
	}

	EventLoop_libevent::~EventLoop_libevent() {
		event_base_free(base_);
	}

	UniquePtr<IEventHandle> EventLoop_libevent::schedule(Function<void()> callback, SystemTimeDelta delay, IAllocator& alloc) {
		auto p = make_unique<LibEventTimer>(alloc);
		auto ev = event_new(base_, -1, EV_TIMEOUT, timer_callback, p.get());
		p->ev = ev;
		p->timeout = delay;
		p->callback = std::move(callback);
		p->activate();
		return std::move(p);
	}

	UniquePtr<IEventHandle> EventLoop_libevent::call_repeatedly(Function<void()> callback, SystemTimeDelta interval, IAllocator& alloc) {
		auto p = make_unique<LibEventTimer>(alloc);
		auto ev = event_new(base_, -1, EV_TIMEOUT | EV_PERSIST, timer_callback, p.get());
		p->ev = ev;
		p->timeout = interval;
		p->callback = std::move(callback);
		p->activate();
		return std::move(p);
	}

	UniquePtr<IEventHandle> EventLoop_libevent::connect(StringRef host, uint16 port, Function<void(NetworkConnectionEvent, INetworkStream&)> callback, SystemTimeDelta timeout) {
		ASSERT(false); // NIY
	}

	UniquePtr<IEventHandle> EventLoop_libevent::listen(uint16 port, Function<void(ServerEvent, Server&)> callback) {
		ASSERT(false); // NIY
	}

	UniquePtr<IEventHandle> EventLoop_libevent::popen(StringRef command, ArrayRef<StringRef> arguments, Function<void(ProcessEvent, Process&)> callback, SystemTimeDelta timeout) {
		auto p = make_unique<ProcessHandle>(default_allocator());
		p->timeout = timeout;
		p->base_ = base_;
		p->command = command;
		p->arguments.reserve(arguments.size());
		for (auto& s: arguments) {
			p->arguments.emplace_back(s);
		}
		p->callback = std::move(callback);
		p->activate();
		return std::move(p);
	}

	UniquePtr<IEventHandle> EventLoop_libevent::stdin(Function<void(StdInEvent, ConsoleStream&)> callback, SystemTimeDelta timeout) {
		auto p = make_unique<LibEventStdInHandle>(default_allocator());
		auto ev = event_new(base_, fileno(::stdin), EV_READ, file_descriptor_callback, p.get());
		p->ev = ev;
		p->timeout = timeout;
		p->callback = std::move(callback);
		p->activate();
		return std::move(p);
	}

	void EventLoop_libevent::run() {
		while (is_running_) {
			event_base_loop(base_, EVLOOP_ONCE);
		}
	}

	void EventLoop_libevent::quit() {
		is_running_ = false;
		event_base_loopbreak(base_);
	}
}