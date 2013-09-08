//
//  time_test.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 04/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_time_test_hpp
#define grace_time_test_hpp

#include "tests/test.hpp"
#include "base/time.hpp"

using namespace grace;

// March 3rd 2012, 16:30:01
static const uint64 TEST_DATE = 1330788601ULL * 1000000ULL;

SUITE(Time) {
	it("should format system time correctly", []() {
		// TODO: Take time zone into account.
		SystemTime time(TEST_DATE*1000);
		StringStream ss;
		ss << time;
		String s = ss.str();
		TEST(s).should == "15:30:01";
	});
}

#endif
