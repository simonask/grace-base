//
//  input_space.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 15/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_input_space_hpp
#define falling_input_space_hpp

#include "event/events.hpp"

namespace falling {
	class IInputResponder;
	
	struct IInputSpace {
		virtual void add(IInputResponder* responder) = 0;
		virtual EventResponse respond(const InputEvent& event) = 0;
		virtual IInputResponder* first_responder() const = 0;
		virtual IInputSpace* predecessor() const = 0;
		virtual void set_predecessor(IInputSpace* predecessor) = 0;
	};
}

#endif
