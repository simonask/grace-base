//
//  event_loop.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 19/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_event_loop_hpp
#define falling_event_loop_hpp

#include "base/basic.hpp"
#include "memory/unique_ptr.hpp"
#include "base/time.hpp"
#include "base/string.hpp"
#include "event/event_handle.hpp"
#include "base/function.hpp"

namespace falling {
	using FileSystemDescriptor = int;
	
	enum class FileSystemEvent : uint8 {
		Read       = 1 << 0, // Callback will be called when the fd is ready to be read from.
		Write      = 1 << 1, // Callback will be called when the fd is ready to be written to.
		Persistent = 1 << 2, // Event will be re-added to the event loop when triggered.
		Timeout    = 1 << 3, // Callback will be called when the fd timed out.
	};
	ENUM_IS_FLAGS(FileSystemEvent);
	
	enum class InputEventType : uint32 {
		// Keyboard events
		KeyUp   = 1 << 0,
		KeyDown = 1 << 1,
		KeyText = 1 << 2,
		KeyEventMask = 0x7,
		
		// Mouse events  (data[0] == button number)
		MouseDown = 1 << 8,
		MouseUp   = 1 << 9,
		MouseMove = 1 << 10,
		MouseEventMask = 0x700,
	};
	ENUM_IS_FLAGS(InputEventType);
	
	enum class KeyboardModifier : uint8 {
		Shift = 1 << 0,
		CtrlOrCmd = 1 << 1,
		WinOrCtrl = 1 << 2,
		Alt = 1 << 3,
	};
	ENUM_IS_FLAGS(KeyboardModifier);
	
	struct InputEvent {
		InputEventType type;
		byte data[4]; // enough to hold all UTF-8 chars?
		
		bool is_key_event() const { return ((uint32)type & (uint32)InputEventType::KeyEventMask) != 0; }
		bool is_mouse_event() const { return ((uint32)type & (uint32)InputEventType::MouseEventMask) != 0; }
		bool has_text() const { return is_key_event() && data[0] != 0; }
		bool is_text() const { return type == InputEventType::KeyText; }
		bool is_modifier() const { return is_key_event() && data[0] == 0; }
		StringRef text() const { return StringRef((const char*)data, text_length()); }
		
		size_t text_length() const {
			// I bet the compiler can optimize this. Right...?!
			return ::strnlen((const char*)data, 4);
		}
	};
	
	using CallbackID = uintptr_t;

	struct IEventLoop {
		using FileSystemCallback = Function<void(FileSystemDescriptor fd, FileSystemEvent event)>;
		using InputEventCallback = Function<bool(const InputEvent&)>; // return: handled?
	
		// Call-later API
		virtual IEventHandle* schedule(IAllocator&, Function<void()>, SystemTimeDelta delay) = 0;
		virtual IEventHandle* call_repeatedly(IAllocator&, Function<void()>, SystemTimeDelta interval) = 0;
		
		// Input Event API
		virtual IEventHandle* listen_for_input_event(IAllocator&, uint32 input_event_mask, InputEventCallback callback) = 0;
		virtual void push_input_event(InputEvent event, bool handle_immediately = true) = 0;
		
		// Async File I/O API
		virtual IEventHandle* watch_descriptor(IAllocator&, FileSystemDescriptor fd, uint8 event_mask, FileSystemCallback callback, SystemTimeDelta timeout = SystemTimeDelta::forever()) = 0;
		
		// Necessary for integration with external event loops (such as platform toolkits)
		virtual SystemTimeDelta time_until_next_event() = 0; // SystemTimeDelta::forever() if there are no events
		virtual void run_once() = 0;
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
