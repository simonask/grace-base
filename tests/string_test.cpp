//
//  string_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/string.hpp"
#include "base/map.hpp"

using namespace grace;

SUITE(String) {
#if defined(__has_feature) && __has_feature(cxx_user_literals)
	it("should create immutable StringRefs with _C suffix", []() {
		auto str = "Hello World"_C;
		TEST(str).should == "Hello World";
	});
#endif

	it("should construct a string with implicit constructor", []() {
		String s = "Hello World!";
		TEST(s.size()).should == 12;
	});
	
	it("should construct a string from a pointer", []() {
		static const char hello[] = "Hello World!";
		String s(hello + 6, 5);
		TEST(s.size()).should == 5;
		TEST(s).should == "World";
	});
	
	it("should assign a bare C-string", []() {
		String s;
		s = "Hello World!";
		TEST(s.size()).should == 12;
		TEST(s).should == "Hello World!";
	});
	
	it("should move-construct", []() {
		String s("Hello World!");
		String s2 = std::move(s);
		TEST(s.size()).should == 0;
		TEST(s).should == "";
		TEST(s2.size()).should == 12;
		TEST(s2).should == "Hello World!";
	});
	
	it("should move-assign", []() {
		String s("Hello World!");
		String s2 = s;
		TEST(s.size()).should == 12;
		TEST(s).should == "Hello World!";
		TEST(s2.size()).should == s.size();
		TEST(s2).should == s;
	});
	
	it("should compare strings of same length", []() {
		String a = "abc";
		String b = "bca";
		ssize_t n[] = {a.compare(b), b.compare(a), a.compare(a)};
		TEST(n[0]).should < 0;
		TEST(n[1]).should > 0;
		TEST(n[2]).should == 0;
		should_be_less_than(a, b);
		should_be_greater_than(b, a);
		should_be_equal(a, a);
		should_be_equal(b, b);
	});
	
	it("should compare strings of different lengths", []() {
		String a = "abc";
		String b = "abcdef";
		ssize_t n[] = {a.compare(b), b.compare(a)};
		TEST(n[0]).should < 0;
		TEST(n[1]).should > 1;
		should_be_less_than(a, b);
		should_be_greater_than(b, a);
	});
	
	it("should work as a map key", []() {
		Map<String, int> map;
		map["foo"] = 1;
		map["bar"] = 2;
		TEST(map["foo"]).should == 1;
		TEST(map["bar"]).should == 2;
	});
}