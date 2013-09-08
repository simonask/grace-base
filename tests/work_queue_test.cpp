//
//  work_queue_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 17/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"

#include "base/work_queue.hpp"

using namespace grace;

SUITE(WorkQueue) {
	it("should process all work in a separate thread", []() {
		int processed = 0;
		WorkQueue<int> q([&](int& n) {
			processed += 1;
		});
		
		for (int i: {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
			q.schedule(i);
		}
		
		q.wait_until_empty();
		TEST(processed).should == 10;
	});
	
	it("should process work at the correct time", []() {
		Array<int> processed;
		WorkQueue<int> q([&](int& n) {
			processed.push_back(n);
		});
		
		Array<int>        numbers = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
		Array<SystemTime> times;
		times.reserve(numbers.size());
		auto now = system_now();
		for (int n: numbers) {
			times.push_back(now + SystemTime::milliseconds(n));
		}
		
		q.schedule(numbers.begin(), numbers.end(), times.begin(), times.end());
		q.wait_until_empty();
		
		TEST(processed).should == Array<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	});
}
