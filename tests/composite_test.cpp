//
//  composite_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "object/reflect.hpp"
#include "object/composite_type.hpp"


using namespace grace;

struct Foo : Object {
	REFLECT;
	void foo_slot(int n) { number = n; }
	int32 number = 0;
};

BEGIN_TYPE_INFO(Foo)
	property(&Foo::number, "number", "Foo number.");
	slot(&Foo::foo_slot, "foo_slot", "Foo slot.");
END_TYPE_INFO()

struct Bar : Object {
	REFLECT;
	void bar_slot(int n) { number = n; }
	int32 number = 0;
};

BEGIN_TYPE_INFO(Bar)
	property(&Bar::number, "number", "Bar number.");
	slot(&Bar::bar_slot, "bar_slot", "Bar slot.");
END_TYPE_INFO()

SUITE(Composite) {
	TestUniverse uni;
	
	it("should compose objects", [&]() {
		CompositeType* ct = new CompositeType(default_allocator(), "Comp01T");
		ct->add_aspect(get_type<Foo>());
		ct->add_aspect(get_type<Bar>());
		ct->freeze();
		ObjectPtr<> o = uni.create_object(ct, "Comp01");
		auto o_foo = aspect_cast<Foo>(o);
		auto o_bar = aspect_cast<Bar>(o);
		TEST(o_foo).should != nullptr;
		TEST(o_bar).should != nullptr;
		uni.clear();
		delete ct;
	});
	
	it("should expose attributes", [&]() {
		CompositeType* ct = new CompositeType(default_allocator(), "Comp02T");
		ct->add_aspect(get_type<Foo>());
		ct->add_aspect(get_type<Bar>());
		ct->expose_attribute(0, "number");
		ct->freeze();
		ObjectPtr<> o = uni.create_object(ct, "Comp02");
		auto attr = ct->find_attribute_by_name("number");
		TEST(attr).should != nullptr;
		bool r = attr->set_any(o.get(), 123);
		TEST(r).should == true;
		auto o_foo = aspect_cast<Foo>(o);
		TEST(o_foo->number).should == 123;
		Any v = attr->get_any(o.get());
		TEST(v.is_a<int32>()).should == true;
		uni.clear();
		delete ct;
	});
	
	it("should expose slots", [&]() {
		CompositeType* ct = new CompositeType(default_allocator(), "Comp03T");
		ct->add_aspect(get_type<Foo>());
		ct->add_aspect(get_type<Bar>());
		ct->expose_slot(0, "foo_slot");
		ct->expose_slot(1, "bar_slot");
		ct->freeze();
		ObjectPtr<> o = uni.create_object(ct, "Comp03");
		auto foo_slot = ct->find_slot_by_name("foo_slot");
		auto bar_slot = ct->find_slot_by_name("bar_slot");
		TEST(foo_slot).should != nullptr;
		TEST(bar_slot).should != nullptr;
		ObjectPtr<Foo> o_foo = aspect_cast<Foo>(o);
		ObjectPtr<Bar> o_bar = aspect_cast<Bar>(o);
		Array<Any> args;
		args.push_back(123);
		bool rf = foo_slot->invoke(o, args);
		TEST(rf).should == true;
		TEST(o_foo->number).should == 123;
		bool rb = bar_slot->invoke(o, args);
		TEST(rb).should == true;
		TEST(o_bar->number).should == 123;
		uni.clear();
		delete ct;
	});
}
