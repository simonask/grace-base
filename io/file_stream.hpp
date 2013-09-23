//
//  filestream.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 21/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_filestream_hpp
#define grace_filestream_hpp

#include "io/input_stream.hpp"
#include "io/output_stream.hpp"
#include "base/string.hpp"
#include "base/error.hpp"

namespace grace {
	struct FileError : ErrorBase<FileError> {};

	static const uint8 FILE_MODE_READ_MASK   = 0x01;
	static const uint8 FILE_MODE_WRITE_MASK  = 0x02;
	static const uint8 FILE_MODE_CREATE_MASK = 0x4;
	static const uint8 FILE_MODE_APPEND_MASK = 0x10 | FILE_MODE_WRITE_MASK;

	enum class FileMode : uint8 {
		Read              = FILE_MODE_READ_MASK,
		ReadWrite         = FILE_MODE_READ_MASK | FILE_MODE_WRITE_MASK,
		WriteCreate       = FILE_MODE_WRITE_MASK | FILE_MODE_CREATE_MASK,
		ReadWriteCreate   = FILE_MODE_READ_MASK | FILE_MODE_WRITE_MASK | FILE_MODE_CREATE_MASK,
		AppendCreate      = FILE_MODE_CREATE_MASK | FILE_MODE_APPEND_MASK,
		ReadAppendCreate  = FILE_MODE_READ_MASK | FILE_MODE_APPEND_MASK | FILE_MODE_CREATE_MASK,
	};

	struct FileStream : IInputStream, IOutputStream {
		static FileStream open(StringRef path, FileMode mode = FileMode::ReadWriteCreate);

		FileStream(FileStream&& other);
		FileStream& operator=(FileStream&& other);
		~FileStream();
		void swap(FileStream& other);

		// FileStream
		void reopen(FileMode new_mode);
		FileMode mode() const;
		StringRef path() const;
		void close();
		bool is_open() const;
		bool eof() const;
		size_t tell() const;
		bool seek(size_t pos);
		bool seek_end();
		size_t file_size() const;
		bool autoflush() const;
		void set_autoflush(bool b);
		uintptr_t handle() const; // fd

		// InputStream
		bool is_readable() const final;
		size_t read(byte* buffer, size_t n) final;
		size_t tell_read() const final;
		bool seek_read(size_t pos) final;
		bool has_length() const final;
		size_t length() const final;

		// OutputStream
		bool is_writable() const final;
		size_t write(const byte* buffer, size_t n) final;
		size_t tell_write() const final;
		bool seek_write(size_t pos) final;
		void flush() final;
	protected:
		FileStream() {}
		FileStream(void* fp, StringRef path, FileMode mode, bool autoflush);

		void* fp_;
		String path_;
		FileMode mode_;
		bool autoflush_ = false;

		void check_valid() const;
	};
}

#endif
