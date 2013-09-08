//
//  test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "io/formatted_stream.hpp"
#include "io/stdio_stream.hpp"
#include "io/formatters.hpp"

#include "base/log.hpp"
#include "io/resource_manager.hpp"
#include "base/arch.hpp"

#include <stdio.h>
#include <sys/ioctl.h>

namespace grace {
	namespace {
		const char TerminalBlack[] =   "\033[22;30m";
		const char TerminalRed[] =     "\033[01;31m";
		const char TerminalGreen[] =   "\033[01;32m";
		const char TerminalMagenta[] = "\033[01;35m";
		const char TerminalCyan[] =    "\033[01;36m";
		const char TerminalYellow[] =  "\033[01;33m";
		const char TerminalResetColor[] = "\033[00m";
		
		TerminalWindowInfo query_terminal_window() {
			TerminalWindowInfo info;
			struct ttysize ts;
			if (ioctl(0, TIOCGSIZE, &ts) == 0) {
				info.columns = ts.ts_cols ? ts.ts_cols : 80;
				info.rows = ts.ts_lines ? ts.ts_lines : 80;
				info.supports_color = true; // TODO: Check this?
			} else {
				info.columns = 80;
				info.rows = 25;
				info.supports_color = false;
			}
			return info;
		}
	}
	
	TerminalWindowInfo& terminal() {
		static TerminalWindowInfo info = query_terminal_window();
		return info;
	}
	
	const char* TerminalWindowInfo::reset() {
		if (supports_color) {
			return TerminalResetColor;
		}
		return "";
	}
	const char* TerminalWindowInfo::red() {
		if (supports_color) {
			return TerminalRed;
		}
		return "";
	}
	const char* TerminalWindowInfo::cyan() {
		if (supports_color) {
			return TerminalCyan;
		}
		return "";
	}
	const char* TerminalWindowInfo::green() {
		if (supports_color) {
			return TerminalGreen;
		}
		return "";
	}
	const char* TerminalWindowInfo::yellow() {
		if (supports_color) {
			return TerminalYellow;
		}
		return "";
	}
	const char* TerminalWindowInfo::magenta() {
		if (supports_color) {
			return TerminalMagenta;
		}
		return "";
	}

	namespace {
		void print_suite_begin(const TestSuite& suite) {
			StdOut << terminal().cyan() << "Testing: " << suite.name << terminal().reset() << "\n";
		}
		
		void print_suite_end(const TestSuite& suite) {
			StdOut << "Ran " << suite.statistics.total << " cases, " << suite.statistics.failed << " failed, " << suite.statistics.pending << " pending.\n";
		}
		
		void print_failure(StringRef suite, StringRef example, StringRef details, StringRef file_path, int lineno) {
			StdOut << terminal().red() << " [F] " << terminal().reset()
			       << pad_or_truncate(example, terminal().columns - 17) << '\n';
			size_t last_slash_pos = rfind(file_path, '/');
			StringRef file_name = last_slash_pos == String::NPos ? file_path : substr(file_path, last_slash_pos+1);
			StdOut << "      " << file_name << ":" << lineno << ": " << details << "\n";
		}
		
		void print_success(StringRef suite, StringRef example) {
			StdOut << terminal().green() << " [✓] " << terminal().reset() <<  truncate(example, terminal().columns - 17, "...") << '\n';
		}
		
		void print_pending(StringRef suite, StringRef example) {
			StdOut << terminal().yellow() << " [?] " << terminal().reset() << truncate(example, terminal().columns - 17, "...") << '\n';
		}
		
		void print_pending_benchmark(StringRef suite, StringRef bm) {
			StdOut << terminal().yellow() << " [?] " << terminal().reset() << "benchmark: " << truncate(bm, terminal().columns - 14, "...") << '\n';
		}
		
		void print_benchmark(StringRef suite, StringRef doing_what, const BenchmarkResults& results, bool verbose) {
			if (verbose) {
				StdOut << terminal().green() << " [✓] " << terminal().reset() << " benchmark: " << truncate(doing_what, terminal().columns - 14, "...") << ":\n";
				StdOut << results;
			} else {
				StdOut << terminal().green() << " [✓] " << terminal().reset() << " benchmark: " << truncate(doing_what, terminal().columns - 14, "...") << ": " << format_time_delta(results.best_time) << '\n';
			}
		}
	}
	
