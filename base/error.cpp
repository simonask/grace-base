#include "base/error.hpp"
#include "base/arch.hpp"
#include "io/formatted_stream.hpp"
#include "base/regex.hpp"
#include "base/string.hpp"

namespace grace {
	static const size_t IGNORE_LEVELS_IN_STACK_TRACE = 6;
	static const size_t MAX_STACK_TRACE_LENGTH = 512 - IGNORE_LEVELS_IN_STACK_TRACE;

	ErrorBaseImpl::ErrorBaseImpl() : allocator_(default_allocator()) {
		build_stack_trace();
	}

	ErrorBaseImpl::ErrorBaseImpl(const ErrorBaseImpl& other) : allocator_(default_allocator()) {
		backtrace_ = (void**)allocator_.allocate(sizeof(void*) * other.backtrace_length_, alignof(void*));
		backtrace_length_ = other.backtrace_length_;
		memcpy(backtrace_, other.backtrace_, sizeof(void*) * other.backtrace_length_);
	}

	ErrorBaseImpl::~ErrorBaseImpl() {
		allocator_.free(backtrace_, backtrace_length_ * sizeof(void*));
		allocator_.free(description_data_, description_length_ * sizeof(char));
	}

	StringRef ErrorBaseImpl::type_name() const {
		return StringRef(_internal_type_name());
	}

	StringRef ErrorBaseImpl::description() const {
		return StringRef(description_data_, description_length_);
	}

	void ErrorBaseImpl::build_stack_trace() {
		void* trace[MAX_STACK_TRACE_LENGTH];
		backtrace_length_ = get_backtrace(trace, MAX_STACK_TRACE_LENGTH, IGNORE_LEVELS_IN_STACK_TRACE);
		backtrace_ = (void**)allocator_.allocate(sizeof(void*) * backtrace_length_, alignof(void*));
		memcpy(backtrace_, trace, sizeof(void*) * backtrace_length_);
	}

	ArrayRef<void*> ErrorBaseImpl::backtrace() const {
		return ArrayRef<void*>(backtrace_, backtrace_ + backtrace_length_);
	}

	void ErrorBaseImpl::_take_description(char* description_data, size_t description_length, IAllocator& check_alloc) {
		if (description_data_) {
			allocator_.free(description_data_, description_length_);
			description_data_   = nullptr;
			description_length_ = 0;
		}
		description_length_ = description_length;
		if (&check_alloc == &allocator_) {
			description_data_ = description_data;
		} else {
			description_data_ = (char*)allocator_.allocate(description_length, alignof(char));
			std::copy(description_data, description_data + description_length, description_data_);
		}
	}

	FormattedStream& operator<<(FormattedStream& os, const IError& error) {
		ScratchAllocator scratch;

		String demangled_type_name = demangle_symbol(error.type_name(), default_allocator());

		os << demangled_type_name << ": " << error.description() << "\n";
		os << "Backtrace:\n";
		String mod;
		String fname;
		uint32 offset;
		Regex grace_namespace("(grace::|__1::|ymous namespace)");
		for (void* ip: error.backtrace()) {
			resolve_symbol(ip, mod, fname, offset);
			fname = replace(fname, grace_namespace, "");
			os <<  "    [" << mod << "] " << fname << " + " << offset << "\n";
		}
		return os;
	}
}