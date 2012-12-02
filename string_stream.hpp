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
#include <string>

namespace falling {
	class StringStream : public FormattedStream {
	public:
		StringStream(IAllocator& alloc = default_allocator()) : buffer_(alloc), FormattedStream(buffer_) {}
		explicit StringStream(const std::string&);
		StringStream(const StringStream& other, IAllocator& alloc = default_allocator()) : buffer_(other.buffer_, alloc), FormattedStream(buffer_) {}
		StringStream(StringStream&& other) : buffer_(std::move(other.buffer_)), FormattedStream(buffer_) {}
		
		std::string string() const;
		void set_string(const std::string&);
		
		// StringStream-like
		std::string str() const { return string(); }
		void str(const std::string& s) { set_string(s); }
	private:
		MemoryBufferStream buffer_;
	};
	
	inline StringStream::StringStream(const std::string& s) : FormattedStream(buffer_) {
		set_string(s);
	}
	
	inline std::string StringStream::string() const {
		std::string result;
		result.resize(buffer_.size());
		buffer_.copy_to(result.begin(), result.end());
		return result;
	}
	
	inline void StringStream::set_string(const std::string& s) {
		buffer_.clear();
		buffer_.insert(s.begin(), s.end());
	}
}

#endif
