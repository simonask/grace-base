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
#include <string>

namespace falling {
	enum FileWriteMode : uint8 {
		Truncate,
		Append,
	};
	
	class FileStreamBase {
	public:
		// FileStream API
		~FileStreamBase() { this->close(); }
		FileStreamBase& operator=(FileStreamBase&& other);
		const std::string& path() const { return path_; }
		void close();
		bool is_open() const;
		bool eof() const;
		size_t tell() const;
		bool seek(size_t pos);
		void seek_end();
		size_t file_size();
		
		// Public for a static_assert, not for external use.
		static const size_t ImplSize = sizeof(void*);
		struct Impl;
	protected:
		FileStreamBase();
		FileStreamBase(FileStreamBase&& other);
		
		byte impl_data_[ImplSize];
		Impl& impl();
		const Impl& impl() const;
		std::string path_;
	};
		
	class InputFileStream : public InputStream, public FileStreamBase {
	public:
		static InputFileStream open(std::string path);
		InputFileStream(InputFileStream&& other) = default;
		InputFileStream& operator=(InputFileStream&& other) = default;
		
		// InputStream API
		bool is_readable() const override;
		size_t read(byte* buffer, size_t n) override;
		size_t tell_read() const override;
		bool seek_read(size_t pos) override;
	private:
		InputFileStream() {}
	};
		
	class OutputFileStream : public OutputStream, public FileStreamBase {
	public:
		static OutputFileStream open(std::string path, FileWriteMode mode = FileWriteMode::Truncate);
		OutputFileStream(OutputFileStream&& other) = default;
		OutputFileStream& operator=(OutputFileStream&& other) = default;
		
		// OutputStream API
		bool is_writable() const override;
		size_t write(const byte* buffer, size_t n) override;
		size_t tell_write() const override;
		bool seek_write(size_t pos) override;
	private:
		OutputFileStream() {}
	};
}

#endif
