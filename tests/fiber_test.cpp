//
//  fiber_test.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_fiber_test_hpp
#define grace_fiber_test_hpp

#include "tests/test.hpp"
#include "base/fiber.hpp"
#include "base/stack_array.hpp"

using namespace grace;

SUITE(Fiber) {
	it("should launch a fiber", []() {
		FiberManager manager;
		bool fiber_ran = false;
		manager.launch([&]() {
			fiber_ran = true;
		});
		manager.update(GameTime());
		TEST(fiber_ran).should == true;
	});
	
	it("should resume a yielded fiber", []() {
		FiberManager manager;
		bool fiber_started = false;
		bool fiber_resumed = false;
		manager.launch([&]() {
			fiber_started = true;
			Fiber::yield();
			fiber_resumed = true;
		});
		manager.update(GameTime());
		manager.update(GameTime());
		TEST(fiber_started).should == true;
		TEST(fiber_resumed).should == true;
	});
	
	it("should not wake up a fiber until it is done sleeping", [](){
		FiberManager manager;
		bool fiber_started = false;
		bool fiber_timeout = false;
		manager.launch([&]() {
			fiber_started = true;
			Fiber::sleep(GameTime::seconds(2.f));
			fiber_timeout = true;
		});
		manager.update(GameTime());
		TEST(fiber_started).should == true;
		manager.update(GameTime() + GameTime::seconds(1.f));
		TEST(fiber_timeout).should == false;
		manager.update(GameTime() + GameTime::seconds(2.f));
		TEST(fiber_timeout).should == true;
	});
	
	it("should schedule fibers according to timeout", []() {
		FiberManager manager;
		bool fiber1_timeout = false;
		bool fiber2_timeout = false;
		manager.launch([&]() {
			Fiber::sleep(GameTime::seconds(2.f));
			fiber1_timeout = true;
		});
		manager.launch([&]() {
			Fiber::sleep(GameTime::seconds(1.f));
			fiber2_timeout = true;
		});
		manager.update(GameTime());
		TEST(fiber1_timeout).should == false;
		TEST(fiber2_timeout).should == false;
		manager.update(GameTime() + GameTime::seconds(1.f));
		TEST(fiber1_timeout).should == false;
		TEST(fiber2_timeout).should == true;
		manager.update(GameTime() + GameTime::seconds(2.f));
		TEST(fiber1_timeout).should == true;
	});
	
	it("should call destructors when terminating a fiber", []() {
		FiberManager manager;
		bool destructor_called = false;
		struct SetTrueOnDestroy {
			bool& b;
			SetTrueOnDestroy(bool& b) : b(b) {}
			~SetTrueOnDestroy() { b = true; }
		};
		manager.launch([&]() {
			SetTrueOnDestroy scoped(destructor_called);
			Fiber::terminate();
		});
		manager.update(GameTime());
		TEST(destructor_called).should == true;
	});
	
	it("should remove terminated fibers from the update queue", []() {
		FiberManager manager;
		int num_resumes = 0;
		manager.launch([&]() {
			++num_resumes;
		});
		manager.update(GameTime());
		TEST(num_resumes).should == 1;
		manager.update(GameTime() + GameTime::seconds(1.f));
		TEST(num_resumes).should == 1;
	});
	
	it("should not corrupt the fiber stack", []() {
		FiberManager manager;
		bool good = true;
		manager.launch([&]() {
			DEFINE_STACK_ARRAY(uint64, numbers, 1024);
			for (uint64 i = 0; i < 1024; ++i) {
				numbers[i] = i;
			}
			Fiber::yield();
			for (uint64 i = 0; i < 1024; ++i) {
				if (numbers[i] != i) {
					good = false;
					return;
				}
			}
		});
		
		manager.update(GameTime());
		
		// See if we can corrupt the stack.
		auto f = []() -> uint64 {
			DEFINE_STACK_ARRAY(uint64, numbers, 1024);
			for (uint64 i = 0; i < 1024; ++i) {
				numbers[i] = ~i;
			}
			return numbers[1023];
		};
		uint64 r = f();
		TEST(r).should == ~(1023ULL);
		manager.update(GameTime());
		TEST(good).should == true;
	});
	
	it("should rethrow unhandled exceptions in host process", []() {
		FiberManager manager;
		struct TestException {
			const char* what = "Hello World!";
		};
		
		manager.launch([&]() {
			throw TestException();
		});
		should_throw_exception<TestException>([&]() {
			manager.update(GameTime());
		});
		
		manager.launch([&]() {
			throw TestException();
		});
	});
	
	it("should catch rethrown exception in the host process", []() {
		FiberManager manager;
		struct TestException {
			const char* what = "Hello World!";
		};
		manager.launch([&]() {
			throw TestException();
		});
		bool good = false;
		try {
			manager.update(GameTime());
		}
		catch (const TestException& ex) {
			good = true;
			TEST(String(ex.what)).should == "Hello World!";
		}
		catch (...) {
			good = false;
		}
		TEST(good).should == true;
	});
}

#endif
