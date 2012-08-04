//
//  fiber.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 01/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/fiber.hpp"
#include "base/log.hpp"

#include <setjmp.h>

namespace falling {
	IFiberManager* IFiberManager::current_manager_ = nullptr; // TODO: Thread-local
	
	FiberManager::FiberManager() : current_(nullptr) {}
	
	void FiberManager::update(GameTime current_time_sync) {
		set_current_manager(this);
		now_ = current_time_sync;
		
		// Don't use iterators, because fibers may spawn new fibers,
		// which would invalidate the iterators.
		for (size_t i = 0; i < fibers_.size();) {
			if (fibers_[i].wake_up_at <= now_) {
				current_ = fibers_[i].fiber.get();
				fibers_[i].fiber->resume();
				current_ = nullptr;
				if (fibers_[i].fiber->state() == FiberState::Unstarted) {
					// Fiber has terminated, remove it.
					fibers_.erase(i);
				} else {
					++i;
				}
			} else {
				++i;
			}
		}
	}
	
	void FiberManager::set_alarm_clock(Fiber *fiber, GameTime at) {
		ASSERT(fiber->owner() == this);
		for (auto& info: fibers_) {
			if (info.fiber.get() == fiber) {
				info.wake_up_at = at;
				return;
			}
		}
		ASSERT(false); // Fiber had this manager as owner, but wasn't in the list of fibers.
	}
	
	void FiberManager::launch(std::function<void ()> function) {
		defer(std::move(function), now_);
	}
	
	void FiberManager::defer(std::function<void ()> function, GameTime until) {
		Fiber* f = new Fiber(*this, std::move(function), until);
		FiberInfo info;
		info.fiber = std::unique_ptr<Fiber>(f);
		info.wake_up_at = until;
		fibers_.emplace_back(std::move(info));
	}
	
	FiberManager::~FiberManager() {
		set_current_manager(this);
		fibers_.clear(); // calls terminate on each fiber.
		set_current_manager(nullptr);
	}
	
	struct Fiber::Impl {
		IFiberManager& owner;
		std::function<void()> function;
		FiberState state;
		Array<byte> saved_stack;
		byte* stack_top;
		jmp_buf portal;
		
		Impl(IFiberManager& manager, std::function<void()> function) : owner(manager), function(std::move(function)), stack_top(nullptr) {}
	};
	
	Fiber::Fiber(IFiberManager& manager, std::function<void()> function, GameTime start_at) {
		impl_ = new Impl(manager, std::move(function));
		impl().state = FiberState::Unstarted;
	}
	
	Fiber::~Fiber() {
		this->terminate(nullptr);
		delete impl_;
	}
	
	Fiber* Fiber::current() {
		return IFiberManager::current()->current_fiber();
	}
	
	void Fiber::yield() {
		return current()->yield(nullptr);
	}
	
	void Fiber::terminate() {
		current()->terminate(nullptr);
	}
	
	IFiberManager* Fiber::owner() const {
		return &impl().owner;
	}
	
	FiberState Fiber::state() const {
		return impl().state;
	}
	
	namespace {
		struct FiberError {};
		struct FiberTerminated {};
		
		__attribute__((noinline)) byte* get_sp() {
			void* sp;
			sp = (void*)&sp;
			return (byte*)sp;
		}
		
		void launchpad(Fiber* fiber) {
			try {
				fiber->impl().function();
			}
			catch (FiberTerminated) {}
			catch (...) {
				ASSERT(false); // Unhandled exception in fiber.
			}
			fiber->impl().saved_stack.clear();
			fiber->impl().state = FiberState::Unstarted;
			longjmp(fiber->impl().portal, 1);
		}
	}
	
	void Fiber::start() {
		ASSERT(state() == FiberState::Unstarted);
		resume_into_state(FiberState::Running);
	}
	
	void Fiber::resume() {
		if (state() != FiberState::Sleeping && state() != FiberState::Unstarted) {
			throw FiberError();
		}
		resume_into_state(FiberState::Running);
	}
	
