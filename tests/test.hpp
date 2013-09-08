//
//  test.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_test_hpp
#define grace_test_hpp

#include <functional>
#include <regex>
#include <typeinfo>
#include <cxxabi.h>

#include "base/basic.hpp"
#include "io/string_stream.hpp"
#include "io/formatters.hpp"
#include "io/stdio_stream.hpp"
#include "base/array.hpp"
#include "base/regex.hpp"
#include "base/maybe.hpp"
#include "base/benchmark.hpp"

namespace grace {
	struct TestRunnerOptions {
		Maybe<Regex> run_only;
		Maybe<Regex> exclude;
		bool quiet = false;
		bool run_benchmarks = false;
	};
	
	struct TerminalWindowInfo {
		uint32 columns;
		uint32 rows;
		bool supports_color;
		
		const char* reset();
		const char* red();
		const char* cyan();
		const char* green();
		const char* yellow();
		const char* magenta();
	};
	TerminalWindowInfo& terminal();
	
	struct DLL_PUBLIC TestFailureException {
		String details;
		StringRef file;
		int lineno;
		TestFailureException(String details, StringRef file, int lineno) : details(std::move(details)), file(file), lineno(lineno) {}
	};
	
	void fail(StringRef details, StringRef file, int lineno);
	
	template <typename ExceptionType>
	void should_throw_exception(Function<void()> closure, StringRef file = "<unknown>", int lineno = 0) {
		try {
			closure();
		}
		catch (ExceptionType ex) {
			return; // Success!
		}
		catch (...) {
			const std::type_info* ex_type = __cxxabiv1::__cxa_current_exception_type();
			StringStream ss;
			ss << "Expected exception of type " << typeid(ExceptionType).name() << ", got exception of type " << ex_type->name() << '.';
			fail(ss.str(), file, lineno);
		}
		StringStream ss;
		ss << "Expected exception of type " << typeid(ExceptionType).name() << ", got nothing.";
		fail(ss.str(), file, lineno);
	}
	
	inline void should_throw_any_exception(Function<void()> closure, StringRef file = "<unknown>", int lineno = 0) {
		try {
			closure();
		}
		catch (TestFailureException ex) {
			throw ex;
		}
		catch (...) {
			return; // Success!
		}
		StringStream ss;
		ss << "Expected exception, got nothing.";
		fail(ss.str(), file, lineno);
	}
	
	template <typename ExceptionType>
	void should_throw_exception_matching(Regex regex, Function<void()> closure, StringRef file = "<unknown>", int lineno = 0) {
		try {
			closure();
		}
		catch (TestFailureException ex) {
			throw ex;
		}
		catch (ExceptionType ex) {
			StringRef what = ex.what();
			bool match = regex.match(what);
			if (match) {
				return; // Success!
			} else {
				StringStream ss;
				ss << "Expected exception of type " << typeid(ExceptionType).name() << ", matching /" << regex << "/, but got what(): " << what;
				fail(ss.str(), file, lineno);
			}
		}
		StringStream ss;
		ss << "Expected exception of type " << typeid(ExceptionType).name() << ", but got nothing.";
		fail(ss.str(), file, lineno);
	}
	
	template <typename ExceptionType>
	void should_not_throw_exception(Function<void()> closure, StringRef file = "<unknown>", int lineno = 0) {
		try {
			closure();
		}
		catch (TestFailureException ex) {
			throw ex;
		}
		catch (ExceptionType ex) {
			StringStream ss;
			ss << "Expected no exceptions, but got exception of type " << typeid(ExceptionType).name() << ".";
			fail(ss.str(), file, lineno);
		}
		catch (const IException& ex) {
			StringStream ss;
			const std::type_info* ex_type = __cxxabiv1::__cxa_current_exception_type();
			ss << "Expected no exceptions, but got exceptions of type " << ex_type->name() << " with message: " << ex.what();
			fail(ss.str(), file, lineno);
		}
		catch (const std::exception& ex) {
			StringStream ss;
			const std::type_info* ex_type = __cxxabiv1::__cxa_current_exception_type();
			ss << "Expected no exceptions, but got exceptions of type " << ex_type->name() << " with message: " << ex.what();
			fail(ss.str(), file, lineno);
		}
		catch (...) {
			const std::type_info* ex_type = __cxxabiv1::__cxa_current_exception_type();
			StringStream ss;
			ss << "Expected no exceptions, but got unrecognized exception of type " << ex_type->name() << '.';
			fail(ss.str(), file, lineno);
		}
		return; // Success!
	}
	
	inline void should_not_throw_any_exception(Function<void()> closure, StringRef file = "<unknown>", int lineno = 0) {
		try {
			closure();
		}
		catch (TestFailureException ex) {
			throw ex;
		}
		catch (const IException& ex) {
			StringStream ss;
			const std::type_info* ex_type = __cxxabiv1::__cxa_current_exception_type();
			ss << "Expected no exceptions, but got exceptions of type " << ex_type->name() << " with message: " << ex.what();
			fail(ss.str(), file, lineno);
		}
		catch (const std::exception& ex) {
			StringStream ss;
			const std::type_info* ex_type = __cxxabiv1::__cxa_current_exception_type();
			ss << "Expected no exceptions, but got exceptions of type " << ex_type->name() << " with message: " << ex.what();
			fail(ss.str(), file, lineno);
		}
		catch (...) {
			const std::type_info* ex_type = __cxxabiv1::__cxa_current_exception_type();
			StringStream ss;
			ss << "Expected no exceptions, but got unrecognized exception of type " << ex_type->name() << '.';
			fail(ss.str(), file, lineno);
		}
		return; // Success!
	}
	
