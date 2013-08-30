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
	class EventLoop_libevent : public IEventLoopWithSockets {
	public:
		EventLoop_libevent();
		~EventLoop_libevent();
		void run();
	
		// IEventLoop interface
		UniquePtr<IEventHandle> schedule(IAllocator&, Function<void()>, SystemTimeDelta delay) final;
		UniquePtr<IEventHandle> call_repeatedly(IAllocator&, Function<void()>, SystemTimeDelta interval) final;
		//UniquePtr<IEventHandle> listen_for_input_event(IAllocator&, uint32 input_event_mask, InputEventCallback callback) final;
		//void push_input_event(InputEvent event, bool handle_immediately = true) final;
		UniquePtr<IEventHandle> watch_descriptor(IAllocator&, FileSystemDescriptor fd, uint8 event_mask, FileSystemCallback callback, SystemTimeDelta timeout) final;
		//void record_input_events(StringRef output_file_path) final {}
		//void replay_recorded_input_events(StringRef recorded_events_file_path, bool authentic_time) final {}
		
		// IEventLoopWithSockets interface
		UniquePtr<IEventedSocket> connect(IAllocator&, StringRef host, uint32 port, ConnectionCallback on_connect, ConnectionErrorCallback on_error) final;
		UniquePtr<IEventHandle> listen(IAllocator&, uint32 port, AcceptCallback on_accept, ConnectionErrorCallback on_error) final;
		
		struct Impl;
		Impl* impl = nullptr;
	private:
		//void broadcast_input_event(const InputEvent&);
	};
}

#endif /* defined(__editor__libevent_adapter__) */
