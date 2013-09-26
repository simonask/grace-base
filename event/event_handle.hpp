//
//  event_handle.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 20/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_event_handle_hpp
#define grace_event_handle_hpp

namespace grace {
	struct IEventHandle {
		virtual ~IEventHandle() {}
		virtual bool is_repeating() const = 0;
		virtual bool is_active() const = 0;
		virtual void activate() = 0;
		virtual void cancel() = 0; // Implementations should also cancel themselves in their destructors
	};
}

#endif
