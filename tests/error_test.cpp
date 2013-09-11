#include "base/error.hpp"
#include "tests/test.hpp"
#include "base/raise.hpp"

namespace {
	using namespace grace;
	struct TestError : ErrorBase<TestError> {};
}

SUITE(Error) {
	feature("catching_errors", []() {
		auto m = catching_errors([&]() {
			raise<TestError>("TestError description");
		});
		TEST(m.is_error()).should == true;
		m.when_error([&](const IError& err) {
			TEST(err.description()) == "TestError description";
		});

		E<int> n = catching_errors([&]() {
			return 123;
		});
		TEST(n.is_error()).should == false;
	});

	it("should capture backtraces on construction", []() {
		auto m = catching_errors([]() {
			throw TestError();
		});
		TEST(m.is_error()).should == true;
		m.when_error([&](const IError& error) {
			TEST(error.backtrace().size()).should >= 2;
		});
	});

	it("should bind error-free arguments to a function", []() {
		auto add3 = [](int a, int b, int c) { return a + b + c; };
		E<int> a(1);
		E<int> b(2);
		E<int> c(3);
		auto result = bind(add3, a, move(b), move(c));
		TEST(result.is_error()).should == false;
	});

	it("should return the first argument with an error when trying to bind to a function", []() {
		auto throw_error = [](bool t) { if (t) throw TestError(); return 123; };
		auto add3 = [](int a, int b, int c) { return a + b + c; };
		E<int> a = 1;
		E<int> b = catching_errors([&]() { return throw_error(true); });
		E<int> c = catching_errors([&]() { return throw_error(false); });
		auto result = bind(add3, a, b, c);
		TEST(result.is_error()).should == true;
	});

	feature("raise error description interpolation", []() {
		try {
			raise<TestError>("{0}, {1}!", "Hello", "World");
		}
		catch (const IError& error) {
			TEST(error.description()).should == "Hello, World!";
		}
	});
}