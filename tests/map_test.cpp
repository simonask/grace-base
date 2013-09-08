//
//  map_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 10/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/map.hpp"

using namespace grace;

SUITE(Map) {
	it("should map integers with set", []() {
		Map<int, int> m;
		m.set(1, 2);
		m.set(3, 4);
		m.set(1, 4);
		TEST(m.size()).should == 2;
		TEST(m[1]).should == 4;
		TEST(m[3]).should == 4;
	});
	
	it("should map integers with subscript operator", []() {
		Map<int, int> m;
		m[1] = 2;
		m[2] = 3;
		m[1] = 4;
		TEST(m.size()).should == 2;
		TEST(m[1]).should == 4;
		TEST(m[2]).should == 3;
	});
	
	it("should order keys naturally", []() {
		Array<int> keys = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
		Array<int> values = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		Map<int, int> m(keys, values);
		static const int correct[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		TEST(m.keys()).should == ArrayRef<const int>(correct);
	});
	
	it("should map strings with literals and StringRefs", []() {
		Map<String, int> m;
		m["foo"] = 123;
		TEST(m["foo"]).should == 123;
		StringRef s = "Hello World!";
		m[s] = 10;
		TEST(m[s]).should == 10;
	});
	
	it("should erase elements by iterator", []() {
		Map<int, int> m;
		m[1] = 10;
		m[2] = 100;
		m[0] = 1000;
		m.erase(m.begin());
		TEST(m.find(0) == m.end()).should == true;
	});
	
	it("should erase elements by key", []() {
		Map<int, int> m;
		m[1] = 10;
		m[2] = 100;
		m[0] = 1000;
		m.erase(2);
		TEST(m.find(2) == m.end()).should == true;
	});
}
