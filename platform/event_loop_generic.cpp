#include "platform/event_loop_libevent.hpp"

namespace grace {
	UniquePtr<IEventLoop> create_event_loop(IAllocator& alloc) {
		// TODO: Add more event loop types
		auto p = make_unique<EventLoop_libevent>(alloc);
		return std::move(p);
	}
}