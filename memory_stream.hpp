//
//  memorystream.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 21/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_memorystream_hpp
#define falling_memorystream_hpp

#include "io/input_stream.hpp"
#include "io/output_stream.hpp"

#include <deque>

namespace falling {
	class MemoryStream : public InputStream {
	public:
		MemoryStream(const byte* begin, const byte* end) : begin_(begin), end_(end), current_(begin) {}
		
		// InputStream API
		bool is_readable() const override { return current_ < end_; }
		size_t read(byte* buffer, size_t max) override;
		size_t tell_read() const { return current_ - begin_; }
		bool seek_read(size_t position) {
			if (position > size()) {
				return false;
			}
			current_ = begin_ + position;
			return true;
		}
		
		// MemoryStream API
		size_t size() const { return end_ - begin_; }
		size_t data_available() const { return current_ - end_; }
	private:
		const byte* begin_;
		const byte* end_;
		const byte* current_;
	};
	
	
	class MemoryBufferStream : public InputStream, public OutputStream {
	public:
		MemoryBufferStream() {
			read_pos_ = write_pos_ = buffer_.begin();
		}
		template <typename T>
		explicit MemoryBufferStream(T container) {
			read_pos_ = write_pos_ = buffer_.begin();
			insert(std::begin(container), std::end(container));
		}
		template <typename InputIterator>
		MemoryBufferStream(InputIterator begin, InputIterator end) {
			read_pos_ = write_pos_ = buffer_.begin();
			insert(begin, end);
		}

		MemoryBufferStream(MemoryBufferStream&& other) = default;
		MemoryBufferStream& operator=(MemoryBufferStream&& other) = default;
		
		// InputStream API
		bool is_readable() const override;
		size_t read(byte* buffer, size_t max) override;
		size_t tell_read() const override;
		bool seek_read(size_t pos) override;
		
		// OutputStream API
		bool is_writable() const override;
		size_t write(const byte* buffer, size_t max);
		size_t tell_write() const override;
		bool seek_write(size_t pos) override;
		
		// MemoryBufferStream API
		size_t size() const { return buffer_.size(); }
		size_t data_available() const { return buffer_.end() - read_pos_; }
		
		template <typename InputIterator>
		void insert(InputIterator begin, InputIterator end) {
			size_t rp = read_pos_ - buffer_.begin();
			size_t wp = write_pos_ - buffer_.begin();
			buffer_.insert(begin, end);
			read_pos_ = buffer_.begin() + rp;
			write_pos_ = buffer_.begin() + wp;
		}
	private:
		typedef typename std::deque<byte>::iterator Position;
		std::deque<byte> buffer_; // TODO: Maybe use ArrayList<T>?
		Position read_pos_;
		Position write_pos_;
	};
	
	inline bool MemoryBufferStream::is_readable() const {
		return read_pos_ != buffer_.end();
	}
	
	inline size_t MemoryBufferStream::read(byte* buffer, size_t max) {
		size_t n = 0;
		for (auto it = read_pos_; it != buffer_.end() && n < max; ++it, ++n) {
			buffer[n] = *it;
		}
		read_pos_ += n;
		return n;
	}
	
	inline size_t MemoryBufferStream::tell_read() const {
		return read_pos_ - buffer_.begin();
	}
	
	inline bool MemoryBufferStream::seek_read(size_t pos) {
		if (pos >= size()) {
			return false;
		}
		read_pos_ = buffer_.begin() + pos;
		return true;
	}
	
	inline bool MemoryBufferStream::is_writable() const {
		return true;
	}
	
	inline size_t MemoryBufferStream::write(const byte* buffer, size_t n) {
		size_t bytes_until_end = buffer_.end() - write_pos_;
		ssize_t enlarge_by = (ssize_t)n - (ssize_t)bytes_until_end;
		if (enlarge_by > 0) {
			buffer_.resize(buffer_.size() + enlarge_by);
		}
		size_t i = 0;
		for (auto it = write_pos_; i < n; ++i, ++it) {
			*it = buffer[i];
		}
		write_pos_ += n;
		return n;
	}
	
	inline size_t MemoryBufferStream::tell_write() const {
		return write_pos_ - buffer_.begin();
	}
	
	inline bool MemoryBufferStream::seek_write(size_t pos) {
		if (pos >= size()) {
			return false;
		}
		write_pos_ = buffer_.begin() + pos;
		return true;
	}
}

#endif
