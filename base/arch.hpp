//
//  arch.h
//  grace
//
//  Created by Simon Ask Ulsnes on 07/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __grace__arch__
#define __grace__arch__

#include "base/basic.hpp"

namespace grace {
	class String;
	class StringRef;
	class IAllocator;
	template <typename> class Array;
	template <typename> struct ArrayRef;

	String demangle_symbol(StringRef mangled, IAllocator&);
	size_t get_backtrace(void** out_instruction_pointers, size_t max_num_steps, size_t offset = 0);

	Array<String> backtrace_to_strings(ArrayRef<void*> backtrace);

	void resolve_symbol(void* ip, String& out_module_name, String& out_demangled_function_name, uint32& out_offset);
}

#endif /* defined(__grace__arch__) */
