#include "platform/event_loop_libevent.hpp"
#include "io/file_stream.hpp"
#include "io/network_stream.hpp"
//#include "io/pipe_stream.hpp"

namespace grace {
	namespace {
		struct timeval system_time_delta_to_timeval(SystemTimeDelta delta) {
			struct timeval tv;
			int64 us = delta.microseconds();
			tv.tv_sec = us / 1000000;
			tv.tv_usec = us % 1000000;
			return tv;
		}

		struct ILibEventAdapter {
			virtual void invoke(int fd, short ev) = 0;
		};

		template <typename F>
		struct LibEventAdapter : ILibEventAdapter {
			F callback;
			LibEventAdapter(F callback) : callback(std::move(callback)) {}
			void invoke(int fd, short ev) final {
				callback(fd, ev);
			}
		};

		template <typename F>
		LibEventAdapter<F>* make_adapter(IAllocator& alloc, F callback) {
			return new(alloc) LibEventAdapter<F>(alloc, std::move(callback));
		}

		void adapter_callback(int fd, short ev, void* adapter) {
			((ILibEventAdapter*)adapter)->invoke(fd, ev);
		}

		struct LibEventHandle : IEventHandle {
			IAllocator& alloc;
			event* ev;
			SystemTimeDelta delay;
			LibEventHandle(IAllocator& alloc, event* ev, SystemTimeDelta delay) : alloc(alloc), event(ev), delay(delay) {}
			~LibEventHandle() {
				cancel();
				event_free(ev);
				ILibEventAdapter* adapter = (ILibEventAdapter*)event_get_callback_arg(event);
				destroy(adapter, alloc);
			}

			bool is_repeating() const final {
				return (event_get_events(event) & EV_PERSIST) != 0;
			}

			bool is_active() const final {
				return event_pending(event, EV_TIMEOUT|EV_READ|EV_WRITE|EV_SIGNAL, nullptr) != 0;
			}

			void activate() final {
				struct timeval tv = system_time_delta_to_timeval(interval);
				event_add(event, &tv);
			}

			void cancel() final {
				event_del(event);
			}
		};
	}

	EventLoop_libevent::EventLoop_libevent() : is_running_(true) {
		base_ = event_base_new();
	}

	EventLoop_libevent::~EventLoop_libevent() {
		event_base_free(base_);
	}

	UniquePtr<IEventHandle> EventLoop_libevent::schedule(Function<void()> callback, SystemTimeDelta delay, IAllocator& alloc) {
		auto f = [=](int fd, short ev) {
			callback();
		};
		event* ev = event_new(base, -1, EV_TIMEOUT, adapter_callback, make_adapter(move(f)));
		auto p = make_unique<LibEventHandle>(alloc, alloc, ev, delay);
		p->activate();
		return move(p);
	}

	UniquePtr<IEventHandle> EventLoop_libevent::call_repeatedly(Function<void()>, SystemTimeDelta interval, IAllocator& alloc) {
		auto f = [=](int fd, short ev) {
			callback();
		};
		event* ev = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, adapter_callback, make_adapter(move(f)));
		auto p = make_unique<LibEventHandle>(alloc, alloc, ev, interval);
		p->activate();
		return move(p);
	}

	UniquePtr<IEventHandle> EventLoop_libevent::add(InputFileStream& stream, Function<void(StreamEvent, InputFileStream& stream)> handler) {
		
	}

	UniquePtr<IEventHandle> EventLoop_libevent::add(OutputFileStream& stream, Function<void(StreamEvent, OutputFileStream& stream)> handler) {

	}

	UniquePtr<IEventHandle> EventLoop_libevent::add(NetworkStream& stream, Function<void(StreamEvent, NetworkStream& stream)> handler) {

	}

	UniquePtr<IEventHandle> EventLoop_libevent::add(ServerStream& stream, Function<void(StreamEvent, ServerStream& stream)> handler) {

	}

	UniquePtr<IEventHandle> EventLoop_libevent::add(PipeStream& stream, Function<void(StreamEvent, PipeStream& stream)> handler) {

	}

	UniquePtr<NetworkStream> EventLoop_libevent::connect(StringRef host, uint16 port, Function<void(StreamEvent, NetworkStream&)> callback) {

	}

	UniquePtr<ServerStream>  EventLoop_libevent::listen(uint16 port, Function<void(StreamEvent, ServerStream&)> callback) {

	}

	void EventLoop_libevent::run() {
		event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
	}

	void EventLoop_libevent::quit() {
		event_base_loopbreak(base);
	}
}