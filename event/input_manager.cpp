//
//  input_manager.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 14/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "event/input_manager.hpp"
#include "event/input_responder.hpp"
#include "event/input_space.hpp"

namespace grace {
	struct InputManager::Impl {
		IInputSpace* space = nullptr;
		
		// TODO: Capture more than one event type at a time.
		IInputResponder* capturing_responder = nullptr;
		InputEventType capturing_event;
		InputEventType capturing_until_match;
		bool capturing_include_matched = true;
	};

	namespace {
		typename std::aligned_storage<sizeof(InputManager), alignof(InputManager)>::type input_manager_storage;
		typename std::aligned_storage<sizeof(InputManager::Impl), alignof(InputManager::Impl)>::type input_manager_impl_storage;
	}

	InputManager& InputManager::get() {
		static InputManager* p = new(&input_manager_storage) InputManager;
		return *p;
	}
	
	InputManager::InputManager() {
		impl = new(&input_manager_impl_storage) InputManager::Impl;
	}
	
	IInputResponder* InputManager::first_responder() const {
		return impl->space->first_responder();
	}
	
	/*void InputManager::set_first_responder(InputResponder *responder) {
		if (responder == nullptr) {
			impl->first_responder = nullptr;
			return;
		}
		
		InputResponder* r = responder;
		while (!r->can_receive_focus()) {
			r = responder->next_responder();
		}
		InputResponder* old = impl->first_responder;
		if (old != nullptr)
			old->will_lose_focus();
		if (r != nullptr)
			r->will_receive_focus();
		impl->first_responder = r;
		if (r != nullptr)
			r->did_receive_focus();
		if (old != nullptr) {
			old->did_lose_focus();
		}
	}*/
	
	/*void InputManager::capture_input_until_event_matching(InputResponder *responder, InputEventType event, InputEventType until_event_type, bool include_matched) {
		impl->capturing_responder = responder;
		impl->capturing_event = event;
		impl->capturing_until_match = until_event_type;
		impl->capturing_include_matched = include_matched;
	}*/
	
	void InputManager::clear() {
		impl->space = nullptr;
		impl->capturing_responder = nullptr;
	}
	
	IInputSpace* InputManager::space() const {
		return impl->space;
	}
	
	void InputManager::push_space(IInputSpace *space) {
		space->set_predecessor(impl->space);
		impl->space = space;
	}
	
	void InputManager::pop_space(IInputSpace *space) {
		if (impl->space == space) {
			impl->space = space->predecessor();
		} else {
			IInputSpace* last = nullptr;
			auto p = impl->space;
			while (p != space) {
				last = p;
				p = p->predecessor();
			}
			if (p != nullptr && last != nullptr) {
				last->set_predecessor(p->predecessor());
			}
		}
	}
	
	EventResponse InputManager::push_event(const InputEvent& event) {
		if (impl->space) {
			return impl->space->respond(event);
		} else {
			return EventResponse::PassOn;
		}
	}
	
	void InputManager::begin_text_input() {
		on_begin_text_input();
	}
	
	void InputManager::end_text_input() {
		on_end_text_input();
	}
}
