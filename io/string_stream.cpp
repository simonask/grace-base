#include "io/string_stream.hpp"
#include "base/string.hpp"

namespace grace {
	StringStream::StringStream(StringRef s) : FormattedStream(buffer_) {
		set_string(s);
	}
	
	String StringStream::string() const {
		return string(buffer_.allocator());
	}
	
	String StringStream::string(IAllocator& alloc) const {
		size_t len = buffer_.size();
		char* buffer = (char*)alloc.allocate(len, 1);
		char* end = buffer + len;
		buffer_.copy_to(buffer, end);
		return String::take_ownership(alloc, buffer, len);
	}
	
	void StringStream::set_string(StringRef s) {
		buffer_.clear();
		buffer_.insert(s.begin(), s.end());
	}

	String StringStream::str() const {
		return string();
	}

	void StringStream::str(StringRef str) {
		set_string(str);
	}
}