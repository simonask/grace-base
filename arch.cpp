//
//  arch.cpp
//  falling
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

namespace falling {
	void get_backtrace(void** out_instruction_pointers, size_t num_steps, size_t offset) {
		size_t total_steps = num_steps + offset + 1;
		void* buffer[total_steps];
		backtrace(buffer, (int)total_steps);
		memcpy(out_instruction_pointers, buffer + offset + 1, num_steps * sizeof(void*));
	}
	
	void resolve_symbol(void* ip, String& out_module_name, String& out_demangled_function_name, uint32& out_offset) {
		// XXX: Most probably platform dependent.
		
		char** symbols = backtrace_symbols(&ip, 1);
		const char* mangled_start = symbols[0] + 59;
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
		COPY_STRING_REF_TO_CSTR_BUFFER(mangled_symbol_name_buffer, mangled_symbol_name);
		
		size_t len;
		int status;
		char* buffer = __cxxabiv1::__cxa_demangle(mangled_symbol_name_buffer.data(), nullptr, &len, &status);
		if (status == 0) {
			out_demangled_function_name = buffer;
		} else {
			out_demangled_function_name = mangled_symbol_name;
		}
		::free(buffer);
		::free(symbols);
	}
}
