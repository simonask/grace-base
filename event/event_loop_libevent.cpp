//
//  libevent_adapter.cpp
//  editor
//
//  Created by Simon Ask Ulsnes on 20/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "event/event_loop_libevent.hpp"

#include <event2/event.h>
#include <event2/util.h>
#include <event2/event_struct.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <errno.h>

#include "base/map.hpp"
#include "base/array.hpp"
#include "base/array_list.hpp"
#include "base/stack_array.hpp"
#include "event/evented_socket.hpp"

namespace grace {
	namespace {
		struct timeval system_time_delta_to_timeval(SystemTimeDelta delta) {
			struct timeval tv;
			int64 us = delta.microseconds();
			tv.tv_sec = us / 1000000;
			tv.tv_usec = us % 1000000;
			return tv;
		}
		
		struct ILibEventCallbackAdapter {
			virtual void invoke(int fd, short ev) const = 0;
			virtual ~ILibEventCallbackAdapter() {}
		};
		
		struct ILibEventBufferEventCallbackAdapter {
			virtual void invoke(bufferevent* ev, short events) const = 0;
			virtual void invoke_data(bufferevent* ev) const = 0;
			virtual ~ILibEventBufferEventCallbackAdapter() {}
		};
		
		struct EventLoop_libeventHandle : IEventHandle {
			event* ev;
			SystemTimeDelta interval;
			
			EventLoop_libeventHandle(event* ev, SystemTimeDelta interval) : ev(ev), interval(interval) {}
			
			bool is_active() const {
				return event_pending(ev, EV_TIMEOUT|EV_READ|EV_WRITE|EV_SIGNAL, nullptr) != 0;
			}
			
			bool is_repeating() const {
				return (event_get_events(ev) & EV_PERSIST) != 0;
			}
		
			void activate() final {
				struct timeval tv = system_time_delta_to_timeval(interval);
				event_add(ev, &tv);
			}
		
			void cancel() final {
				event_del(ev);
			}
		
			~EventLoop_libeventHandle() {
				cancel();
				auto p = (ILibEventCallbackAdapter*)event_get_callback_arg(ev);
				delete p;
				event_free(ev);
				ev = nullptr;
			}
		};
		
		template <typename T>
		struct LibEventCallbackAdapter : ILibEventCallbackAdapter {
			T f;
			LibEventCallbackAdapter(T f) : f(move(f)) {}
			void invoke(int fd, short ev) const final {
				f(fd, ev);
			}
		};
		
		struct LibEventBufferedSocket : EventedSocketBase {
			bufferevent* ev_;
			ConnectionEstablishedCallback on_connect_;
			ConnectionErrorCallback on_error_;
			DataCallback on_data_;
			ConnectionClosedCallback on_close_;
			
			LibEventBufferedSocket(bufferevent* ev) : ev_(ev) {}
			~LibEventBufferedSocket() {
				close();
				bufferevent_free(ev_);
			}
			
			// IEventedSocket interface
			void on_data(uint8 event_mask, DataCallback callback) final {
				on_data_ = move(callback);
			}
			void on_error(ConnectionErrorCallback callback) final {
				on_error_ = move(callback);
			}
			void on_connect(ConnectionEstablishedCallback callback) final {
				on_connect_ = move(callback);
			}
			void on_close(ConnectionClosedCallback callback) final {
				on_close_ = move(callback);
			}
			size_t data_available() const final {
				evbuffer* input = bufferevent_get_input(ev_);
				return evbuffer_get_length(input);
			}
			void send_data(ArrayRef<byte> data) final {
				bufferevent_write(ev_, data.data(), data.size());
			}
			void close() final {
				cancel();
				int fd = bufferevent_getfd(ev_);
				::close(fd);
				bufferevent_setfd(ev_, -1);
			}
			
			// InputStream interface
			size_t read(byte* buffer, size_t max) {
				evbuffer* input = bufferevent_get_input(ev_);
				size_t r = evbuffer_copyout(input, buffer, max);
				evbuffer_drain(input, r);
				return r;
			}
			
			// IEventHandle interface
			bool is_repeating() const final { return true; }
			bool is_active() const final {
				return bufferevent_get_enabled(ev_);
			}
			void activate() final {
				int fd = bufferevent_getfd(ev_);
				if (fd < 0) {
					if (on_error_) on_error_("Invalid socket", true);
				} else {
					int r = bufferevent_enable(ev_, EV_READ|EV_WRITE);
					ASSERT(r == 0);
				}
			}
			void cancel() final {
				bufferevent_disable(ev_, EV_READ|EV_WRITE);
			}

