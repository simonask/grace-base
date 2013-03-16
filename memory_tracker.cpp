//
//  memory_tracker.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 07/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "memory/memory_tracker.hpp"
#include "base/array.hpp"
#include "base/string.hpp"
#include "io/string_stream.hpp"
#include "base/stack_array.hpp"
#include "base/arch.hpp"
#include <sys/mman.h>

namespace falling {
	struct MemoryTracker::Impl {
		MemoryLeak* begin;
		MemoryLeak* end;
		size_t num_live_allocations = 0;
		bool is_tracking = false;
		bool is_paused = false;
	};
	
	static const size_t TRACKING_ARENA_SIZE = 1 << 25; // 32 MiB
	static const size_t LEAKS_PER_ARENA = TRACKING_ARENA_SIZE / sizeof(MemoryLeak);
	static const size_t NUM_BUCKETS = 128; // 256 KiB/bucket == L2 cache size
	static const size_t BUCKET_SIZE = TRACKING_ARENA_SIZE / NUM_BUCKETS;
	static const size_t LEAKS_PER_BUCKET = BUCKET_SIZE / sizeof(MemoryLeak);
	static const size_t BUCKET_MASK_BITS = (NUM_BUCKETS - 1);
	
	static inline MemoryLeak* get_bucket_for_address(MemoryLeak* begin, MemoryLeak* end, void* addr) {
		uintptr_t p = (uintptr_t)addr;
		uintptr_t idx = (p & (BUCKET_MASK_BITS << 4)) >> 4;
		return begin + LEAKS_PER_BUCKET * idx;
	}
	
	MemoryTracker::~MemoryTracker() {
		if (impl) {
			::munmap(impl->begin, (impl->end - impl->begin) * sizeof(MemoryLeak));
			::free(impl);
		}
	}
	
	
	void MemoryTracker::ensure_init() {
		if (impl == nullptr) {
			impl = (Impl*)malloc(sizeof(MemoryTracker::Impl));
			new(impl) Impl;
			impl->begin = (MemoryLeak*)::mmap(nullptr, TRACKING_ARENA_SIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
			memset(impl->begin, 0, TRACKING_ARENA_SIZE);
			byte* end = (byte*)impl->begin + TRACKING_ARENA_SIZE;
			impl->end = (MemoryLeak*)end;
			impl->is_tracking = false;
			impl->is_paused = false;
		}
	}
	
	void MemoryTracker::track_allocation(void *address, size_t size) {
		if (impl && impl->is_tracking && !impl->is_paused) {
			MemoryLeak* bucket = get_bucket_for_address(impl->begin, impl->end, address);
			for (size_t i = 0; i < LEAKS_PER_BUCKET; ++i) {
				if (bucket[i].address == nullptr) {
					impl->num_live_allocations++;
					bucket[i].address = address;
					bucket[i].size = size;
					get_backtrace(bucket[i].backtrace, MEMORY_LEAK_BACKTRACE_STEPS, 2);
					return;
				} else if (bucket[i].address == address) {
					bucket[i].address = address;
					bucket[i].size = size;
					get_backtrace(bucket[i].backtrace, MEMORY_LEAK_BACKTRACE_STEPS, 2);
					return;
				}
			}
			ASSERT(false); // Bucket is full! Something is wrong.
		}
	}
	
	void MemoryTracker::track_free(void *address) {
		if (address == nullptr) return;
		if (impl && impl->is_tracking && !impl->is_paused) {
			MemoryLeak* bucket = get_bucket_for_address(impl->begin, impl->end, address);
			for (size_t i = 0; i < LEAKS_PER_BUCKET; ++i) {
				if (bucket[i].address == address) {
					ASSERT(impl->num_live_allocations != 0);
					impl->num_live_allocations--;
					memset(bucket + i, 0, sizeof(MemoryLeak));
					return;
				}
			}
		}
	}
	
	void MemoryTracker::start() {
		ensure_init();
		impl->is_tracking = true;
		if (impl->is_paused) {
			impl->is_paused = false;
		} else {
			::memset(impl->begin, 0, TRACKING_ARENA_SIZE);
		}
	}
	
	void MemoryTracker::pause() {
		if (impl == nullptr) return;
		impl->is_paused = true;
	}
	
	void MemoryTracker::unpause() {
		if (impl == nullptr) return;
		impl->is_paused = false;
	}
	
	void MemoryTracker::stop() {
		if (impl != nullptr) {
			impl->is_tracking = false;
			impl->is_paused = false;
		}
	}
	
	void MemoryTracker::get_results(Array<MemoryLeak>& out_results) {
		if (impl == nullptr) return;
		out_results.reserve(impl->num_live_allocations);
		for (MemoryLeak* p = impl->begin; p != impl->end; ++p) {
			if (p->address != nullptr) {
				out_results.push_back(*p);
			}
		}
	}
}
