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

namespace grace {
	using FileSystemDescriptor = int;
	
	using CallbackID = uintptr_t;

	struct IEventLoop {
		using FileSystemCallback = Function<void(FileSystemDescriptor fd, FileSystemEvent event)>;
		using InputEventCallback = Function<EventResponse(const InputEvent&)>; // return: handled?
	
		// Call-later API
		virtual IEventHandle* schedule(IAllocator&, Function<void()>, SystemTimeDelta delay) = 0;
		virtual IEventHandle* call_repeatedly(IAllocator&, Function<void()>, SystemTimeDelta interval) = 0;
		
		// Input Event API
		virtual IEventHandle* listen_for_input_event(IAllocator&, uint32 input_event_mask, InputEventCallback callback) = 0;
		virtual void push_input_event(InputEvent event, bool handle_immediately = true) = 0;
		
		// Input Event Recording API
		virtual void record_input_events(StringRef output_file_path) = 0;
		virtual void replay_recorded_input_events(StringRef recorded_events_file_path, bool authentic_time) = 0;
		
		// Async File I/O API
		virtual IEventHandle* watch_descriptor(IAllocator&, FileSystemDescriptor fd, uint8 event_mask, FileSystemCallback callback, SystemTimeDelta timeout = SystemTimeDelta::forever()) = 0;
		
		// Main
		virtual void run() = 0;
	};
	
	struct IEventedSocket;
	
	struct IEventLoopWithSockets : public IEventLoop {
		using ConnectionCallback = Function<void(IEventedSocket*)>;
		using ConnectionErrorCallback = Function<void(StringRef, bool fatal)>;
		virtual ~IEventLoopWithSockets() {}
		virtual IEventHandle* connect(IAllocator&, StringRef host, uint32 port, ConnectionCallback on_connect, ConnectionErrorCallback on_error) = 0;
		virtual IEventHandle* listen(IAllocator&, uint32 port, ConnectionCallback on_accept, ConnectionErrorCallback on_error) = 0;
	};
}

#endif
