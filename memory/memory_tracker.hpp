//
//  memory_tracker.h
//  grace
//
//  Created by Simon Ask Ulsnes on 07/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __grace__memory_tracker__
#define __grace__memory_tracker__

#include "base/basic.hpp"
#include "base/array_ref.hpp"
#include "base/maybe.hpp"

namespace grace {
	class String;
	
	static const size_t MEMORY_LEAK_BACKTRACE_STEPS = 14;
	
	struct MemoryLeak {
		void* address;
		size_t size;
		void* backtrace[MEMORY_LEAK_BACKTRACE_STEPS];
	};
	
	template <typename T> class Array;

	struct MemoryTracker {
	public:
		~MemoryTracker();
		void track_allocation(void* address, size_t size);
		void track_free(void* address);
		void start();
		void pause();
		void unpause();
		void stop();
		
		void get_results(Array<MemoryLeak>& out_results);
		
		Maybe<MemoryLeak> get_allocation(void* ptr);
	private:
		struct Impl;
		Impl* impl = nullptr;
		void ensure_init();
	};
}

#endif /* defined(__grace__memory_tracker__) */
