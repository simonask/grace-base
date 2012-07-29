//
//  file_stream.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/file_stream.hpp"

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
	
	FileStreamBase::FileStreamBase() {
		impl().fp = nullptr;
	}
	
	FileStreamBase::FileStreamBase(FileStreamBase&& other) {
		impl().fp = other.impl().fp;
		other.impl().fp = nullptr;
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
			auto r = fseek(impl().fp, 0, SEEK_END);
		}
	}
	
	size_t FileStreamBase::file_size() {
		size_t pos = tell();
		seek_end();
		size_t len = tell();
		seek(pos);
		return len;
	}
	
	InputFileStream InputFileStream::open(std::string path) {
		FILE* fp = fopen(path.c_str(), "r");
		InputFileStream fs;
		fs.impl().fp = fp;
		return fs;
	}
	
	bool InputFileStream::is_readable() const {
		return !eof();
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
	
	OutputFileStream OutputFileStream::open(std::string path, FileWriteMode mode) {
		const char* m = mode == FileWriteMode::Truncate ? "w" : "a";
		FILE* fp = fopen(path.c_str(), m);
		OutputFileStream fs;
		fs.impl().fp = fp;
		return fs;
	}
	
	bool OutputFileStream::is_writable() const {
		return is_open();
	}
	
	size_t OutputFileStream::write(const byte* buffer, size_t n) {
		if (is_open()) {
			return fwrite(buffer, 1, n, impl().fp);
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
