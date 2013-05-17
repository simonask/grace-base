//
//  keys.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 16/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "event/keys.hpp"
#include "io/formatted_stream.hpp"

#include <SDL2/SDL_keyboard.h>

namespace falling {
	FormattedStream& operator<<(FormattedStream& os, Key key) {
		SDL_Keycode code = (SDL_Keycode)key;
		StringRef keyname = SDL_GetKeyName(code);
		return os << '[' << keyname << ']';
	}
	
	FormattedStream& operator<<(FormattedStream& os, enum KeyModifier modifier) {
		switch (modifier) {
			case KeyModifier::None:       return os << "[]";
			case KeyModifier::LeftShift:  return os << Key::LeftShift;
			case KeyModifier::RightShift: return os << Key::RightShift;
			case KeyModifier::LeftCtrl:   return os << Key::LeftCtrl;
			case KeyModifier::RightCtrl:  return os << Key::RightCtrl;
			case KeyModifier::LeftAlt:    return os << Key::LeftAlt;
			case KeyModifier::RightAlt:   return os << Key::RightAlt;
			case KeyModifier::LeftGui:    return os << Key::LeftGui;
			case KeyModifier::RightGui:   return os << Key::RightGui;
			case KeyModifier::Ctrl:       return os << "[Ctrl]";
			case KeyModifier::Shift:      return os << "[Shift]";
			case KeyModifier::Alt:        return os << "[Alt]";
			case KeyModifier::Gui:        return os << "[Gui]";
		}
	}
}
