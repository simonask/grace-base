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
#include <sys/mman.h>
#include <execinfo.h>
#include <cxxabi.h>

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
	static const size_t BUCKET_MASK_BITS = LEAKS_PER_BUCKET - 1;
	
	static inline MemoryLeak* get_bucket_for_address(MemoryLeak* begin, MemoryLeak* end, void* addr) {
		uintptr_t p = (uintptr_t)addr;
		uintptr_t idx = p & BUCKET_MASK_BITS;
		return begin + idx;
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
			byte* end = (byte*)impl->begin + TRACKING_ARENA_SIZE;
			impl->end = (MemoryLeak*)end;
			impl->is_tracking = false;
		}
	}
	
	void MemoryTracker::track_allocation(void *address, void *rip) {
		if (impl && impl->is_tracking) {
			impl->num_live_allocations++;
			MemoryLeak* bucket = get_bucket_for_address(impl->begin, impl->end, address);
			for (size_t i = 0; i < LEAKS_PER_BUCKET; ++i) {
				if (bucket[i].address == nullptr) {
					bucket[i].address = address;
					bucket[i].allocation_rip = rip;
					return;
				} else if (bucket[i].address == address) {
					ASSERT(false); // Double allocation of the same pointer?!
				}
			}
			ASSERT(false); // Bucket is full! Something is wrong.
		}
	}
	
	void MemoryTracker::track_free(void *address) {
		if (impl && impl->is_tracking) {
			impl->num_live_allocations--;
			MemoryLeak* bucket = get_bucket_for_address(impl->begin, impl->end, address);
			for (size_t i = 0; i < LEAKS_PER_BUCKET; ++i) {
				if (bucket[i].address == address) {
					bucket[i].address = nullptr;
					bucket[i].allocation_rip = nullptr;
					return;
				}
			}
			//ASSERT(false); // Double free!
		}
	}
	
	void MemoryTracker::start() {
		ensure_init();
		impl->is_tracking = true;
		if (impl->is_paused) {
			impl->is_paused = false;
		} else {
			::memset(impl->begin, 0, (impl->end - impl->begin) * sizeof(MemoryLeak));
		}
	}
	
	void MemoryTracker::pause() {
		if (impl == nullptr) return;
		impl->is_tracking = false;
		impl->is_paused = true;
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
	
	bool MemoryLeak::resolve_symbol(String &out_name) const {
		char** symbols = backtrace_symbols(&allocation_rip, 1);
		const char* mangled_start = symbols[0] + 59;
		StringRef symbol_line = symbols[0];
		StringRef binary_location = substr(symbol_line, 0, 59);
		size_t offset_position = rfind(symbol_line, '+') - 1;
		StringRef offset_string = substr(symbol_line, offset_position);
		size_t mangled_symbol_name_length = symbol_line.size() - 59 - offset_string.size();
		StringRef mangled_symbol_name = substr(symbol_line, 59, mangled_symbol_name_length);
		COPY_STRING_REF_TO_CSTR_BUFFER(mangled_symbol_name_buffer, mangled_symbol_name);
		
		size_t len;
		int status;
		char* buffer = __cxxabiv1::__cxa_demangle(mangled_symbol_name_buffer.data(), nullptr, &len, &status);
		if (status == 0) {
			out_name = concatenate(concatenate(binary_location, buffer, out_name.allocator()), offset_string, out_name.allocator());
		} else {
			out_name = symbols[0];
		}
		::free(buffer);
		::free(symbols);
		return true;
	}
}
