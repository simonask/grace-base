//
//  libevent_adapter.h
//  editor
//
//  Created by Simon Ask Ulsnes on 20/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __editor__libevent_adapter__
#define __editor__libevent_adapter__

#include "event/event_loop.hpp"

namespace grace {
	class EventLoop_libevent : public IEventLoop {
	public:
		EventLoop_libevent();
		~EventLoop_libevent();

		// Call-later API
		UniquePtr<IEventHandle> schedule(Function<void()>, SystemTimeDelta delay, IAllocator& = default_allocator());
		UniquePtr<IEventHandle> call_repeatedly(Function<void()>, SystemTimeDelta interval, IAllocator& = default_allocator());
		
		// Async File I/O API
		UniquePtr<IEventHandle> add_input_stream(IAsyncInputStream& stream, InputStreamCallback, SystemTimeDelta timeout = SystemTimeDelta::forever(), IAllocator& alloc = default_allocator());
		UniquePtr<IEventHandle> add_output_stream(IAsyncOutputStream& stream, OutputStreamCallback, SystemTimeDelta timeout = SystemTimeDelta::forever(), IAllocator& alloc = default_allocator());

		// Async Network I/O API
		UniquePtr<IEventedSocket> connect(IAllocator&, StringRef host, uint32 port, ConnectionCallback on_connect, ConnectionErrorCallback on_error);
		UniquePtr<IEventHandle> listen(IAllocator&, uint32 port, AcceptCallback on_accept, ConnectionErrorCallback on_error);
		
		// Main
		void quit();
		void run();
	
		struct Impl;
		Impl* impl = nullptr;
	};
}

#endif /* defined(__editor__libevent_adapter__) */
