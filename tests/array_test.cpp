//
//  array_test.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_array_test_hpp
#define grace_array_test_hpp

#include "tests/test.hpp"
#include <vector>

using namespace grace;

static size_t num_constructed = 0;
static bool move_constructed = false;

SUITE(Array) {
	it("should not throw an exception when push_back is called", [](){
		Array<int> a;
		should_not_throw_any_exception([&](){
			a.push_back(123);
		});
		should_return<int>(123, [&]() { return a[0]; });
	});
	
	it("should throw IndexOutOfBoundsException when array element is accessed out of bounds", []() {
		Array<int> a;
		should_throw_any_exception([&]() {
			int n = a[0];
		});
		should_throw_exception<IndexOutOfBoundsException>([&]() {
			int n = a[0];
		});
	});
	
	it("should not construct elements on reserve", []() {
		struct Foo {
			Foo() { ++num_constructed; }
			Foo(Foo&&) = default;
		};
		Array<Foo> array;
		array.reserve(100);
		auto n = num_constructed;
		num_constructed = 0;
		TEST(n).should == 0;
	});
	
	it("should construct elements on resize", []() {
		struct Foo {
			Foo() { ++num_constructed; }
			Foo(const Foo&) { ++num_constructed; }
			Foo(Foo&&) = default;
		};
		
		Array<Foo> array;
		array.resize(100);
		auto n = num_constructed;
		num_constructed = 0;
		TEST(n).should == 101;
	});
	
	it("should move-construct elements on push_back", []() {
		struct Foo {
			Foo() { ++num_constructed; }
			Foo(Foo&& f) { move_constructed = true; }
		};
		Array<Foo> array;
		array.push_back(Foo());
		auto m = move_constructed;
		move_constructed = false;
		auto n = num_constructed;
		num_constructed = 0;
		TEST(m).should == true;
		TEST(n).should == 1;
	});
	
	it("should correctly grow array on insertion at end", []() {
		std::vector<int> v = {1, 2, 3, 4, 5};
		Array<int> a = {2, 3, 4};
		a.insert(v.begin(), v.end());
		TEST(a.size()).should == 8;
		TEST(a).should == Array<int>({2, 3, 4, 1, 2, 3, 4, 5});
	});
	
	it("should correctly grow on insertion at beginning", []() {
		std::vector<int> v = {1, 2, 3, 4, 0};
		Array<int> a = {2, 3, 4};
		a.insert(v.begin(), v.end(), a.begin());
		TEST(a.size()).should == 8;
		TEST(a).should == Array<int>({1, 2, 3, 4, 0, 2, 3, 4});
	});
	
	it("should correctly grow on insertion in the middle", []() {
		std::vector<int> v = {1, 2, 3};
		Array<int> a = {1, 2, 3, 4, 5};
		a.insert(v.begin(), v.end(), a.begin() + 2);
		TEST(a.size()).should == 8;
		TEST(a).should == Array<int>({1, 2, 1, 2, 3, 3, 4, 5});
	});
	
	it("should erase last element by iterator", []() {
		// Erase back
		Array<int> a = {1, 2, 3, 4};
		Array<int>::iterator it = a.erase(a.end()-1);
		TEST(it).should == a.end();
		TEST(a).should == Array<int>{1, 2, 3};
	});
	
	it("should erase first element by iterator", []() {
		// Erase front
		Array<int> b = {1, 2, 3, 4};
		Array<int>::iterator itb = b.erase(b.begin());
		TEST(itb).should == b.begin();
		TEST(b).should == Array<int>{2, 3, 4};
	});
	
	it("should erase element in the middle by iterator", []() {
		// Erase middle
		Array<int> c = {1, 2, 3, 4};
		Array<int>::iterator itc = c.erase(c.begin() + 2);
		TEST(itc).should == c.begin() + 2;
		TEST(c).should == Array<int>{1, 2, 4};
	});
	
	it("should throw an exception when trying to erase end()", []() {
		Array<int> a = {1, 2, 3};
		should_throw_exception<IndexOutOfBoundsException>([&]() {
			a.erase(a.end());
		});
	});
	
	it("should be faster than std::vector", []() {
		auto build_array = []() {
			Array<int> a;
			for (int i = 0; i < 1000; ++i) {
				a.push_back(i);
			}
		};
		auto build_vector = []() {
			std::vector<int> v;
			for (int i = 0; i < 1000; ++i) {
				v.push_back(i);
			}
		};
		TEST(build_array).should_be FASTER_THAN(build_vector);
	});
}


#endif
