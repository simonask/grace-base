//
//  input_manager.h
//  grace
//
//  Created by Simon Ask Ulsnes on 14/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __grace__input_manager__
#define __grace__input_manager__

#include "event/event_loop.hpp"
#include "base/auto_link_queue.hpp"
#include "object/signal.hpp"

namespace grace {
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
		
		Signal<> on_begin_text_input;
		Signal<> on_end_text_input;
		
		void begin_text_input();
		void end_text_input();

		struct Impl;
		Impl* impl;
	private:
		InputManager();
	};
}

#endif /* defined(__grace__input_manager__) */
