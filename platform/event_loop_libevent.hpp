#include "event/event_loop.hpp"
#include "event/capabilities.hpp"
#include <event2/event.h>

namespace grace {
	struct EventLoop_libevent : public InterfaceWithCapabilities<IEventLoop,
		capability::POpen,
		capability::Connect,
		capability::Listen,
		capability::StdIn
	> {
		EventLoop_libevent();
		virtual ~EventLoop_libevent();

		// Timer API
		UniquePtr<IEventHandle> schedule(Function<void()>, SystemTimeDelta delay, IAllocator& = default_allocator()) final;
		UniquePtr<IEventHandle> call_repeatedly(Function<void()>, SystemTimeDelta interval, IAllocator& = default_allocator()) final;

		// Capabilities
		UniquePtr<IEventHandle> connect(StringRef host, uint16 port, Function<void(NetworkConnectionEvent, INetworkStream&)> callback, SystemTimeDelta timeout) final;
		UniquePtr<IEventHandle> listen(uint16 port, Function<void(ServerEvent, Server&)> callback) final;
		UniquePtr<IEventHandle> popen(StringRef command, ArrayRef<StringRef> arguments, Function<void(ProcessEvent, Process&)> callback, SystemTimeDelta timeout) final;
		UniquePtr<IEventHandle> stdin(Function<void(StdInEvent, ConsoleStream&)> callback, SystemTimeDelta timeout) final;
		
		// Main
		void quit();
		void run();
	private:
		event_base* base_;
		bool is_running_ = true;
	};
}