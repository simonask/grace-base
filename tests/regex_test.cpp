//
//  regex_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 28/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/regex.hpp"

using namespace grace;

SUITE(Regex) {
	String foo = "The quick brown fox jumps over the lazy dog.";
	String bar = "aaaaaaaabbbbbbb";
	
	it("should match beginning of the string", [=]() {
		Regex r = Regex("The quick");
		TEST(r.match(foo)).should == true;
	});
	
	it("should match middle of the string", [=]() {
		Regex r = "brown fox";
		TEST(r.match(foo)).should == true;
	});
	
	it("should match the end of the string", [=]() {
		Regex r = "lazy dog.";
		TEST(r.match(foo)).should == true;
	});
	
	it("should match complex sequence", [=]() {
		Regex r = "[ab]*";
		TEST(r.match(bar)).should == true;
	});
}
