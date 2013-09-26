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
#include "event/event_handle.hpp"
#include "base/function.hpp"

namespace grace {
	struct IAsyncInputStream;
	struct IAsyncOutputStream;
	struct IEventedSocket;
	struct IInputManager;

	struct IInputStreamNonblocking;
	struct IOutputStreamNonblocking;

	struct IEventLoop {
		virtual ~IEventLoop() {}

		// Timer API
		virtual UniquePtr<IEventHandle> schedule(Function<void()>, SystemTimeDelta delay, IAllocator& = default_allocator()) = 0;
		virtual UniquePtr<IEventHandle> call_repeatedly(Function<void()>, SystemTimeDelta interval, IAllocator& = default_allocator()) = 0;
		
		// Main
		virtual void quit() = 0;
		virtual void run() = 0;
	};

	// This creates a suitable event loop for the current platform:
	UniquePtr<IEventLoop> create_event_loop(IAllocator& = default_allocator());
}

#endif