	// TODO: Consider if these are ever useful?
	template <typename ExceptionType>
	void should_not_throw_exception_matching(std::regex regex, Function<void()> closure, StringRef file = "<unknown>", int lineno = 0);
	void should_not_throw_standard_exception_matching(std::regex regex, Function<void()> closure, StringRef file = "<unknown>", int lineno = 0);
	
	template <typename R>
	void should_return(R expected, Function<R()> closure, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&](){
			r = expected == closure();
		});
		if (!r) {
			StringStream ss;
			ss << "Closure should return " << expected << ", but it didn't.";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename R>
	void should_not_return(R unexpected, Function<R()> closure, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&](){
			r = unexpected == closure();
		});
		if (r) {
			StringStream ss;
			ss << "Closure should not return " << unexpected << ", but it did.";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_be_equal(A a, B b, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&](){
			r = a == b;
		});
		if (!r) {
			StringStream ss;
			ss << "Expected equality, but " << a << " is not equal to " << b << ".";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_not_be_equal(A a, B b, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&]() {
			r = !(a == b);
		});
		if (!r) {
			StringStream ss;
			ss << "Expected inequality, but " << a << " is equal to " << b << ".";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_be_not_equal(A a, B b, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&]() {
			r = a != b;
		});
		if (!r) {
			StringStream ss;
			ss << "Expected inequality, but " << a << " isn't not equal to " << b << " (you heard me).";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_be_less_than(A a, B b, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&]() {
			r = a < b;
		});
		if (!r) {
			StringStream ss;
			ss << "Expected " << a << " to be less than " << b << ", but it isn't.";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_be_not_less_than(A a, B b, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&]() {
			r = !(a < b);
		});
		if (!r) {
			StringStream ss;
			ss << "Expected " << a << " to not be less than " << b << ", but it is.";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_be_less_than_or_equal_to(A a, B b, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&](){
			r = a <= b;
		});
		if (!r) {
			StringStream ss;
			ss << "Expected " << a << " to be less than or equal to " << b << ", but it isn't.";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_be_greater_than(A a, B b, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&](){
			r = a > b;
		});
		if (!r) {
			StringStream ss;
			ss << "Expected " << a << " to be greater than " << b << ", but it isn't.";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_be_greater_than_or_equal_to(A a, B b, StringRef file = "<unknown>", int lineno = 0) {
		bool r;
		should_not_throw_any_exception([&](){
			r = a >= b;
		});
		if (!r) {
			StringStream ss;
			ss << "Expected " << a << " to be greater than or equal to " << b << ", but it isn't.";
			fail(ss.str(), file, lineno);
		}
	}
	
	template <typename A, typename B>
	void should_be_faster_than(A a, StringRef a_name, B b, StringRef b_name, uint32 iterations = 10, StringRef file = "<unknown>", int lineno = 0) {
		StringStream description_ss;
		description_ss << file << ":" << lineno;
		String description = description_ss.string();
		should_not_throw_any_exception([&]() {
			Function<void()> functions[2] = {move(a), move(b)};
			StringRef descriptions[2] = {a_name, b_name};
			auto results = benchmark_compare(description, iterations, ArrayRef<Function<void()>>(functions, functions + 2), ArrayRef<StringRef>(descriptions, descriptions+2));
			auto& a_results = results.results[0];
			auto& b_results = results.results[1];
			if (a_results.best_time >= b_results.best_time) {
				StdOut << a_results << b_results;
				StringStream ss;
				ss << "Expected '" << a_name << "' to be faster than '" << b_name << "' at least once in " << iterations << " iterations, but " << format_time_delta(a_results.best_time) << " isn't faster than " << format_time_delta(b_results.best_time) << ".";
				fail(ss.string(), file, lineno);
			}
		});
	}
	
	template <typename A, typename B>
	void should_be_at_least_as_fast_as(A a, StringRef a_name, B b, StringRef b_name, uint32 iterations = 10, StringRef file = "<unknown>", int lineno = 0) {
		StringStream description_ss;
		description_ss << file << ":" << lineno;
		String description = description_ss.string();
		should_not_throw_any_exception([&]() {
			Function<void()> functions[2] = {move(a), move(b)};
			StringRef descriptions[2] = {a_name, b_name};
			auto results = benchmark_compare(description, iterations, functions, descriptions);
			auto& a_results = results.results[0];
			auto& b_results = results.results[1];
			if (a_results.best_time > b_results.best_time) {
				StdOut << a_results << b_results;
				StringStream ss;
				ss << "Expected '" << a_name << "' to be at least as fast as '" << b_name << "' at least once in " << iterations << " iterations, but " << format_time_delta(a_results.best_time) << " isn't faster than " << format_time_delta(b_results.best_time) << ".";
				fail(ss.string(), file, lineno);
			}
		});
	}
	
	template <typename Value>
	struct TestValue {
		struct Not {
			TestValue<Value>& owner;
			Not(TestValue<Value>& owner) : owner(owner) {}
			
			template <typename B>
			void operator==(B b) { owner.should != b; }
			template <typename B>
			void operator!=(B b) { owner.should == b; }
			/*template <typename B>
			void operator<(B b) { should_be_not_less_than(v, b); }
			template <typename B>
			void operator<=(B b) { should_be_not_less_than_or_equal_to(v, b); }
			template <typename B>
			void operator>(B b) { should_be_not_greater_than(v, b); }
			template <typename B>
			void operator>=(B b) { should_be_not_greater_than_or_equal_to(v, b); }*/
		};
		
		TestValue(Value v, StringRef expr, StringRef file, int lineno) : v(move(v)), expr(std::move(expr)), file(file), lineno(lineno), should(*this), should_be(*this), should_not(*this), should_not_be(should_not) {}
		StringRef expr;
		StringRef file;
		int lineno;
		Value v;
		
		TestValue<Value>& should;
		TestValue<Value>& should_be;
		Not should_not;
		Not& should_not_be;
		template <typename B>
		void operator==(B b) { should_be_equal(v, std::forward<B>(b), file, lineno); }
		template <typename B>
		void operator!=(B b) { should_be_not_equal(v, std::forward<B>(b), file, lineno); }
		template <typename B>
		void operator<(B b) { should_be_less_than(v, std::forward<B>(b), file, lineno); }
		template <typename B>
		void operator<=(B b) { should_be_less_than_or_equal_to(v, std::forward<B>(b), file, lineno); }
		template <typename B>
		void operator>(B b) { should_be_greater_than(v, std::forward<B>(b), file, lineno); }
		template <typename B>
		void operator>=(B b) { should_be_greater_than_or_equal_to(v, std::forward<B>(b), file, lineno); }
		
		template <typename B>
		void faster_than(const B& b, StringRef b_name, uint32 iterations = 20) {
			should_be_faster_than(v, expr, b, b_name, iterations, file, lineno);
		}
		template <typename B>
		void at_least_as_fast_as(const B& b, StringRef b_name, uint32 iterations = 20) {
			should_be_at_least_as_fast_as(v, expr, b, b_name, iterations, file, lineno);
		}
	};
	
#define MACRO_RETARDATION(x, y) x ## y
#define MACRO_CONCAT(x, y) MACRO_RETARDATION(x, y)
#define TEST(VALUE) \
	grace::TestValue<typename grace::RemoveConstRef<decltype(VALUE)>::Type> MACRO_CONCAT(_testAssertionObject_, __LINE__)(VALUE, #VALUE, __FILE__, __LINE__); MACRO_CONCAT(_testAssertionObject_, __LINE__)
#define FASTER_THAN(OTHER) \
	.faster_than(OTHER, #OTHER)
#define AT_LEAST_AS_FAST_AS(OTHER) \
	.at_least_as_fast_as(OTHER, #OTHER)
	
	struct TestSuite {
		void before_each(Function<void()> closure);
		void after_each(Function<void()> closure);
		
		void it(StringRef should, Function<void()> closure);
		void it(StringRef should); // for pending tests
		void feature(StringRef name, Function<void()> closure);
		void feature(StringRef name); // for pending tests
		
		void benchmark(StringRef doing_what, Function<void()> closure, uint32 iterations = 10);
		void benchmark(StringRef doing_what); // for pending benchmarks
		
		void fail(StringRef details, StringRef file, int lineno);
		
		StringRef name;
		void run(const TestRunnerOptions& opts);
		virtual void describe__() = 0;
		
		struct {
			int total = 0;
			int succeeded = 0;
			int failed = 0;
			int pending = 0;
			int benchmarks = 0;
		} statistics;
	protected:
		explicit TestSuite(StringRef name) : name(std::move(name)) {}
	private:
		const TestRunnerOptions* current_options_ = nullptr;
		bool last_was_quiet_success_ = false;
		Array<Function<void()>> before_example_;
		Array<Function<void()>> after_example_;
	};

	int test_main(int argc, char** argv, TestSuite& suite);
}


#define SUITE(NAME) \
	struct TestSuite_##NAME : public grace::TestSuite { \
	public: \
		TestSuite_##NAME(grace::StringRef name) : grace::TestSuite(std::move(name)) {} \
		void describe__() final; \
	}; \
	static TestSuite_##NAME describe_suite_##NAME = TestSuite_##NAME(#NAME); \
	void run_suite_##NAME(const grace::TestRunnerOptions& opts) { describe_suite_##NAME.run(opts); } \
	int main(int argc, char** argv) { TestSuite_##NAME suite(#NAME); return grace::test_main(argc, argv, suite); } \
	void TestSuite_##NAME::describe__()


#endif
