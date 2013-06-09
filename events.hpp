//
//  events.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 14/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_events_hpp
#define grace_events_hpp

#include "event/keys.hpp"
#include "base/vector.hpp"

namespace grace {
	enum class FileSystemEvent : uint8 {
		Read       = 1 << 0, // Callback will be called when the fd is ready to be read from.
		Write      = 1 << 1, // Callback will be called when the fd is ready to be written to.
		Persistent = 1 << 2, // Event will be re-added to the event loop when triggered.
		Timeout    = 1 << 3, // Callback will be called when the fd timed out.
	};
	ENUM_IS_FLAGS(FileSystemEvent);
	
	enum class InputEventType {
		// Keyboard events
		KeyUp   = 1 << 0,
		KeyDown = 1 << 1,
		KeyTextInput = 1 << 2,
		KeyTextEdit  = 1 << 3,
		KeyEventMask = 0xf,
		
		// Mouse events
		MouseDown  = 1 << 8,
		MouseUp    = 1 << 9,
		MouseMove  = 1 << 10,
		MouseWheel = 1 << 11,
		MouseEventMask = 0xf00,
		
		AllEventsMask = 0xffff
	};
	ENUM_IS_FLAGS(InputEventType);
	
	
	struct InputEvent {
		InputEventType type;
		union {
			struct {
				char character[4]; // 1 utf8 character
				
				// for editing events:
				uint16 offset;
				uint16 length;
			} text;
			struct {
				Key key;
				uint32 modifiers;
				bool is_repeat;
			} key;
			struct {
				uint8 button;
			} mouse;
			struct {
				int32 x;
				int32 y;
			} wheel;
			struct {
				struct {int32 x; int32 y;} position;
				struct {int32 x; int32 y;} movement;
			} move;
		};
		
		
		bool is_key_event() const { return ((uint32)type & (uint32)InputEventType::KeyEventMask) != 0; }
		bool is_mouse_event() const { return ((uint32)type & (uint32)InputEventType::MouseEventMask) != 0; }
		StringRef str() const { return StringRef(text.character, str_length()); }
		size_t str_length() const {
			// I bet the compiler can optimize this. Right...?!
			return ::strnlen(text.character, 4);
		}
	};
	
	enum class EventResponse {
		PassOn,
		Handled,
	};
}

#endif
