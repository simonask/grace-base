//
//  string_stream.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_string_stream_hpp
#define falling_string_stream_hpp

#include "io/formatted_stream.hpp"
#include "io/memory_stream.hpp"
#include "base/string.hpp"

namespace falling {
	class StringStream : public FormattedStream {
	public:
		StringStream(IAllocator& alloc = default_allocator()) : buffer_(alloc), FormattedStream(buffer_) {}
		explicit StringStream(StringRef);
		StringStream(const StringStream& other, IAllocator& alloc = default_allocator()) : buffer_(other.buffer_, alloc), FormattedStream(buffer_) {}
		StringStream(StringStream&& other) : buffer_(std::move(other.buffer_)), FormattedStream(buffer_) {}
		
		String string(IAllocator& alloc) const;
		String string() const;
		void set_string(StringRef);
		
		// StringStream-like
		String str() const { return string(); }
		void str(StringRef s) { set_string(s); }
	private:
		MemoryBufferStream buffer_;
	};
	
	inline StringStream::StringStream(StringRef s) : FormattedStream(buffer_) {
		set_string(s);
	}
	
	inline String StringStream::string() const {
		return string(buffer_.allocator());
	}
	
	inline String StringStream::string(IAllocator& alloc) const {
		size_t len = buffer_.size();
		char* buffer = (char*)alloc.allocate(len, 1);
		char* end = buffer + len;
		buffer_.copy_to(buffer, end);
		return String::take_ownership(alloc, buffer, len);
	}
	
	inline void StringStream::set_string(StringRef s) {
		buffer_.clear();
		buffer_.insert(s.begin(), s.end());
	}
}

#endif
