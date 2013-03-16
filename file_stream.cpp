//
//  file_stream.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/file_stream.hpp"
#include "base/stack_array.hpp"

#include <stdio.h>

namespace falling {
	struct FileStreamBase::Impl {
		FILE* fp;
	};
	
	static_assert(sizeof(FileStreamBase::Impl) <= FileStreamBase::ImplSize, "FileStreamBase::Impl is too big. Increase the value of FileStreamBase::ImplSize.");
	
	FileStreamBase::Impl& FileStreamBase::impl() {
		return *reinterpret_cast<Impl*>(impl_data_);
	}
	
	const FileStreamBase::Impl& FileStreamBase::impl() const {
		return *reinterpret_cast<const Impl*>(impl_data_);
	}
	
	FileStreamBase::FileStreamBase() : synchronize_(false) {
		impl().fp = nullptr;
	}
	
	FileStreamBase::FileStreamBase(FileStreamBase&& other) {
		impl().fp = other.impl().fp;
		other.impl().fp = nullptr;
	}
	
	FileStreamBase::~FileStreamBase() {
		if (impl().fp != stdout && impl().fp != stderr && impl().fp != stdin) {
			close();
		}
	}
	
	FileStreamBase& FileStreamBase::operator=(FileStreamBase&& other) {
		close();
		impl().fp = other.impl().fp;
		other.impl().fp = nullptr;
		return *this;
	}
	
	void FileStreamBase::close() {
		if (impl().fp != nullptr) {
			fclose(impl().fp);
			impl().fp = nullptr;
		}
	}
	
	bool FileStreamBase::is_open() const {
		return impl().fp != nullptr;
	}
	
	bool FileStreamBase::eof() const {
		return !is_open() || feof(impl().fp);
	}
	
	size_t FileStreamBase::tell() const {
		if (is_open()) {
			auto r = ftell(impl().fp);
			ASSERT(r >= 0);
			return r;
		}
		return 0;
	}
	
	bool FileStreamBase::seek(size_t pos) {
		if (is_open()) {
			auto r = fseek(impl().fp, pos, SEEK_SET);
			return r == 0;
		}
		return false;
	}
	
	void FileStreamBase::seek_end() {
		if (is_open()) {
			fseek(impl().fp, 0, SEEK_END);
		}
	}
	
	size_t FileStreamBase::file_size() const {
		size_t pos = ftell(impl().fp);
		fseek(impl().fp, 0, SEEK_END);
		size_t len = ftell(impl().fp);
		fseek(impl().fp, pos, SEEK_SET);
		return len;
	}
	
	InputFileStream InputFileStream::open(StringRef path) {
		COPY_STRING_REF_TO_CSTR_BUFFER(path_buffer, path);
		FILE* fp = fopen(path_buffer.data(), "r");
		return wrap_file_pointer(fp);
	}
	
	InputFileStream InputFileStream::wrap_file_pointer(void *os_fp) {
		FILE* fp = (FILE*)os_fp;
		InputFileStream fs;
		fs.impl().fp = fp;
		return fs;
	}
	
	bool InputFileStream::is_readable() const {
		return is_open() && !eof();
	}
	
	size_t InputFileStream::read(byte* buffer, size_t n) {
		if (is_open()) {
			return fread(buffer, 1, n, impl().fp);
		}
		return 0;
	}
	
	size_t InputFileStream::tell_read() const {
		return tell();
	}
	
	bool InputFileStream::seek_read(size_t pos) {
		return seek(pos);
	}
	
	bool InputFileStream::has_length() const {
		return true;
	}
	
	size_t InputFileStream::length() const {
		return file_size();
	}
	
	OutputFileStream OutputFileStream::open(StringRef path, FileWriteMode mode) {
		const char* m = mode == FileWriteMode::Truncate ? "w" : "a";
		COPY_STRING_REF_TO_CSTR_BUFFER(path_buffer, path);
		FILE* fp = fopen(path_buffer.data(), m);
		return wrap_file_pointer(fp);
	}
	
	OutputFileStream OutputFileStream::wrap_file_pointer(void *os_fp) {
		FILE* fp = (FILE*)os_fp;
		OutputFileStream fs;
		fs.impl().fp = fp;
		return fs;
	}
	
	bool OutputFileStream::is_writable() const {
		return is_open();
	}
	
	size_t OutputFileStream::write(const byte* buffer, size_t n) {
		if (is_open()) {
			auto r = fwrite(buffer, 1, n, impl().fp);
			if (synchronize_) {
				fflush(impl().fp);
			}
			return r;
		}
		return 0;
	}
	
	size_t OutputFileStream::tell_write() const {
		return tell();
	}
	
	bool OutputFileStream::seek_write(size_t pos) {
		return seek(pos);
	}
}
