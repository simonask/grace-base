//
//  memory_stream_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 22/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "io/memory_stream.hpp"

using namespace grace;

SUITE(MemoryStream) {
	it("should represent arbitrary buffer", [](){
		byte buffer[128];
		for (byte i = 0; i < 128; ++i) {
			buffer[i] = i;
		}
		
		MemoryStream stream(buffer, buffer + 128);
		byte u = 0;
		while (stream.is_readable()) {
			byte one;
			size_t n = stream.read(&one, 1);
			TEST(n).should == 1;
			TEST(one).should == u;
			++u;
		}
		TEST(u).should == 128;
	});

	it("should write, then read, the same data", []() {
		byte buffer[128];
		for (byte i = 0; i < 128; ++i) {
			buffer[i] = i;
		}
		
		MemoryBufferStream stream;
		size_t n = stream.write(buffer, 128);
		TEST(n).should == 128;
		auto tw = stream.tell_write();
		TEST(n).should == tw;
		
		byte obuffer[128];
		stream.read(obuffer, 128);
		for (byte i = 0; i < 128; ++i) {
			TEST(obuffer[i]).should == buffer[i];
		}
	});
}
