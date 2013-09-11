//
//  string_stream.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_string_stream_hpp
#define grace_string_stream_hpp

#include "io/formatted_stream.hpp"
#include "io/memory_stream.hpp"
#include "base/string.hpp"

namespace grace {
	class StringStream : public FormattedStream {
	public:
		StringStream(IAllocator& alloc = default_allocator()) : buffer_(alloc), FormattedStream(buffer_) {}
		explicit StringStream(StringRef);
		StringStream(const StringStream& other, IAllocator& alloc = default_allocator()) : buffer_(other.buffer_, alloc), FormattedStream(buffer_) {}
		StringStream(StringStream&& other) : buffer_(std::move(other.buffer_)), FormattedStream(buffer_) {}
		
		String string(IAllocator& alloc) const;
		String string() const;
		void set_string(StringRef);
		size_t size() const { return buffer_.size(); }
		
		// StringStream-like
		String str() const;
		void str(StringRef s);

		MemoryBufferStream& buffer() { return buffer_; }
		const MemoryBufferStream& buffer() const { return buffer_; }
	private:
		MemoryBufferStream buffer_;
	};
}

#endif
