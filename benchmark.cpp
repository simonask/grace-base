//
//  benchmark.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 06/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "base/benchmark.hpp"
#include "base/log.hpp"
#include "io/formatters.hpp"

namespace falling {
	BenchmarkResults benchmark(StringRef description, uint32 iterations, Function<void()> function) {
		BenchmarkResults results;
		results.description = description;
		results.iterations = 0;
		results.worst_time = SystemTime::microseconds(0);
		results.best_time = SystemTimeDelta::forever();
		SystemTimeDelta total_time;
		for (uint32 i = 0; i < iterations; ++i) {
			SystemTime before;
			SystemTime after;
			try {
				before = system_now();
				function();
				after = system_now();
			}
			catch (...) {
				Error() << "Unhandled exception in benchmark, stopping!";
				break;
			}
			SystemTimeDelta elapsed = after - before;
			total_time += elapsed;
			results.add_time(elapsed, i);
		}
		results.avg_time = SystemTime::microseconds(total_time.microseconds() / results.iterations);
		results.med_time = results.best_time + SystemTime::microseconds((results.worst_time - results.best_time).microseconds() / 2);
		return move(results);
	}
	
	BenchmarkCompareResults benchmark_compare(StringRef description, uint32 iterations, ArrayRef<Function<void()>> functions, ArrayRef<StringRef> descriptions) {
		BenchmarkCompareResults results;
		results.results.reserve(functions.size());
		results.description = description;
		
		for (size_t i = 0; i < functions.size(); ++i) {
			StringRef d = i >= descriptions.size() ? "<unknown>" : descriptions[i];
			BenchmarkResults r = benchmark(d, iterations, functions[i]);
			results.results.push_back(move(r));
		}
		
		std::sort(results.results.begin(), results.results.end(), [&](const BenchmarkResults& a, const BenchmarkResults& b) {
			return a.best_time < b.best_time;
		});
		
		return move(results);
	}
	
	BenchmarkCompareResults benchmark_compare_interleaved(StringRef description, uint32 iterations, ArrayRef<Function<void()>> functions, ArrayRef<StringRef> descriptions) {
		BenchmarkCompareResults results;
		results.results.resize(functions.size());
		results.description = description;
		
		for (size_t i = 0; i < functions.size(); ++i) {
			BenchmarkResults& r = results.results[i];
			r.description = i >= descriptions.size() ? "<unknown>" : descriptions[i];
			r.iterations = 0;
			r.worst_time = SystemTime::microseconds(0);
			r.best_time = SystemTimeDelta::forever();
		}
		
		for (uint32 i = 0; i < iterations; ++i) {
			for (size_t j = 0; j < functions.size(); ++j) {
				auto& f = functions[j];
				BenchmarkResults& r = results.results[j];
				SystemTime before;
				SystemTime after;
				try {
					before = system_now();
					f();
					after = system_now();
				}
				catch (...) {
					Error() << "Unhandled exception in benchmark, stopping!";
					goto stop;
				}
				SystemTimeDelta elapsed = after - before;
				// using avg_time as total time for now
				r.avg_time += elapsed;
				r.add_time(elapsed, i);
			}
		}
		
		stop:
		for (auto& r: results.results) {
			r.avg_time = SystemTime::microseconds(r.avg_time.microseconds() / r.iterations);
			r.med_time = r.best_time + SystemTime::microseconds((r.worst_time - r.best_time).microseconds() / 2);
		}
		
		std::sort(results.results.begin(), results.results.end(), [&](const BenchmarkResults& a, const BenchmarkResults& b) {
			return a.best_time < b.best_time;
		});
		
		return move(results);
	}
	
	void BenchmarkResults::add_time(SystemTimeDelta elapsed, uint32 iteration_no) {
		++iterations;
		if (elapsed < best_time) {
			best_time = elapsed;
			best_iteration = iteration_no;
		}
		if (elapsed > worst_time) {
			worst_time = elapsed;
			worst_iteration = iteration_no;
		}
	}
	
	FormattedStream& format_time_delta(FormattedStream& os, SystemTimeDelta delta) {
		uint64 us = delta.microseconds();
		if (us > 1000000) {
			uint64 seconds = us / 1000000;
			uint64 rem = us % 1000000;
			uint64 rem_ms = rem / 1000;
			os << seconds << "." << pad_or_truncate(rem_ms, 3, '0', true) << " s";
		} else if (us > 1000) {
			uint64 milliseconds = us / 1000;
			uint64 rem = us % 1000;
			os << milliseconds << '.' << pad_or_truncate(rem, 3, '0', true) << " ms";
		} else {
			os << us << " μs";
		}
		return os;
	}
	
	void print_benchmark_results(FormattedStream& os, const BenchmarkResults& bm, bool include_heading = true) {
		if (include_heading) {
			os << "=== BENCHMARK: " << bm.description << '\n';
		}
		os << "  " << pad_or_truncate("Iterations: ", 32, ' ', true) << bm.iterations << '\n';
		os << "  " << pad_or_truncate("Best time: ", 32, ' ', true);
		format_time_delta(os, bm.best_time) << '\n';
		os << "  " << pad_or_truncate("Worst time: ", 32, ' ', true);
		format_time_delta(os, bm.worst_time) << '\n';
		os << "  " << pad_or_truncate("Avg. time: ", 32, ' ', true);
		format_time_delta(os, bm.avg_time) << '\n';
	}
	
	FormattedStream& operator<<(FormattedStream& os, const BenchmarkResults& bm) {
		print_benchmark_results(os, bm, true);
		return os;
	}
	
	FormattedStream& operator<<(FormattedStream& os, const BenchmarkCompareResults& bm) {
		os << "=== BENCHMARK/COMPARE: " << bm.description << '\n';
		if (bm.results.size()) {
			os << "BEST: " << bm.results.front().description << '\n';
			print_benchmark_results(os, bm.results.front(), false);
		}
		if (bm.results.size() > 1) {
			os << "WORST: " << bm.results.back().description << '\n';
			print_benchmark_results(os, bm.results.back(), false);
		}
		return os;
	}
	
	namespace {
		static byte benchmark_manager_memory[sizeof(BenchmarkManager)];
	}
	
	BenchmarkManager& BenchmarkManager::get() {
		static BenchmarkManager* p = new(benchmark_manager_memory) BenchmarkManager;
		return *p;
	}
	
	void BenchmarkManager::begin_frame() {
		frame.force_clear();
		current = nullptr;
		frame_alloc.reset(frame_alloc.begin());
	}
	
	void BenchmarkManager::add_scoped_times(const BareLinkList<BenchmarkScopeResults> &times) {
		for (auto it = times.begin(); it != times.end(); ++it) {
			BenchmarkResults& results = accum[it->name];
			results.add_time(it->time, results.iterations);
			add_scoped_times(it->children);
		}
	}
	
	void BenchmarkManager::finish_frame() {
		add_scoped_times(frame);
	}
	
	void BenchmarkManager::clear() {
		begin_frame();
		accum.clear();
	}
	
	void BenchmarkManager::enter_scope(StringRef name) {
		BenchmarkScopeResults* results = new(frame_alloc) BenchmarkScopeResults;
		results->name = name;
		if (current) {
			current->children.link_tail(results);
			results->parent = current;
		} else {
			frame.link_tail(results);
		}
		current = results;
	}
	
	void BenchmarkManager::leave_scope(SystemTimeDelta elapsed) {
		ASSERT(current);
		current->time = elapsed;
		current = current->parent;
	}
}