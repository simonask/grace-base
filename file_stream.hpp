//
//  filestream.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 21/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_filestream_hpp
#define falling_filestream_hpp

#include "io/input_stream.hpp"
#include "io/output_stream.hpp"
#include "base/string.hpp"

namespace falling {
	enum FileWriteMode : uint8 {
		Truncate,
		Append,
	};
	
	class FileStreamBase {
	public:
		// FileStream API
		~FileStreamBase();
		FileStreamBase& operator=(FileStreamBase&& other);
		const String& path() const { return path_; }
		void close();
		bool is_open() const;
		bool eof() const;
		size_t tell() const;
		bool seek(size_t pos);
		void seek_end();
		size_t file_size() const;
		bool sync() const { return synchronize_; }
		void set_sync(bool b) { synchronize_ = b; }
		
		// Public for a static_assert, not for external use.
		static const size_t ImplSize = sizeof(void*);
		struct Impl;
	protected:
		FileStreamBase();
		FileStreamBase(FileStreamBase&& other);
		
		byte impl_data_[ImplSize];
		Impl& impl();
		const Impl& impl() const;
		String path_;
		bool synchronize_;
	};
		
	class InputFileStream : public InputStream, public FileStreamBase {
	public:
		static InputFileStream open(StringRef path);
		static InputFileStream wrap_file_pointer(void* os_fp);
		InputFileStream() {}
		InputFileStream(InputFileStream&& other) = default;
		InputFileStream& operator=(InputFileStream&& other) = default;
		
		// InputStream API
		bool is_readable() const override;
		size_t read(byte* buffer, size_t n) override;
		size_t tell_read() const override;
		bool seek_read(size_t pos) override;
		bool has_length() const final;
		size_t length() const final;
	};
		
	class OutputFileStream : public OutputStream, public FileStreamBase {
	public:
		static OutputFileStream open(StringRef path, FileWriteMode mode = FileWriteMode::Truncate);
		static OutputFileStream wrap_file_pointer(void* os_fp);
		OutputFileStream() {}
		OutputFileStream(OutputFileStream&& other) = default;
		OutputFileStream& operator=(OutputFileStream&& other) = default;
		
		// OutputStream API
		bool is_writable() const override;
		size_t write(const byte* buffer, size_t n) override;
		size_t tell_write() const override;
		bool seek_write(size_t pos) override;
	};
}

#endif
