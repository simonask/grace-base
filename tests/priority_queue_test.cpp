//
//  priority_queue_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 17/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"

#include "base/priority_queue.hpp"

using namespace grace;

SUITE(PriorityQueue) {
	it("should insert elements without throwing exceptions", []() {
		PriorityQueue<int> q;
		for (int i: {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
			q.insert(i);
		}
		TEST(q).should == Array<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
	});
	
	it("should order inserted elements, reversely sorted", []() {
		PriorityQueue<int> q;
		for (int i: {10, 9, 8, 7, 6, 5, 4, 3, 2, 1}) {
			q.insert(i);
		}
		TEST(q).should == Array<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
	});
	
	it("should order inserted elements, randomly sorted", []() {
		PriorityQueue<int> q;
		for (int i: {1, 10, 2, 9, 3, 8, 4, 7, 5, 6}) {
			q.insert(i);
		}
		TEST(q).should == Array<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
	});
}
