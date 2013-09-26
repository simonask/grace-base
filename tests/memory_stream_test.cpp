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
		bool eof = false;
		while (!eof) {
			byte one;
			auto m = stream.read(&one, 1);
			m.template when<size_t>([&](size_t n) {
				TEST(n).should == 1;
				TEST(one).should == u;
				++u;
			}).template when<IOEvent>([&](IOEvent ev) {
				eof = true;
			});
			
		}
		TEST(u).should == 128;
	});

	it("should write, then read, the same data", []() {
		byte buffer[128];
		for (byte i = 0; i < 128; ++i) {
			buffer[i] = i;
		}
		
		MemoryBufferStream stream;
		auto m = stream.write(buffer, 128);
		m.template when<size_t>([&](size_t n) {
			TEST(n).should == 128;
			auto tw = stream.tell_write();
			TEST(n).should == tw;	
		});
		
		byte obuffer[128];
		stream.read(obuffer, 128);
		for (byte i = 0; i < 128; ++i) {
			TEST(obuffer[i]).should == buffer[i];
		}
	});
}
