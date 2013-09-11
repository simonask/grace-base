#include "tests/test.hpp"
#include "io/formatted_stream.hpp"
#include "io/string_stream.hpp"
#include "io/printf.hpp"

using namespace grace;

SUITE(Formatting) {
	feature("printf with no interpolation", []() {
		StringStream ss;
		ss.printf("Hello, World!");
		TEST(ss.string()).should == "Hello, World!";
	});

	feature("printf with one number", []() {
		StringStream ss;
		ss.printf("Hello {0}!", 123);
		TEST(ss.string()).should == "Hello 123!";
	});

	feature("printf with random access", []() {
		StringStream ss;
		ss.printf("{0}, {2}, {1}", "Word", 123, 456);
		TEST(ss.string()).should == "Word, 456, 123";
	});

	feature("printf with old-style formatting", []() {
		StringStream ss;
		ss.printf("Hey {0}!", format("%.2f", 12.345f));
		TEST(ss.string()).should == "Hey 12.35!";
	});
}