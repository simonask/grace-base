//
//  maxarray_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 31/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/maxarray.hpp"

using namespace grace;

static size_t num_constructed = 0;
static bool move_constructed = false;

SUITE(MaxArray) {
	it("should not throw an exception when push_back is called", [](){
		MaxArray<int, 10> a;
		should_not_throw_any_exception([&](){
			a.push_back(123);
		});
		should_return<int>(123, [&]() { return a[0]; });
	});
	
	it("should throw IndexOutOfBoundsException when array element is accessed out of bounds", []() {
		MaxArray<int, 10> a;
		should_throw_any_exception([&]() {
			int n = a[0];
		});
		should_throw_exception<IndexOutOfBoundsException>([&]() {
			int n = a[0];
		});
	});
	
	it("should construct elements on resize", []() {
		struct Foo {
			Foo() { ++num_constructed; }
			Foo(const Foo&) { ++num_constructed; }
			Foo(Foo&&) = default;
		};
		
		MaxArray<Foo, 100> array;
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
		MaxArray<Foo, 10> array;
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
		MaxArray<int, 10> a = {2, 3, 4};
		a.insert(v.begin(), v.end());
		TEST(a.size()).should == 8;
		int correct[] = {2, 3, 4, 1, 2, 3, 4, 5};
		TEST(a).should == ArrayRef<const int>(correct);
	});
	
	it("should correctly grow on insertion at beginning", []() {
		std::vector<int> v = {1, 2, 3, 4, 0};
		MaxArray<int, 10> a = {2, 3, 4};
		a.insert(v.begin(), v.end(), a.begin());
		TEST(a.size()).should == 8;
		int correct[] = {1, 2, 3, 4, 0, 2, 3, 4};
		TEST(a).should == ArrayRef<const int>(correct);
	});
	
	it("should correctly grow on insertion in the middle", []() {
		std::vector<int> v = {1, 2, 3};
		MaxArray<int, 10> a = {1, 2, 3, 4, 5};
		a.insert(v.begin(), v.end(), a.begin() + 2);
		TEST(a.size()).should == 8;
		int correct[] = {1, 2, 1, 2, 3, 3, 4, 5};
		TEST(a).should == ArrayRef<const int>(correct);
	});
	
	it("should erase last element by iterator", []() {
		// Erase back
		MaxArray<int, 10> a = {1, 2, 3, 4};
		MaxArray<int, 10>::iterator it = a.erase(a.end()-1);
		TEST(it).should == a.end();
		int correct[] = {1, 2, 3};
		TEST(a).should == ArrayRef<const int>{correct};
	});
	
	it("should erase first element by iterator", []() {
		// Erase front
		MaxArray<int, 10> b = {1, 2, 3, 4};
		MaxArray<int, 10>::iterator itb = b.erase(b.begin());
		TEST(itb).should == b.begin();
		int correct[] = {2, 3, 4};
		TEST(b).should == ArrayRef<const int>{correct};
	});
	
	it("should erase element in the middle by iterator", []() {
		// Erase middle
		MaxArray<int, 10> c = {1, 2, 3, 4};
		MaxArray<int, 10>::iterator itc = c.erase(c.begin() + 2);
		TEST(itc).should == c.begin() + 2;
		int correct[] = {1, 2, 4};
		TEST(c).should == ArrayRef<const int>{correct};
	});
	
	it("should throw an exception when trying to erase end()", []() {
		MaxArray<int, 10> a = {1, 2, 3};
		should_throw_exception<IndexOutOfBoundsException>([&]() {
			a.erase(a.end());
		});
	});
	
	it("should be faster than std::vector", []() {
		auto build_array = []() {
			MaxArray<int, 1000> a;
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
