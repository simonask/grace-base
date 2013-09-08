//
//  signal_test.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_signal_test_hpp
#define grace_signal_test_hpp

#include "tests/test.hpp"

#include "object/signal.hpp"
#include "object/object.hpp"
#include "object/reflect.hpp"
#include "object/universe.hpp"

using namespace grace;

static bool raw_function_called = false;
static void raw_function() {
	raw_function_called = true;
}


class TestSignalObject : public grace::Object {
	REFLECT;
public:
	bool slot_called = false;
	mutable bool const_slot_called = false;
	
	void slot() {
		slot_called = true;
	}
	void const_slot() const {
		const_slot_called = true;
	}
};
	
BEGIN_TYPE_INFO(TestSignalObject)
slot(&TestSignalObject::slot, "slot", "A slot.");
slot(&TestSignalObject::const_slot, "const_slot", "A const slot");
END_TYPE_INFO()


SUITE(Signal) {
	it("should call raw functions", []() {
		Signal<> signal;
		signal.connect(raw_function);
		signal();
		TEST(raw_function_called).should == true;
		raw_function_called = false; // reset
	});
	
	it("should call member functions", []() {
		Signal<> signal;
		
		struct Foo {
			bool called = false;
			void member_function() {
				called = true;
			}
		};
		
		Foo foo;
		signal.connect(&foo, &Foo::member_function);
		signal();
		TEST(foo.called).should == true;
	});
	
	it("should call const member functions", []() {
		Signal<> signal;
		
		struct Foo {
			mutable bool called = false;
			void member_function() const {
				called = true;
			}
		};
		
		Foo foo;
		signal.connect(&foo, &Foo::member_function);
		signal();
		TEST(foo.called).should == true;
	});
	
	it("should call lambda functions", []() {
		Signal<> signal;
		bool lambda_called = false;
		signal.connect([&]() {
			lambda_called = true;
		});
		signal();
		TEST(lambda_called).should == true;
	});
	
	it("should call Object slots", []() {
		Signal<> signal;
		TestUniverse universe;
		auto o = aspect_cast<TestSignalObject>(universe.create_object_and_set_as_root(get_type<TestSignalObject>(), "TestSignalObject01"));
		signal.connect(o, &TestSignalObject::slot);
		signal();
		TEST(o->slot_called).should == true;
	});
	
	it("should call const Object slots", []() {
		Signal<> signal;
		TestUniverse universe;
		auto o = aspect_cast<TestSignalObject>(universe.create_object_and_set_as_root(get_type<TestSignalObject>(), "TestSignalObject01"));
		signal.connect(o, &TestSignalObject::const_slot);
		signal();
		TEST(o->const_slot_called).should == true;
	});
	
	it("should call Object slots by name", []() {
		Signal<> signal;
		TestUniverse universe;
		auto o = aspect_cast<TestSignalObject>(universe.create_object_and_set_as_root(get_type<TestSignalObject>(), "TestSignalObject01"));
		signal.connect(o, "slot");
		signal();
		TEST(o->slot_called).should == true;
	});
	
	it("should call const Object slots by name", []() {
		Signal<> signal;
		TestUniverse universe;
		auto o = universe.create_root<TestSignalObject>("TestSignalObject01");
		signal.connect(o, "const_slot");
		signal();
		TEST(o->const_slot_called).should == true;
	});
	
	it("should fail to connect slots by name of different type", []() {
		Signal<int> signal;
		TestUniverse universe;
		auto o = universe.create_root<TestSignalObject>("TestSignalObject01");
		auto connected = signal.connect(o, "slot");
		TEST(connected.is_set()).should == false;
	});
	
	it("should pass the correct value to slots", []() {
		Signal<int> signal;
		int received = -1;
		signal.connect([&](int n) {
			received = n;
		});
		signal(123);
		TEST(received).should == 123;
	});
	
	it("should recognize raw member function connections as named slot connections", []() {
		Signal<> signal;
		TestUniverse universe;
		auto o = universe.create_root<TestSignalObject>("TestSignalObject01");
		signal.connect(o, &TestSignalObject::slot);
		auto connection = signal.begin();
		auto slot_attribute = connection->slot();
		TEST(slot_attribute).should != nullptr;
		if (slot_attribute != nullptr) {
			TEST(slot_attribute->name()).should == "slot";
		}
	});
	
	it("should connect signals to slots of compatible type", []() {
		Signal<int> signal;
		bool fired1 = false;
		signal.connect([&](const int& n) {
			fired1 = true;
		});
		bool fired2 = false;
		signal.connect([&](int&& n) {
			fired2 = true;
		});
		signal(123);
		TEST(fired1).should == true;
		TEST(fired2).should == true;
	});
	
	it("should connect signals to slots that return values", []() {
		Signal<int> signal;
		bool fired = false;
		signal.connect([&](int n) -> double {
			fired = true;
			return n;
		});
		signal(123);
		TEST(fired).should == true;
	});
	
	it("should disconnect a single signal", []() {
		Signal<> signal;
		int fired = 0;
		auto connector = signal.connect([&]() {
			++fired;
		});
		signal();
		signal();
		signal.disconnect(connector);
		signal();
		TEST(fired).should == 2;
	});
	
	it("should be reentrant when processing multiple signals and one disconnects in the middle of processing", []() {
		Signal<> signal;
		int fired1 = 0;
		int fired2 = 0;
		int fired3 = 0;
		
		SignalConnectionID conn1 = signal.connect([&]() {
			++fired1;
			if (fired1 == 1) {
				signal.disconnect(conn1);
			}
		});
		SignalConnectionID conn2 = signal.connect([&]() {
			++fired2;
			if (fired2 == 2) {
				signal.disconnect(conn2);
			}
		});
		SignalConnectionID conn3 = signal.connect([&]() {
			++fired3;
			if (fired3 == 3) {
				signal.disconnect(conn3);
			}
		});
		
		signal();
		signal();
		signal();
		signal();
		
		TEST(fired1).should == 1;
		TEST(fired2).should == 2;
		TEST(fired3).should == 3;
	});
}

#endif
