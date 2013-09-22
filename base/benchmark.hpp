//
//  benchmark.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 06/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __grace__benchmark__
#define __grace__benchmark__

#include "base/time.hpp"
#include "base/string.hpp"
#include "base/function.hpp"
#include "base/dictionary.hpp"
#include "base/bare_link_list.hpp"
#include "base/array.hpp"

namespace grace {
	struct BenchmarkResults {
		StringRef description;
		uint32 iterations;
		uint32 best_iteration;
		uint32 worst_iteration;
		ProcessTimeDelta best_time;
		ProcessTimeDelta worst_time;
		ProcessTimeDelta avg_time;
		ProcessTimeDelta med_time; // best + (worst-best) / 2
		
		void add_time(ProcessTimeDelta, uint32 i);
	};
	
	class FormattedStream;
	FormattedStream& operator<<(FormattedStream&, const BenchmarkResults&);
	
	BenchmarkResults benchmark(StringRef description, uint32 iterations, Function<void()> function);
	
	struct BenchmarkCompareResults {
		StringRef description;
		Array<BenchmarkResults> results; // sorted by best first
	};
	
	FormattedStream& operator<<(FormattedStream&, const BenchmarkCompareResults&);
	
	BenchmarkCompareResults benchmark_compare(StringRef description, uint32 iterations, ArrayRef<Function<void()>> functions, ArrayRef<StringRef> descriptions = ArrayRef<StringRef>());
	BenchmarkCompareResults benchmark_compare_interleaved(StringRef description, uint32 iterations, ArrayRef<Function<void()>> functions, ArrayRef<StringRef> descriptions = ArrayRef<StringRef>());
	
	
	struct BenchmarkScopeResults : ListLinkBase<BenchmarkScopeResults> {
		StringRef name;
		ProcessTimeDelta time;
		BenchmarkScopeResults* parent;
		BareLinkList<BenchmarkScopeResults> children;
		
		bool is_toplevel() const { return parent == nullptr; }
	};
	
	struct BenchmarkScope;
	
	struct BenchmarkManager {
		static BenchmarkManager& get();
		
		const BareLinkList<BenchmarkScopeResults>& frame_results() { return frame; }
		void begin_frame(); // Clears previous frame.
		void finish_frame(); // Registers accumulated results.
		const Dictionary<BenchmarkResults>& accumulated_results() { return accum; }
		void clear(); // Clears accumulated results!
		void enter_scope(StringRef name);
		void leave_scope(ProcessTimeDelta elapsed);
	private:
		BenchmarkManager() : frame_alloc(sizeof(BenchmarkScopeResults) * 256) {}
		BenchmarkScopeResults* current = nullptr;
		BareLinkList<BenchmarkScopeResults> frame;
		Dictionary<BenchmarkResults> accum;
		LinearAllocator frame_alloc;
		
		void add_scoped_times(const BareLinkList<BenchmarkScopeResults>& times);
	};
	
	struct BenchmarkScope {
		StringRef name;
		bool is_toplevel;
		BenchmarkScope(StringRef name, bool is_toplevel = false) : name(name), is_toplevel(is_toplevel) {
			start_time_ = process_now();
			BenchmarkManager::get().enter_scope(name);
		}
		~BenchmarkScope() {
			ProcessTimeDelta elapsed = process_now() - start_time_;
			BenchmarkManager::get().leave_scope(elapsed);
		}
	private:
		ProcessTime start_time_;
	};
	
#define BENCHMARK_SCOPE(NAME) BenchmarkScope _benchmark_scope_## NAME ## _ ## __LINE__(#NAME, true)
#define BENCHMARK_SUBSCOPE(NAME) BenchmarkScope _benchmark_scope_## NAME ## _ ## __LINE__(#NAME, false)
}

#endif /* defined(__grace__benchmark__) */
