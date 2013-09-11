//
//  arch.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 07/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "base/arch.hpp"
#include "base/string.hpp"
#include "base/stack_array.hpp"
#include "base/parse.hpp"
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

namespace grace {
	size_t get_backtrace(void** out_instruction_pointers, size_t num_steps, size_t offset) {
		const void* main_sym = ::dlsym(nullptr, "main");

		offset += 1;
		size_t total_steps = num_steps + offset;
		void* buffer[total_steps];
		int r = backtrace(buffer, (int)total_steps);
		void** begin = buffer + offset;
		void** end = buffer + r;
		if (end <= begin) {
			begin = end-1;
		}
		for (void** p = begin; p != end; ++p) {
			if (*p == main_sym) {
				end = p;
				break;
			}
		}
		std::copy(begin, end, out_instruction_pointers);
		return (size_t)r;
	}

	String demangle_symbol(StringRef mangled, IAllocator& alloc) {
		size_t len;
		int status;
		COPY_STRING_REF_TO_CSTR_BUFFER(mangled_buffer, mangled);
		char* buffer = __cxxabiv1::__cxa_demangle(mangled_buffer.data(), nullptr, &len, &status);
		String result(alloc);
		if (status == 0) {
			result = buffer;
		} else {
			result = mangled;
		}
		::free(buffer);
		return std::move(result);
	}
	
	void resolve_symbol(void* ip, String& out_module_name, String& out_demangled_function_name, uint32& out_offset) {
		// XXX: Most probably platform dependent.
		
		char** symbols = backtrace_symbols(&ip, 1);
		StringRef symbol_line = symbols[0];
		StringRef binary_location = substr(symbol_line, 4, 36);
		out_module_name = strip(binary_location);
		size_t offset_position = rfind(symbol_line, '+') - 1;
		size_t offset_num_position = offset_position + 3;
		StringRef offset_string = substr(symbol_line, offset_position);
		StringRef offset_number_string = substr(symbol_line, offset_num_position);
		out_offset = parse<uint32>(offset_number_string).get_or(0);
		
		size_t mangled_symbol_name_length = symbol_line.size() - 59 - offset_string.size();
		StringRef mangled_symbol_name = substr(symbol_line, 59, mangled_symbol_name_length);

		out_demangled_function_name = demangle_symbol(mangled_symbol_name, out_module_name.allocator());

		::free(symbols);
	}
}
