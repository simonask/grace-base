//
//  either_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_either_test_cpp
#define grace_either_test_cpp

#include "tests/test.hpp"

#include "base/either.hpp"

using namespace grace;

static int g_NumLiveFooObjects = 0;

SUITE(Either) {
	it("should accept single argument", []() {
		Either<int> n(123);
	});
	
	it("should be able to act as a Maybe replacement", []() {
		Either<int, NothingType> n(123);
		TEST(n.is_a<int>()).should == true;
		n = Nothing;
		TEST(n.is_nothing()).should == true;
	});
	
	it("should call destructors", []() {
		struct Foo {
			Foo() { ++g_NumLiveFooObjects; }
			Foo(Foo&&) { ++g_NumLiveFooObjects; }
			Foo(const Foo&) { ++g_NumLiveFooObjects; }
			~Foo() { --g_NumLiveFooObjects; }
		};
		
		{
			Either<Foo, NothingType> foo((Foo()));
		}
		
		TEST(g_NumLiveFooObjects).should == 0;
	});
	
	it("should encapsulate non-copyable objects", []() {
		struct Foo {
			Foo() {}
			Foo(const Foo&) = delete;
			Foo(Foo&&) = default;
			Foo& operator=(const Foo&) = delete;
			Foo& operator=(Foo&&) = default;
		};
		
		Either<Foo, NothingType> foo{Foo()};
		Either<Foo, NothingType> foo2{move(foo)};
		Either<Foo, NothingType> foo3{Nothing};
		foo3 = move(foo2);
	});
	
	it("should encapsulate non-movable objects", []() {
		struct Foo {
			Foo() {}
			Foo(const Foo&) = default;
			Foo& operator=(const Foo&) = default;
			Foo& operator=(Foo&&) = delete;
		};
		
		Either<Foo, NothingType> foo{Foo()};
		Either<Foo, NothingType> foo2{foo};
		Either<Foo, NothingType> foo3{Nothing};
		foo3 = foo2;
	});
	
	it("should convert to bool if it can be a NothingType", []() {
		Either<NothingType, int> x{Nothing};
		TEST((bool)x).should == false;
		x = 123;
		TEST((bool)x).should == true;
	});
	
	it("should support control flow with 'when' and 'otherwise'", []() {
		Either<int, StringRef> x(123);
		bool when_int_called = false;
		bool otherwise_called = false;
		x.when<int>([&](int) {
			when_int_called = true;
		}).otherwise([&]() {
			otherwise_called = true;
		});
		TEST(when_int_called).should == true;
		TEST(otherwise_called).should == false;
		when_int_called = false;
		bool when_stringref_called = false;
		x = StringRef("Hello World!");
		x.when<int>([&](int) {
			when_int_called = true;
		}).when<StringRef>([&](StringRef) {
			when_stringref_called = true;
		}).otherwise([&]() {
			otherwise_called = true;
		});
		TEST(when_int_called).should == false;
		TEST(when_stringref_called).should == true;
		TEST(otherwise_called).should == false;
		x.when<int>([&](int) {
			when_int_called = true;
		}).otherwise([&]() {
			otherwise_called = true;
		});
		TEST(otherwise_called).should == true;
	});
	
	it("should visit different types", []() {
		using E = Either<int32, float32, StringRef>;
		struct Visitor {
			bool int32_called = false;
			bool float32_called = false;
			bool stringref_called = false;
			void operator()(int32 n) {
				int32_called = true;
			}
			void operator()(float32 f) {
				float32_called = true;
			}
			void operator()(StringRef str) {
				stringref_called = true;
			}
		};
		
		E e_int32(123);
		E e_float32(123.f);
		E e_stringref(StringRef("Hello World!"));
		Visitor v;
		e_int32.visit(v);
		e_float32.visit(v);
		e_stringref.visit(v);
		TEST(v.int32_called).should == true;
		TEST(v.float32_called).should == true;
		TEST(v.stringref_called).should == true;
	});

	feature("either_switch", []() {
		Either<int, StringRef> e {StringRef("Hello, World!")};
		Either<int, StringRef> f {123};

		bool e_was_int = false;
		bool e_was_string = false;
		either_switch(e,
			[&](int n) {
				e_was_int = true;
			},
			[&](StringRef s) {
				e_was_string = true;
			}
		);
		TEST(e_was_int).should == false;
		TEST(e_was_string).should == true;

		bool f_was_int = false;
		bool f_was_string = false;
		either_switch(f,
			[&](int n) {
				f_was_int = true;
			},
			[&](StringRef s) {
				f_was_string = true;
			}
		);
		TEST(f_was_int).should == true;
		TEST(f_was_string).should == false;
	});

	feature("operator>>", []() {
		Either<int, StringRef> e {StringRef("Hello, World!")};
		Either<int, StringRef> f {123};

		int n;
		StringRef s;

		bool e_was_int = false;
		bool e_was_string = false;
		bool f_was_int = false;
		bool f_was_string = false;

		if (e >> n) {
			e_was_int = true;
		} else if (e >> s) {
			e_was_string = true;
		}

		if (f >> n) {
			f_was_int = true;
		} else if (f >> s) {
			f_was_string = true;
		}

		TEST(e_was_int).should == false;
		TEST(e_was_string).should == true;

		TEST(f_was_int).should == true;
		TEST(f_was_string).should == false;
	});
}

#endif