	void Fiber::terminate(void*) {
		switch (state()) {
			case FiberState::Terminating:
			case FiberState::Running: throw FiberTerminated();
			case FiberState::Unstarted: return;
			case FiberState::Sleeping: resume_into_state(FiberState::Terminating); return;
		}
	}
	
	void Fiber::yield(void*) {
		if (state() != FiberState::Running) {
			throw FiberError();
		}
		jmp_buf away_from_fiber;
		memcpy(away_from_fiber, impl().portal, sizeof(away_from_fiber));
		if (setjmp(impl().portal) == 0) {
			// first, save the stack:
			byte* stack_bottom = get_sp();
			size_t stack_size = impl().stack_top - stack_bottom;
			impl().saved_stack.resize(stack_size);
			std::copy(stack_bottom, impl().stack_top, impl().saved_stack.begin());
			
			// change state
			impl().state = FiberState::Sleeping;
			
			// jump away!
			longjmp(away_from_fiber, 1);
		} else {
			// resuming
			if (state() == FiberState::Terminating) {
				this->terminate(nullptr); // which throws and thus unwinds the stack
			}
			if (state() != FiberState::Running) {
				throw FiberError(); // Wrong state!
			}
			return;
		}
	}
	
	void Fiber::resume_into_state(FiberState new_state) {
		if (new_state != FiberState::Running && new_state != FiberState::Terminating) {
			throw FiberError();
		}
		
		uintptr_t sp = (uintptr_t)get_sp();
		uintptr_t fiber_stack_top_int = sp & (~0xfffULL); // align on page boundary
		byte* fiber_stack_top = (byte*)fiber_stack_top_int;
		
		switch (state()) {
			case FiberState::Unstarted: {
				if (new_state == FiberState::Terminating)
					return;
				if (setjmp(impl().portal) == 0) {
					// launch!
					impl().state = FiberState::Running;
					impl().stack_top = fiber_stack_top;
					__asm__ __volatile__
					(
					 "movq %%rsp, %%rbx\n"
					 "movq %0, %%rsp\n"
					 "movq %1, %%rdi\n"
					 "callq *%2\n"
					 "movq %%rbx, %%rsp\n"
					 : // input registers
					 : "r"(fiber_stack_top), "r"(this), "r"(launchpad) // output registers
					 : "rsp", "rbx" // clobbered
					 );
				} else {
					// coming back!
					if (impl().state != FiberState::Sleeping && impl().state != FiberState::Unstarted) {
						throw FiberError();
					}
					return;
				}
			}
			case FiberState::Sleeping: {
				jmp_buf into_fiber;
				memcpy(into_fiber, impl().portal, sizeof(into_fiber));
				if (setjmp(impl().portal) == 0) {
					impl().state = new_state;
					
					// Restore stack
					if (fiber_stack_top != impl().stack_top) {
						Warning() << "Resuming fiber from different callsite!";
						impl().stack_top = fiber_stack_top;
					}
					size_t bytes_to_restore = impl().saved_stack.size();
					byte* copy_to = fiber_stack_top - bytes_to_restore;
					std::copy(impl().saved_stack.begin(), impl().saved_stack.end(), copy_to);
					
					// jump back in!
					longjmp(into_fiber, 1);
				} else {
					// coming back!
					if (impl().state != FiberState::Sleeping && impl().state != FiberState::Unstarted) {
						throw FiberError();
					}
					return;
				}
			}
			default: {
				Error() << "Cannot resume running or terminating fiber!";
				throw FiberError();
			}
		}
	}
	
	void Fiber::sleep(GameTimeDelta delta) {
		Fiber::current()->sleep(delta, nullptr);
	}
	
	void Fiber::sleep(GameTimeDelta delta, void *dummy) {
		GameTime until = impl().owner.now() + delta;
		sleep_until(until, nullptr);
	}
	
	void Fiber::sleep_until(GameTime time) {
		Fiber::current()->sleep_until(time);
	}
	
	void Fiber::sleep_until(GameTime until, void *dummy) {
		impl().owner.set_alarm_clock(this, until);
		yield(nullptr);
	}
}