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

#include "base/array_list.hpp"

namespace falling {
	class MemoryStream : public InputStream {
	public:
		MemoryStream(const byte* begin, const byte* end) : begin_(begin), end_(end), current_(begin) {}
		// MemoryStream is safe to copy, because it doesn't own its data.
		MemoryStream(MemoryStream&& other) = default;
		~MemoryStream() {}
		MemoryStream& operator=(const MemoryStream& other) = default;
		
		// InputStream API
		bool is_readable() const override { return current_ < end_; }
		size_t read(byte* buffer, size_t max) override {
			size_t to_copy = data_available() < max ? data_available() : max;
			std::copy(current_, current_ + to_copy, buffer);
			current_ += to_copy;
			return to_copy;
		}
		size_t tell_read() const { return current_ - begin_; }
		bool seek_read(size_t position) {
			if (position > size()) {
				return false;
			}
			current_ = begin_ + position;
			return true;
		}
		bool has_length() const final { return true; }
		size_t length() const final {
			return size();
		}
		
		// MemoryStream API
		size_t size() const { return end_ - begin_; }
		size_t data_available() const { return end_ - current_; }
	private:
		const byte* begin_;
		const byte* end_;
		const byte* current_;
	};
	
	
	class MemoryBufferStream : public InputStream, public OutputStream {
	public:
		MemoryBufferStream(IAllocator& alloc = default_allocator()) : buffer_(alloc) {
			read_pos_ = write_pos_ = buffer_.begin();
		}
		MemoryBufferStream(const MemoryBufferStream& other, IAllocator& alloc = default_allocator());
		template <typename T>
		explicit MemoryBufferStream(T container, IAllocator& alloc = default_allocator()) {
			read_pos_ = write_pos_ = buffer_.begin();
			insert(std::begin(container), std::end(container));
		}
		template <typename InputIterator>
		MemoryBufferStream(InputIterator begin, InputIterator end, IAllocator& alloc = default_allocator()) {
			read_pos_ = write_pos_ = buffer_.begin();
			insert(begin, end);
		}

		MemoryBufferStream(MemoryBufferStream&& other);
		MemoryBufferStream& operator=(const MemoryBufferStream& other) = default;
		
		// InputStream API
		bool is_readable() const final;
		size_t read(byte* buffer, size_t max) final;
		size_t tell_read() const final;
		bool seek_read(size_t pos) final;
		bool has_length() const final;
		size_t length() const final;
		
		// OutputStream API
		bool is_writable() const final;
		size_t write(const byte* buffer, size_t max) final;
		size_t tell_write() const final;
		bool seek_write(size_t pos) final;
		
		// MemoryBufferStream API
		IAllocator& allocator() const { return buffer_.allocator(); }
		void clear();
		size_t size() const { return buffer_.size(); }
		size_t data_available() const { return buffer_.end() - read_pos_; }
		
		template <typename InputIterator>
		void insert(InputIterator begin, InputIterator end) {
			size_t rp = read_pos_ - buffer_.begin();
			size_t wp = write_pos_ - buffer_.begin();
			buffer_.insert(begin, end, write_pos_);
			read_pos_ = buffer_.begin() + rp;
			write_pos_ = buffer_.begin() + wp;
			write_pos_ += end - begin;
		}
		
		template <typename OutputIterator>
		size_t copy_to(OutputIterator begin, OutputIterator end) const {
			size_t output_len = end - begin;
			size_t input_len = buffer_.size();
			size_t len = output_len < input_len ? output_len : input_len;
			auto i0 = buffer_.begin();
			auto i1 = i0 + len;
			std::copy(i0, i1, begin);
			return len;
		}
	private:
		typedef typename ArrayList<byte>::iterator Position;
		ArrayList<byte> buffer_;
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
	
	inline bool MemoryBufferStream::has_length() const {
		return true;
	}
	
	inline size_t MemoryBufferStream::length() const {
		return buffer_.size();
	}
	
	inline bool MemoryBufferStream::is_writable() const {
		return true;
	}
	
	inline size_t MemoryBufferStream::write(const byte* buffer, size_t n) {
		size_t bytes_until_end = buffer_.end() - write_pos_;
		ssize_t enlarge_by = (ssize_t)n - (ssize_t)bytes_until_end;
		if (enlarge_by > 0) {
			size_t wp = tell_write();
			size_t rp = tell_read();
			buffer_.resize(buffer_.size() + enlarge_by);
			seek_write(wp);
			seek_read(rp);
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
	
	inline void MemoryBufferStream::clear() {
		buffer_.clear();
		read_pos_ = write_pos_ = buffer_.begin();
	}
}

#endif