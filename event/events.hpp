//
//  events.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 14/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_events_hpp
#define grace_events_hpp

#include "geometry/vector.hpp"

namespace grace {
	enum class FileSystemEvent : uint8 {
		Read       = 1 << 0, // Callback will be called when the fd is ready to be read from.
		Write      = 1 << 1, // Callback will be called when the fd is ready to be written to.
		Persistent = 1 << 2, // Event will be re-added to the event loop when triggered.
		Timeout    = 1 << 3, // Callback will be called when the fd timed out.
	};
	ENUM_IS_FLAGS(FileSystemEvent);
	
	enum class EventResponse {
		PassOn,
		Handled,
	};
}

#endif