			// Private API
			void handle(bufferevent* ev, short events) {
				if (events & BEV_EVENT_CONNECTED) {
					if (on_connect_) on_connect_();
					handle(ev); // if there is data already, push it right away!
				}
				if (events & BEV_EVENT_EOF) {
					if (on_close_) on_close_();
				}
				if (events & BEV_EVENT_ERROR) {
					auto err = EVUTIL_SOCKET_ERROR();
					DEFINE_STACK_ARRAY(char, error_string, 1024);
					strerror_r(err, error_string.data(), error_string.size());
					StringRef err_msg = error_string.data();
					if (on_error_) on_error_(err_msg, true);
				}
			}
			void handle(bufferevent* ev) {
				if (data_available() > 0 && on_data_) {
					on_data_(*this, SocketEvent::Read);
				}
			}
		};
		
		template <typename F>
		LibEventCallbackAdapter<F>* make_adapter(F f) {
			return new LibEventCallbackAdapter<F>(move(f));
		}
		
		void libevent_callback(int fd, short ev, void* arg) {
			auto adapter = (ILibEventCallbackAdapter*)arg;
			adapter->invoke(fd, ev);
		}
		
		void libevent_bufferevent_data_callback(bufferevent* ev, void* arg) {
			auto adapter = (LibEventBufferedSocket*)arg;
			adapter->handle(ev);
		}
		
		void libevent_bufferevent_callback(bufferevent* ev, short events, void* arg) {
			auto adapter = (LibEventBufferedSocket*)arg;
			adapter->handle(ev, events);
		}
	}

	struct EventLoop_libevent::Impl {
		event_base* loop;
		Map<FileSystemDescriptor, event*> fd_events;
	};

	EventLoop_libevent::EventLoop_libevent() {
		impl = new(default_allocator()) Impl;
		impl->loop = event_base_new();
	}
	
	EventLoop_libevent::~EventLoop_libevent() {
		event_base_free(impl->loop);
		destroy(impl, default_allocator());
	}
	
	void EventLoop_libevent::run() {
		while (true) {
			event_base_loop(impl->loop, EVLOOP_ONCE);
		}
	}

	// Call-later API
	UniquePtr<IEventHandle> EventLoop_libevent::schedule(IAllocator& allocator, Function<void()> f, SystemTimeDelta delay) {
		auto f2 = [=](int fd, short ev) {
			f();
		};
		event* ev = event_new(impl->loop, -1, EV_TIMEOUT, libevent_callback, make_adapter(move(f2)));
		auto p = make_unique<EventLoop_libeventHandle>(allocator, ev, delay);
		p->activate();
		return move(p);
	}
	
	UniquePtr<IEventHandle> EventLoop_libevent::call_repeatedly(IAllocator& allocator, Function<void ()> f, SystemTimeDelta interval) {
		auto f2 = [=](int fd, short ev) {
			f();
		};
		event* ev = event_new(impl->loop, -1, EV_TIMEOUT | EV_PERSIST, libevent_callback, make_adapter(move(f2)));
		auto p = make_unique<EventLoop_libeventHandle>(allocator, ev, interval);
		p->activate();
		return move(p);
	}
	
	// Async File I/O API
	UniquePtr<IEventHandle> EventLoop_libevent::watch_descriptor(IAllocator& allocator, FileSystemDescriptor fd, uint8 event_mask, FileSystemCallback callback, SystemTimeDelta timeout) {
		short ev_mask = 0;
		if (event_mask & (uint8)FileSystemEvent::Read) ev_mask |= EV_READ;
		if (event_mask & (uint8)FileSystemEvent::Write) ev_mask |= EV_WRITE;
		if (event_mask & (uint8)FileSystemEvent::Persistent) ev_mask |= EV_PERSIST;
		if (event_mask & (uint8)FileSystemEvent::Timeout) ev_mask |= EV_TIMEOUT;
		ASSERT(ev_mask != 0);
		
		auto fd_callback = [=](int in_fd, short in_ev) {
			FileSystemEvent e;
			switch (in_ev) {
				case EV_READ: e = FileSystemEvent::Read; break;
				case EV_WRITE: e = FileSystemEvent::Write; break;
				case EV_TIMEOUT: { e = FileSystemEvent::Timeout; break; }
				default: ASSERT(false); // invalid event from libevent?
			}
			callback(in_fd, e);
		};
		event* ev = event_new(impl->loop, fd, ev_mask, libevent_callback, make_adapter(move(fd_callback)));
		auto p = make_unique<EventLoop_libeventHandle>(allocator, ev, timeout);
		p->activate();
		return move(p);
	}
	
