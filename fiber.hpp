//
//  fiber.h
//  falling
//
//  Created by Simon Ask Ulsnes on 01/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef __falling__fiber__
#define __falling__fiber__

#include <functional>
#include "base/array.hpp"
#include "base/time.hpp"
#include "memory/unique_ptr.hpp"

namespace falling {
	enum FiberState : byte {
		Unstarted,
		Running,
		Sleeping,
		Terminating,
		UnhandledException,
	};
	
	class IFiberManager;
	
	class Fiber {
	public:
		static Fiber* current();
		static void yield();
		static void sleep(GameTimeDelta delta);
		static void sleep_until(GameTime time);
		static void terminate();
		
		~Fiber();
		
		IFiberManager* owner() const;
		FiberState state() const;
		void start();
		void resume();
		void terminate(void* dummy);
		
		struct Impl;
		Impl* impl_;
		Impl& impl() { return *impl_; }
		const Impl& impl() const { return *impl_; }
		explicit Fiber(IFiberManager& m, std::function<void()> f, GameTime start_at);
	private:
		void yield(void* dummy);
		void resume_into_state(FiberState new_state);
		void sleep_until(GameTime time, void* dummy);
		void sleep(GameTimeDelta, void* dummy);
	};
	
	class IFiberManager {
	public:
		virtual GameTime now() const = 0;
		virtual void update(GameTime current_time_sync) = 0;
		virtual void set_alarm_clock(Fiber* fiber, GameTime at) = 0;
		virtual void launch(std::function<void()> f) = 0;
		virtual void defer(std::function<void()> f, GameTime until) = 0;
		virtual Fiber* current_fiber() const = 0;
		
		static IFiberManager* current() { return current_manager_; }
		// Call this in update():
		static void set_current_manager(IFiberManager* manager) { current_manager_ = manager; }
	private:
		static IFiberManager* current_manager_;
	};
	
	class FiberManager : public IFiberManager {
	public:
		FiberManager();
		virtual ~FiberManager();
		GameTime now() const override { return now_; }
		void update(GameTime current_time_sync) override;
		void set_alarm_clock(Fiber* fiber, GameTime at) override;
		void launch(std::function<void()> f) override;
		void defer(std::function<void()> f, GameTime until) override;
		Fiber* current_fiber() const override { return current_; }
	private:
		Fiber* current_;
		GameTime now_;
		struct FiberInfo {
			UniquePtr<Fiber> fiber;
			GameTime wake_up_at;
		};
		Array<FiberInfo> fibers_;
	};
}

#endif /* defined(__falling__fiber__) */
