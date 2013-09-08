//
//  binary_document_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 24/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "serialization/binary.hpp"

using namespace grace;

SUITE(BinarySerializer) {
	it("should marshal strings", []() {
		Document document;
		document.root() << "Hello World!";
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		TEST(stream.size()).should > 12;
	});
	
	it("should marshal integers", []() {
		Document document;
		document.root() << (int32)123;
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		TEST(stream.size()).should > 4;
	});
	
	it("should marshal floats", []() {
		Document document;
		document.root() << 123.0;
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		TEST(stream.size()).should > 8;
	});
	
	it("should marshal arrays of strings", []() {
		Document document;
		for (size_t i = 0; i < 100; ++i) {
			StringStream ss;
			ss << "foo" << format("%02d", (int32)i);
			auto& node = document.root().array_push();
			node << ss.string();
		}
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		TEST(stream.size()).should > 500;
	});
	
	it("should marshal arrays of integers", []() {
		Document document;
		for (size_t i = 0; i < 100; ++i) {
			auto& node = document.root().array_push();
			node << (int32)i;
		}
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		TEST(stream.size()).should > 400;
	});
	
	it("should marshal arrays of floats", []() {
		Document document;
		for (size_t i = 0; i < 100; ++i) {
			auto& node = document.root().array_push();
			node << (float32)i;
		}
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		TEST(stream.size()).should > 400;
	});
	
	it("should marshal arrays of arrays", []() {
		Document document;
		for (size_t i = 0; i < 10; ++i) {
			auto& outer = document.root().array_push();
			for (size_t j = 0; j < 10; ++j) {
				auto& inner = outer.array_push();
				inner << (int32)i;
			}
		}
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		TEST(stream.size()).should > 400;
	});
	
	it("should marshal dictionaries", []() {
		Document document;
		for (size_t i = 0; i < 10; ++i) {
			StringStream outer_key_ss;
			outer_key_ss << "key" << i;
			String outer_key = outer_key_ss.string();
			auto& outer_value = document.root()[outer_key];
			for (size_t j = 0; j < 23; ++j) {
				StringStream inner_key_ss;
				inner_key_ss << "key" << j;
				String inner_key = inner_key_ss.string();
				auto& n = outer_value[inner_key];
				switch (j%4) {
					case 0: n << 123; break;
					case 1: n << 123.0; break;
					case 2: n << "Hello"; break;
					case 3: auto& array_val = n.array_push(); array_val << 0; break;
				}
			}
		}
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		TEST(stream.size()).should > 230*4;
	});
	
	it("should correctly report stream size as uint32", []() {
		Document document;
		for (size_t i = 0; i < 10; ++i) {
			StringStream outer_key_ss;
			outer_key_ss << "key" << i;
			String outer_key = outer_key_ss.string();
			auto& outer_value = document.root()[outer_key];
			for (size_t j = 0; j < 23; ++j) {
				StringStream inner_key_ss;
				inner_key_ss << "key" << j;
				String inner_key = inner_key_ss.string();
				auto& n = outer_value[inner_key];
				switch (j%4) {
					case 0: n << 123; break;
					case 1: n << 123.0; break;
					case 2: n << "Hello"; break;
					case 3: auto& array_val = n.array_push(); array_val << 0; break;
				}
			}
		}
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		
		uint32 stream_size;
		stream.read((byte*)&stream_size, sizeof(stream_size));
		TEST(stream.size()).should == stream_size + sizeof(stream_size);
	});
	
	it("should serialize and then deserialize equivalent structure", []() {
		Document document;
		for (size_t i = 0; i < 10; ++i) {
			StringStream outer_key_ss;
			outer_key_ss << "key" << i;
			String outer_key = outer_key_ss.string();
			auto& outer_value = document.root()[outer_key];
			for (size_t j = 0; j < 23; ++j) {
				StringStream inner_key_ss;
				inner_key_ss << "key" << j;
				String inner_key = inner_key_ss.string();
				auto& n = outer_value[inner_key];
				switch (j%4) {
					case 0: n << 123; break;
					case 1: n << 123.0; break;
					case 2: n << "Hello"; break;
					case 3: auto& array_val = n.array_push(); array_val << 0; break;
				}
			}
		}
		MemoryBufferStream stream;
		BinarySerializer().write(stream, document);
		
		Document document2;
		String error;
		bool result = BinarySerializer().read(document2, stream, error);
		TEST(result).should_be == true;
		
		MemoryBufferStream stream2;
		BinarySerializer().write(stream2, document2);
		TEST(stream2.size()).should == stream.size();
	});
}
