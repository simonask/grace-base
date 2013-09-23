#include "io/file_stream.hpp"
#include "base/stack_array.hpp"

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "base/raise.hpp"

namespace grace {
	const char* file_mode_to_mstr(FileMode mode) {
		switch (mode) {
			case FileMode::Read: return "r";
			case FileMode::ReadWrite: return "r+";
			case FileMode::WriteCreate: return "w";
			case FileMode::ReadWriteCreate: return "w+";
			case FileMode::AppendCreate: return "a";
			case FileMode::ReadAppendCreate: return "a+";
		}
	}

	FileStream FileStream::open(StringRef path, FileMode mode) {
		const char* mstr = file_mode_to_mstr(mode);
		COPY_STRING_REF_TO_CSTR_BUFFER(path_cstr, path);
		FILE* fp = ::fopen(path_cstr.data(), mstr);
		if (!fp) {
			raise<FileError>("fopen ({0}): {1}", path, ::strerror(errno));
		}
		FileStream f(fp, path, mode, false);
		return std::move(f);
	}

	FileStream::FileStream(void* fp, StringRef path, FileMode mode, bool autoflush) : fp_(fp), path_(path), mode_(mode), autoflush_(autoflush) {}
	FileStream::~FileStream() {
		close();
	}

	FileStream::FileStream(FileStream&& other) : fp_(nullptr) {
		swap(other);
	}

	FileStream& FileStream::operator=(FileStream&& other) {
		close();
		swap(other);
		return *this;
	}

	void FileStream::swap(FileStream& other) {
		std::swap(fp_, other.fp_);
		std::swap(path_, other.path_);
		std::swap(mode_, other.mode_);
		std::swap(autoflush_, other.autoflush_);
	}

	void FileStream::reopen(FileMode mode) {
		const char* mstr = file_mode_to_mstr(mode);
		COPY_STRING_REF_TO_CSTR_BUFFER(path_cstr, path_);
		fp_ = ::freopen(path_cstr.data(), mstr, (FILE*)fp_);
		if (!fp_) {
			raise<FileError>("freopen ({0}): {1}", path_, ::strerror(errno));
		}
	}

	FileMode FileStream::mode() const {
		return mode_;
	}

	StringRef FileStream::path() const {
		return path_;
	}

	void FileStream::close() {
		if (fp_) {
			::fclose((FILE*)fp_);
			fp_ = nullptr;
		}
	}

	bool FileStream::is_open() const {
		return fp_ != nullptr;
	}

	bool FileStream::eof() const {
		check_valid();
		return ::feof((FILE*)fp_);
	}

	size_t FileStream::tell() const {
		check_valid();
		return ::ftell((FILE*)fp_);
	}

	bool FileStream::seek(size_t pos) {
		check_valid();
		int r = ::fseek((FILE*)fp_, pos, SEEK_SET);
		if (r != 0) {
			raise<FileError>("fseek: {0}", ::strerror(errno));
		}
		return true;
	}

	bool FileStream::seek_end() {
		check_valid();
		int r = ::fseek((FILE*)fp_, 0, SEEK_END);
		if (r != 0) {
			raise<FileError>("fseek: {0}", ::strerror(errno));
		}
		return true;
	}

	size_t FileStream::file_size() const {
		struct stat s;
		::fstat(::fileno((FILE*)fp_), &s);
		return s.st_size;
	}

	bool FileStream::autoflush() const {
		return autoflush_;
	}

	void FileStream::set_autoflush(bool b) {
		autoflush_ = b;
	}

	uintptr_t FileStream::handle() const {
		check_valid();
		return ::fileno((FILE*)fp_);
	}

	bool FileStream::is_readable() const {
		return is_open() && (((uint8)mode() & FILE_MODE_READ_MASK) != 0);
	}

	size_t FileStream::read(byte* buffer, size_t n) {
		check_valid();
		size_t r = ::fread(buffer, 1, n, (FILE*)fp_);
		if (r < n && ::ferror((FILE*)fp_)) {
			raise<FileError>("fread: {0}", ::strerror(errno));
		}
		return r;
	}

	size_t FileStream::tell_read() const {
		return tell();
	}

	bool FileStream::seek_read(size_t pos) {
		return seek(pos);
	}

	bool FileStream::has_length() const {
		return true;
	}

	size_t FileStream::length() const {
		return file_size();
	}

	bool FileStream::is_writable() const {
		return is_open() && (((uint8)mode() & FILE_MODE_WRITE_MASK) != 0);
	}

	size_t FileStream::write(const byte* buffer, size_t n) {
		check_valid();
		size_t r = ::fwrite(buffer, 1, n, (FILE*)fp_);
		if (r < n) {
			raise<FileError>("fwrite: {0}", ::strerror(errno));
		}
		if (autoflush_) flush();
		return r;
	}

	size_t FileStream::tell_write() const {
		return tell();
	}

	bool FileStream::seek_write(size_t pos) {
		return seek(pos);
	}

	void FileStream::flush() {
		check_valid();
		::fflush((FILE*)fp_);
	}

	void FileStream::check_valid() const {
		if (!is_open()) {
			raise<FileError>("File isn't open.");
		}
	}
}