	void TestSuite::run(const TestRunnerOptions& opts) {
		current_options_ = &opts;
		bool skip = false;
		opts.run_only.map([&](const Regex& r) {
			if (!r.match(name)) {
				skip = true;
			}
		});
		opts.exclude.map([&](const Regex& r) {
			if (r.match(name)) {
				skip = true;
			}
		});
		
		if (skip)
			return;
		
		print_suite_begin(*this);
		try {
			describe__();
		}
		catch (const IException& ex) {
			if (last_was_quiet_success_) {
				StdOut << '\n';
				last_was_quiet_success_ = false;
			}
			StringStream ss;
			ss << "Unexpected exception outside of examples: " << ex.what();
			auto s = ss.string();
			print_failure(name, "<none>", s, "<unknown>", 0);
		}
		catch (const std::exception& ex) {
			if (last_was_quiet_success_) {
				StdOut << '\n';
				last_was_quiet_success_ = false;
			}
			StringStream ss;
			ss << "Unexpected STL exception outside of examples: " << ex.what();
			auto s = ss.string();
			print_failure(name, "<none>", s, "<unknown>", 0);
		}
		catch (...) {
			if (last_was_quiet_success_) {
				StdOut << '\n';
				last_was_quiet_success_ = false;
			}
			print_failure(name, "<none>", "Unexpected exception outside of examples.", "<unknown>", 0);
		}
		if (last_was_quiet_success_) {
			StdOut << '\n';
			last_was_quiet_success_ = false;
		}
		print_suite_end(*this);
	}
	
	void TestSuite::before_each(Function<void ()> closure) {
		before_example_.push_back(closure);
	}
	
	void TestSuite::after_each(Function<void ()> closure) {
		after_example_.push_back(closure);
	}
	
	void TestSuite::it(StringRef should, Function<void ()> closure) {
		ScratchAllocator scratch;
		StringStream ss(scratch);
		ss << "it should " << should;
		feature(ss.string(scratch), move(closure));
	}
	
	void TestSuite::it(StringRef should) {
		ScratchAllocator scratch;
		StringStream ss(scratch);
		ss << "it should " << should;
		feature(ss.string(scratch));
	}
	
	void TestSuite::feature(StringRef should, Function<void ()> closure) {
		statistics.total++;
		try {
			for (auto x: before_example_) { x(); }
			closure();
			for (auto x: after_example_) { x(); }
			statistics.succeeded++;
			if (!current_options_->quiet) {
				print_success(name, should);
			} else {
				if (!last_was_quiet_success_) {
					StdOut << ' ';
				}
				last_was_quiet_success_ = true;
				StdOut << '.';
			}
		}
		catch (const IException& ex) {
			statistics.failed++;
			if (last_was_quiet_success_) {
				StdOut << '\n';
				last_was_quiet_success_ = false;
			}
			StringStream ss;
			ss << "Exception: " << ex.what();
			String err = ss.string();
			print_failure(name, should, err, "<unknown>", 0);
		}
		catch (const std::exception& ex) {
			statistics.failed++;
			if (last_was_quiet_success_) {
				StdOut << '\n';
				last_was_quiet_success_ = false;
			}
			StringStream ss;
			ss << "STL Exception: " << ex.what();
			String err = ss.string();
			print_failure(name, should, err, "<unknown>", 0);
		}
		catch (TestFailureException ex) {
			statistics.failed++;
			if (last_was_quiet_success_) {
				StdOut << '\n';
				last_was_quiet_success_ = false;
			}
			print_failure(name, should, ex.details, ex.file, ex.lineno);
		}
	}
	
	void TestSuite::feature(StringRef should) {
		statistics.pending++;
		if (last_was_quiet_success_) {
			StdOut << '\n';
			last_was_quiet_success_ = false;
		}
		print_pending(name, should);
	}
	
