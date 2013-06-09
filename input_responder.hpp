//
//  input_responder.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 24/02/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_input_responder_hpp
#define grace_input_responder_hpp

#include "base/auto_link_queue.hpp"
#include "base/geometry.hpp"
#include "event/events.hpp"

namespace grace {
	struct InputEvent;
	
	class IInputResponder {
	public:
		virtual EventResponse respond(const InputEvent& event) = 0;
		virtual IInputResponder* next_responder() = 0;
		
		virtual bool wants_mouse_motion_events() const = 0;
		
		// "Focus" == being first responder
		virtual bool can_receive_focus() const = 0;
		virtual void will_receive_focus() = 0;
		virtual void did_receive_focus() = 0; // i.e., received focus
		virtual void will_lose_focus() = 0;
		virtual void did_lose_focus() = 0;
		virtual Rect screen_bounds() const = 0;
		virtual bool screen_hit_test(vec2 point) const = 0;
	};
}

#endif
