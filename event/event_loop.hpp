//
//  event_loop.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 19/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_event_loop_hpp
#define grace_event_loop_hpp

#include "base/basic.hpp"
#include "memory/unique_ptr.hpp"
#include "base/time.hpp"
#include "base/string.hpp"
#include "event/event_handle.hpp"
#include "base/function.hpp"
#include "event/events.hpp"
#include "memory/unique_ptr.hpp"

namespace grace {
	struct IAsyncInputStream;
	struct IAsyncOutputStream;
	struct IEventedSocket;
	struct IInputManager;

	enum class StreamEvent : uint8 {
		Read   = 1,
		Write  = 1 << 1,
		Error  = 1 << 2,
		Closed = 1 << 3,
		Ready  = 1 << 4,
		Accept = 1 << 5,
		Any    = 0xff
	};
	ENUM_IS_FLAGS(StreamEvent);

	// Different stream types that event loops must support:
	struct FileStream;
	struct NetworkStream;
	struct ServerStream;
	struct PipeStream;
	struct ConsoleStream;

	struct IEventLoop {
		// Timer API
		virtual UniquePtr<IEventHandle> schedule(Function<void()>, SystemTimeDelta delay, IAllocator& = default_allocator()) = 0;
		virtual UniquePtr<IEventHandle> call_repeatedly(Function<void()>, SystemTimeDelta interval, IAllocator& = default_allocator()) = 0;

		// Async I/O API
		virtual UniquePtr<IEventHandle> add(FileStream& stream,    uint8 events, Function<void(StreamEvent, FileStream& stream)> handler) = 0;
		virtual UniquePtr<IEventHandle> add(ServerStream& stream,  uint8 events, Function<void(StreamEvent, ServerStream& stream)> handler) = 0;
		virtual UniquePtr<IEventHandle> add(PipeStream& stream,    uint8 events, Function<void(StreamEvent, PipeStream& stream)> handler) = 0;
		virtual UniquePtr<IEventHandle> add(NetworkStream& stream, uint8 events, Function<void(StreamEvent, NetworkStream& stream)> handler) = 0;
		virtual UniquePtr<IEventHandle> add(ConsoleStream& stream, uint8 events, Function<void(StreamEvent, ConsoleStream& stream)> handler) = 0;
		
		// Main
		virtual void quit() = 0;
		virtual void run() = 0;
	};

	// This creates a suitable event loop for the current platform:
	UniquePtr<IEventLoop> create_event_loop();
}

#endif