	UniquePtr<IEventedSocket> EventLoop_libevent::connect(IAllocator& alloc, StringRef host, uint32 port, ConnectionCallback on_connect, ConnectionErrorCallback on_error) {
		bufferevent* ev = bufferevent_socket_new(impl->loop, -1, 0);
		COPY_STRING_REF_TO_CSTR_BUFFER(hostname_cstr, host);
		// TODO: Don't block on DNS name resolution
		int err = bufferevent_socket_connect_hostname(ev, nullptr, AF_UNSPEC, hostname_cstr.data(), port);
		if (err != 0) {
			const char* msg = ::strerror(errno);
			on_error(msg, true);
			return nullptr;
		}
		
		auto p = make_unique<LibEventBufferedSocket>(alloc, ev);
		auto raw = p.get();
		auto conn_established = [on_connect,raw]() {
			on_connect(*raw);
		};
		p->on_connect(move(conn_established));
		p->on_error(move(on_error));
		bufferevent_setcb(ev, libevent_bufferevent_data_callback, nullptr, libevent_bufferevent_callback, raw);
		p->activate();
		return move(p);
	}
	
	namespace {
		struct LibEventListenSocket : public IEventHandle {
			EventLoop_libevent* owner;
			event* ev;
			IEventLoopWithSockets::AcceptCallback on_accept;
			IEventLoopWithSockets::ConnectionErrorCallback on_error;
			bool active_ = false;
			
			LibEventListenSocket(EventLoop_libevent* owner) : owner(owner) {}
			~LibEventListenSocket() {
				cancel();
				event_free(ev);
			}
			
			void activate() final {
				event_add(ev, nullptr);
				active_ = true;
			}
			
			void cancel() final {
				event_del(ev);
			}
			
			bool is_active() const final {
				return active_;
			}
			
			bool is_repeating() const final {
				return true;
			}
			
			void handle(evutil_socket_t fd, short ev) {
				if (ev & EV_READ) {
					bool keepgoing = true;
					while (keepgoing) {
						struct sockaddr_in remote_addr;
						socklen_t addrlen = sizeof(remote_addr);
						int sockfd = accept(fd, (struct sockaddr*)&remote_addr, &addrlen);
						if (sockfd < 0) {
							if (errno != EWOULDBLOCK && errno != EAGAIN) {
								on_error("Error accepting socket.", false);
							}
							keepgoing = false;
						} else {
							bufferevent* bev = bufferevent_socket_new(owner->impl->loop, sockfd, BEV_OPT_THREADSAFE);
							auto p = make_unique<LibEventBufferedSocket>(default_allocator(), bev);
							auto raw = p.get();
							bufferevent_setcb(bev, libevent_bufferevent_data_callback, nullptr, libevent_bufferevent_callback, raw);
							on_accept(move(p)); // XXX: on_accept may delete p
							raw->activate();
							if (raw->on_connect_)
								raw->on_connect_();
						}
					}
					
				}
			}
		};
		
		void libevent_listensocket_callback(evutil_socket_t fd, short ev, void* p) {
			reinterpret_cast<LibEventListenSocket*>(p)->handle(fd, ev);
		}
	}
	
	UniquePtr<IEventHandle> EventLoop_libevent::listen(IAllocator& alloc, uint32 port, AcceptCallback on_accept, ConnectionErrorCallback on_error) {
		sockaddr_in local_addr;
		memset(&local_addr, 0, sizeof(local_addr));
		local_addr.sin_family = AF_UNSPEC;
		local_addr.sin_port = htons(port);
		local_addr.sin_addr.s_addr = INADDR_ANY;
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0) {
			on_error(strerror(errno), true);
			return nullptr;
		}
		if (evutil_make_listen_socket_reuseable(fd) < 0) {
			on_error("Error making socket reusable.", false);
		}
		if (::bind(fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
			on_error("Error binding socket address.", true);
			return nullptr;
		}
		if (::listen(fd, 8) < 0) {
			on_error("Error calling listen().", true);
			return nullptr;
		}
		if (evutil_make_socket_nonblocking(fd) < 0) {
			on_error("Error making socket nonblocking.", true);
			return nullptr;
		}
		
		auto p = make_unique<LibEventListenSocket>(alloc, this);
		event* ev = event_new(impl->loop, fd, EV_READ | EV_PERSIST, libevent_listensocket_callback, p.get());
		p->ev = ev;
		p->on_accept = move(on_accept);
		p->on_error = move(on_error);
		p->activate();
		return move(p);
	}
}