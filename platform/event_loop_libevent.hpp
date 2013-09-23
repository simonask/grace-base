#include "event/event_loop.hpp"
#include <event2/event.h>

namespace grace {
	struct EventLoop_libevent : IEventLoop {
		EventLoop_libevent();
		virtual ~EventLoop_libevent();

		// Timer API
		UniquePtr<IEventHandle> schedule(Function<void()>, SystemTimeDelta delay, IAllocator& = default_allocator()) final;
		UniquePtr<IEventHandle> call_repeatedly(Function<void()>, SystemTimeDelta interval, IAllocator& = default_allocator()) final;

		// Async I/O API
		UniquePtr<IEventHandle> add(InputFileStream& stream,  Function<void(StreamEvent, InputFileStream& stream)> handler) final;
		UniquePtr<IEventHandle> add(OutputFileStream& stream, Function<void(StreamEvent, OutputFileStream& stream)> handler) final;
		UniquePtr<IEventHandle> add(NetworkStream& stream,    Function<void(StreamEvent, NetworkStream& stream)> handler) final;
		UniquePtr<IEventHandle> add(ServerStream& stream,     Function<void(StreamEvent, ServerStream& stream)> handler) final;
		UniquePtr<IEventHandle> add(PipeStream& stream,       Function<void(StreamEvent, PipeStream& stream)> handler) final;

		// Network API
		UniquePtr<NetworkStream> connect(StringRef host, uint16 port, Function<void(StreamEvent, NetworkStream&)> callback) final;
		UniquePtr<ServerStream>  listen(uint16 port, Function<void(StreamEvent, ServerStream&)> callback) final;
		
		// Main
		void quit();
		void run();
	private:
		event_base_t* base_;
	};
}