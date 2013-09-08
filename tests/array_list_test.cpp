//
//  array_list_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 24/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/array_list.hpp"
#include "io/formatters.hpp"

using namespace grace;

static size_t num_constructed = 0;
static bool move_constructed = false;

SUITE(ArrayList) {
    it("should add elements with push_back", []() {
        ArrayList<int> list;
        int ints[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        for (size_t i = 0; i < 10; ++i) {
            list.push_back(ints[i]);
        }
        TEST(list.size()).should == 10;
    });
    
    it("should iterate over elements", []() {
        ArrayList<int> list;
        int ints[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        for (size_t i = 0; i < 10; ++i) {
            list.push_back(ints[i]);
        }
        
        int sum = 0;
        for (auto& n: list) {
            sum += n;
        }
    });
	
	it("should not throw an exception when push_back is called", [](){
		ArrayList<int> a;
		should_not_throw_any_exception([&](){
			a.push_back(123);
		});
		should_return<int>(123, [&]() { return a[0]; });
	});
	
	it("should throw IndexOutOfBoundsException when array element is accessed out of bounds", []() {
		ArrayList<int> a;
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
			Foo& operator=(const Foo&) = default;
		};
		
		ArrayList<Foo> array;
		array.resize(100);
		auto n = num_constructed;
		num_constructed = 0;
		TEST(n).should == 101;
	});
	
	it("should move-assign elements on push_back", []() {
		struct Foo {
			Foo() { ++num_constructed; }
			Foo& operator=(Foo&& f) { move_constructed = true; return *this; }
		};
		ArrayList<Foo> array;
		array.push_back(Foo());
		auto m = move_constructed;
		move_constructed = false;
		auto n = num_constructed;
		num_constructed = 0;
		TEST(m).should == true;
		TEST(n).should == 2; // TODO: Consider if it can be done without intermediates
	});
	
	it("should correctly grow array on insertion at end", []() {
		std::vector<int> v = {1, 2, 3, 4, 5};
		ArrayList<int> a = {2, 3, 4};
		a.insert(v.begin(), v.end());
		TEST(a.size()).should == 8;
		TEST(a).should == ArrayList<int>({2, 3, 4, 1, 2, 3, 4, 5});
	});
	
	it("should correctly grow on insertion at beginning", []() {
		std::vector<int> v = {1, 2, 3, 4, 0};
		ArrayList<int> a = {2, 3, 4};
		a.insert(v.begin(), v.end(), a.begin());
		TEST(a.size()).should == 8;
		TEST(a).should == ArrayList<int>({1, 2, 3, 4, 0, 2, 3, 4});
	});
	
	it("should correctly grow on insertion in the middle", []() {
		std::vector<int> v = {1, 2, 3};
		ArrayList<int> a = {1, 2, 3, 4, 5};
		a.insert(v.begin(), v.end(), a.begin()+2);
		TEST(a.size()).should == 8;
		TEST(a).should == ArrayList<int>({1, 2, 1, 2, 3, 3, 4, 5});
	});
	
	it("should erase last element by iterator", []() {
		// Erase back
		ArrayList<int> a = {1, 2, 3, 4};
		a.erase(a.end()-1);
		TEST(a).should == ArrayList<int>{1, 2, 3};
	});
	
	it("should erase first element by iterator", []() {
		// Erase front
		ArrayList<int> b = {1, 2, 3, 4};
		b.erase(b.begin());
		TEST(b).should == ArrayList<int>{2, 3, 4};
	});
	
	it("should erase element in the middle by iterator", []() {
		// Erase middle
		ArrayList<int> c = {1, 2, 3, 4};
		c.erase(c.begin() + 2);
		TEST(c).should == ArrayList<int>{1, 2, 4};
	});
	
	it("should throw an exception when trying to erase end()", []() {
		ArrayList<int> a = {1, 2, 3};
		should_throw_exception<IndexOutOfBoundsException>([&]() {
			a.erase(a.end());
		});
	});

	it("should subtract iterators correctly", []() {
		ArrayList<int> a;
		for (int i = 0; i < 10000; ++i) {
			a.push_back(i);
		}
		auto size = a.end() - a.begin();
		TEST(size).should == a.size();
	});
	
	it("should skip iterators ahead correctly", []() {
		ArrayList<int> a;
		for (int i = 0; i < 10000; ++i) {
			a.push_back(i);
		}
		auto i = a.begin() + 5000;
		TEST(*i).should == 5000;
	});
	
	it("should rewind iterators from the end", []() {
		ArrayList<int> a;
		for (int i = 0; i < 10000; ++i) {
			a.push_back(i);
		}
		auto i = a.end() - 5000;
		TEST(*i).should == 5000;
	});
}
