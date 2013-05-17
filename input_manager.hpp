//
//  input_manager.h
//  falling
//
//  Created by Simon Ask Ulsnes on 14/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __falling__input_manager__
#define __falling__input_manager__

#include "event/event_loop.hpp"
#include "base/auto_link_queue.hpp"

namespace falling {
	class IInputResponder;
	class IInputSpace;

	class InputManager {
	public:
		static InputManager& get();
		
		void capture_input_until_event_matching(IInputResponder* responder, InputEventType capture_event_type, InputEventType until_event_type, bool include_matched = true);
		
		IInputSpace* space() const;
		void push_space(IInputSpace* space);
		void pop_space(IInputSpace* space);
		IInputResponder* first_responder() const;
		EventResponse push_event(const InputEvent&);
		void clear();

		struct Impl;
		Impl* impl;
	private:
		InputManager();
	};
}

#endif /* defined(__falling__input_manager__) */