	void TestSuite::benchmark(StringRef doing_what, Function<void ()> closure, uint32 iterations) {
		if (!current_options_->run_benchmarks) return;
		statistics.benchmarks++;
		try {
			BenchmarkResults r;
			should_not_throw_any_exception([&]() {
				r = grace::benchmark(doing_what, iterations, move(closure));
			});
			if (!current_options_->quiet) {
				print_benchmark(name, doing_what, r, true);
			} else {
				if (last_was_quiet_success_) {
					StdOut << '\n';
				}
				last_was_quiet_success_ = false;
				print_benchmark(name, doing_what, r, false);
			}
		}
		catch (TestFailureException ex) {
			statistics.failed++;
			if (last_was_quiet_success_) {
				StdOut << '\n';
				last_was_quiet_success_ = false;
			}
			print_failure(name, doing_what, ex.details, ex.file, ex.lineno);
		}
	}
	
	void TestSuite::benchmark(StringRef doing_what) {
		if (!current_options_->run_benchmarks) return;
		statistics.pending++;
		if (last_was_quiet_success_) {
			StdOut << '\n';
			last_was_quiet_success_ = false;
		}
		print_pending_benchmark(name, doing_what);
	}
	
	void fail(StringRef details, StringRef file, int lineno) {
		throw TestFailureException(String(details, default_allocator()), file, lineno);
	}

	int test_main(int argc, char** argv, TestSuite& suite) {
		default_allocator().start_allocation_tracking();

		int result = 0;
		{
			LogManager::current_log_level() = LogLevelNone;
			TestRunnerOptions options;
			for (int i = 1; i < argc; ++i) {
				String arg = argv[i];
				if (arg == "--no-color") {
					terminal().supports_color = false;
				} else if (arg == "--show-log") {
					LogManager::current_log_level() = LogLevelDebug;
				} else if (arg == "--only" || arg == "-m") {
					int ni = i + 1;
					if (ni < argc) {
						options.run_only = Regex(argv[ni], "i");
						i = ni; // skip
					} else {
						Warning() << "--only specified without pattern.";
					}
				} else if (arg == "--quiet" || arg == "-q") {
					options.quiet = true;
				} else if (arg == "--verbose" || arg == "-v") {
					LogManager::current_log_level() = LogLevelDebug;
					options.quiet = false;
				} else if (arg == "--benchmark" || arg == "-b") {
					options.run_benchmarks = true;
				}
			}
		
			StdOut.stream().set_sync(true);
			
			try {
				suite.run(options);
			}
			catch (const IException& exception) {
				StdOut << "UNCAUGHT EXCEPTION OUTSIDE OF EXAMPLES: " << exception.what() << '\n';
				result = -1;
			}
			catch (const std::exception& exception) {
				StdOut << "UNCAUGHT EXCEPTION OUTSIDE OF EXAMPLES: " << exception.what() << '\n';
				result = -1;
			}
			catch (...) {
				StdErr << "UNKNOWN EXCEPTION OUTSIDE OF EXAMPLES.\n";
				result = -1;
			}
		}
		
		LinearAllocator tmp_ext_scratch(0x4000000); // 64 MiB
		ScratchAllocator leak_scratch(tmp_ext_scratch);
		auto leaks = default_allocator().finish_allocation_tracking(leak_scratch);
		if (leaks.size()) {
			StdOut << "LEAK REPORT: " << leaks.size() << " unfreed allocations.\n";
			uint32 leak_count = 0;
			uint32 max_display_leaks = 10;
			Regex grace_namespace("(grace::|__1::|ymous namespace)", leak_scratch);
			for (auto& leak: leaks) {
				ScratchAllocator scratch;
				String module(scratch);
				String symbol(scratch);
				uint32 offset;
				++leak_count;
				if (leak_count > max_display_leaks) {
					StdOut << "Too many leaks, omitting " << leaks.size() - max_display_leaks << " from output.\n";
					break;
				}
				
				StdOut << "  " << format("%p", leak.address) << " (" << format_data_size(leak.size) << "):\n";
				for (uint32 i = 0; i < MEMORY_LEAK_BACKTRACE_STEPS; ++i) {
					resolve_symbol(leak.backtrace[i], module, symbol, offset);
					symbol = replace(symbol, grace_namespace, "", scratch);
					StdOut <<  "    [" << module << "] " << symbol << " + " << offset << "\n";
				}
			}
		}
		return result;
	}
}
