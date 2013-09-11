//
//  function_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 21/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/function.hpp"
#include "base/function_error.hpp"

using namespace grace;

namespace {
	static int g_Number = 0;
	void free_function(int n) {
		g_Number = n;
	}
}

SUITE(Function) {
	it("should invoke free functions", [](){
		TEST(g_Number).should != 123;
		Function<void(int)> f = free_function;
		f(123);
		TEST(g_Number).should == 123;
		g_Number = 0;
	});
	
	it("should invoke lambda functions", []() {
		int lololol = 0;
		auto lambda = [&](int n) {
			lololol = n;
		};
		Function<void(int)> f = lambda;
		f(123);
		TEST(lololol).should == 123;
	});
	
	it("should invoke function objects", []() {
		struct Foo {
			int number = 0;
			void operator()(int n) {
				number = n;
			}
		};
		Foo foo;
		Function<void(int)> f = std::ref<Foo>(foo);
		f(123);
		TEST(foo.number).should == 123;
	});
	
	it("should copy function objects to own storage", []() {
		struct Foo {
			int number = 0;
			void operator()(int n) {
				number = n;
			}
		};
		Foo foo;
		Function<void(int)> f = foo;
		f(123);
		TEST(foo.number).should == 0;
	});
	
	it("should destroy function objects when going out of scope", []() {
		static int num_foos = 0;
		struct Foo {
			Foo() { num_foos++; }
			Foo(const Foo&) { num_foos++; }
			~Foo() { num_foos--; }
			void operator()(int n) {
				
			}
		};
		Foo foo;
		TEST(num_foos).should == 1;
		{
			Function<void(int)> f = foo;
			TEST(num_foos).should == 2;
		}
		TEST(num_foos).should == 1;
	});
	
	it("should correctly align guts to support pointer tagging", []() {
		LinearAllocator alloc(4096);
		void* some_garbage = alloc.allocate(9, 1); // Make sure the internal offsets in the allocator are uneven
		int number = 0;
		auto lambda = [&]() {
			number = 123;
		};
		Function<void()> foo(move(lambda), alloc);
		foo();
		TEST(number).should == 123;
	});
	
	it("should throw exception on empty call", []() {
		Function<void()> f;
		should_throw_exception<EmptyFunctionCallError>([&]() {
			f();
		});
	});
}
