//
//  string.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 25/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_string_hpp
#define grace_string_hpp

#include "base/basic.hpp"
#include "base/array_ref.hpp"
#include "memory/allocator.hpp"
#include "base/iterators.hpp"
#include "base/string_ref.hpp"
#include <algorithm> // for std::copy

namespace grace {
	class String {
	public:
		static const ssize_t NPos = SSIZE_MAX;
	
		explicit String(IAllocator& alloc = default_allocator()) : allocator_(alloc) {}
		String(const char* utf8, IAllocator& alloc = default_allocator());
		String(const char* utf8, size_t len, IAllocator& alloc = default_allocator());
		String(const String& other, IAllocator& alloc);
		String(const String& other);
		explicit String(StringRef other, IAllocator& alloc = default_allocator());
		String(String&& other);
		~String();
		String& operator=(const char* utf8);
		String& operator=(StringRef other);
		String& operator=(const String& other);
		String& operator=(String&& other);
		void swap(String& other);
		
		IAllocator& allocator() const { return allocator_; }
		
		ssize_t compare(StringRef other/*, Collation collation = default_collation()*/) const;
		ssize_t compare(const char* other/*, Collation collation = default_collation()*/) const;
		bool operator==(StringRef other) const;
		bool operator!=(StringRef other) const;
		bool operator< (StringRef other) const;
		bool operator> (StringRef other) const;
		bool operator<=(StringRef other) const;
		bool operator>=(StringRef other) const;

		char operator[](size_t idx) const;
		
		String operator+(StringRef other) const;
		template <size_t N>
		String operator+(const char(&other)[N]) const;
		String operator+(const char* other) const;
		String operator+(char) const;
		
		operator StringRef() const;
		
		const char* data() const { return data_; }
		size_t size() const;
		
		char front() const { return (*this)[0]; }
		char back() const { return (*this)[size()-1]; }
		
		StringRef substr(ssize_t pos, ssize_t len = NPos) const;
		
		using const_iterator = LinearMemoryIterator<char, true>;
		using iterator = const_iterator;
		const_iterator begin() const { return const_iterator(data_); }
		const_iterator end() const { return const_iterator(data_ + size_); }
		
		static String take_ownership(IAllocator& alloc, const char* utf8, size_t size); // WARNING: utf8 *MUST* be allocated with alloc.
		
		struct Algorithms;
		friend struct Algorithms;
	private:
		IAllocator& allocator_;
		const char* data_ = nullptr;
		size_t size_ = 0;
		
		void assign(const char* utf8);
		void assign(const char* utf8, size_t sz);
		void clear();
	};
	
	/*
		Substring.
		
		pos: the offset from the beginning of the string. If negative, the offset will count back from the end of
		     the string.
		len: length of the requested string. If longer than the input, the result will be cut off. If negative,
			 the result will go from pos to length-N, i.e. providing -n will skip the last n characters of the result.
	*/
	StringRef substr(StringRef, ssize_t pos, ssize_t len = String::NPos);
	size_t find(StringRef, char letter);
	size_t find(StringRef, const StringRef& substring);
	size_t rfind(StringRef, char letter);
	size_t rfind(StringRef, const StringRef& substring);
	StringRef lstrip(StringRef s, char l = ' ');
	StringRef rstrip(StringRef s, char l = ' ');
	StringRef strip(StringRef s, char l = ' ');
	String reverse(StringRef str, IAllocator& alloc = default_allocator());
	String reverse(String&& str, IAllocator& alloc);
	String reverse(String&& str);
	String concatenate(StringRef a, StringRef b, IAllocator& alloc = default_allocator());
	String replace(const String& str, String::iterator pos, char new_letter);
	String replace(String&& str, String::iterator pos, char new_letter);
	String replace(const String& str, String::iterator pos, char new_letter);
	String replace(String&& str, String::iterator pos, StringRef new_substring);
	String join(ArrayRef<const StringRef>, StringRef delimiter = ", ", IAllocator& alloc = default_allocator());
	String encapsulate_join(ArrayRef<const StringRef>, StringRef begin, StringRef end, StringRef delimiter = ", ", IAllocator& alloc = default_allocator());
	
	inline String::String(const char* utf8, IAllocator& alloc) : allocator_(alloc) {
		assign(utf8);
	}
	
	inline String::String(const char* utf8, size_t len, IAllocator& alloc) : allocator_(alloc) {
		assign(utf8, len);
	}
	
	inline String::String(StringRef other, IAllocator& alloc) : allocator_(alloc) {
		assign(other.data(), other.size());
	}
	
	inline String::String(const String& other, IAllocator& alloc) : allocator_(alloc) {
		assign(other.data(), other.size());
	}
	
	inline String::String(const String& other) : String(other, default_allocator()) {
	}
	
	inline String::String(String&& other) : allocator_(other.allocator_), data_(nullptr), size_(0) {
		swap(other);
	}
	
	inline String::~String() {
		clear();
	}
	
	inline String& String::operator=(const char* utf8) {
		if (utf8 >= data_ && utf8 < data_ + size_) {
			// assigning a pointer to the inside of this string!!
			size_t len = strlen(utf8);
			char buffer[len];
			memcpy(buffer, utf8, len);
			clear();
			assign(buffer, len);
		} else {
			clear();
			assign(utf8);
		}
		return *this;
	}
	
	inline String& String::operator=(StringRef other) {
		if (other.data() >= data_ && other.data() <= data_ + size_) {
			// StringRef is a substring of this!
			size_t len = other.size();
			char buffer[len];
			memcpy(buffer, other.data(), len);
			clear();
			assign(buffer, len);
		} else {
			clear();
			assign(other.data(), other.size());
		}
		return *this;
	}
	
	inline String& String::operator=(const String& other) {
		if (this == &other) {
			return *this;
		}
		clear();
		assign(other.data(), other.size());
		return *this;
	}
	
	inline String& String::operator=(String&& other) {
		if (this == &other) {
			return *this;
		}
		if (&allocator_ == &other.allocator_) {
			std::swap(data_, other.data_);
			std::swap(size_, other.size_);
		} else {
			clear();
			assign(other.data_, other.size_);
			other.clear();
		}
		return *this;
	}
	
	inline void String::swap(String& other) {
		if (&allocator() == &other.allocator()) {
			std::swap(data_, other.data_);
			std::swap(size_, other.size_);
		} else {
			String tmp(move(*this));
			*this = other;
			other = move(tmp);
		}
	}
	
	inline String::operator StringRef() const {
		return StringRef(data_, data_ + size_);
	}
	
	inline char String::operator[](size_t idx) const {
		if (idx > size_) {
			detail::string_index_out_of_bounds_exception(idx, size_);
		}
		return data_[idx];
	}
	
	inline ssize_t String::compare(StringRef other) const {
		return grace::compare(*this, other);
	}
	
	inline bool String::operator==(StringRef other) const {
		return (size_ == other.size()) && (compare(other) == 0);
	}
	
	inline bool String::operator!=(StringRef other) const {
		return (size_ != other.size()) || (compare(other) != 0);
	}
	
	inline bool String::operator<(StringRef other) const {
		return compare(other) < 0;
	}
	
	inline bool String::operator>(StringRef other) const {
		return compare(other) > 0;
	}
	
	inline bool String::operator<=(StringRef other) const {
		return compare(other) <= 0;
	}
	
	inline bool String::operator>=(StringRef other) const {
		return compare(other) >= 0;
	}
	
	inline String String::operator+(StringRef other) const {
		return concatenate(*this, other, allocator_);
	}
	
	template <size_t N>
	inline String String::operator+(const char(&other)[N]) const {
		return concatenate(*this, StringRef(other, other+N), allocator_);
	}
	
	inline String String::operator+(const char* utf8) const {
		return concatenate(*this, StringRef(utf8, utf8+strlen(utf8)), allocator_);
	}
	
	inline String String::operator+(char c) const {
		return concatenate(*this, StringRef(&c, &c+1), allocator_);
	}
	
	inline size_t String::size() const {
		return size_;
	}
	
	inline StringRef String::substr(ssize_t b, ssize_t len) const {
		return grace::substr(*this, b, len);
	}
	
	inline void String::assign(const char* utf8) {
		assign(utf8, utf8 ? strlen(utf8) : 0);
	}
	
	inline void String::assign(const char* utf8, size_t len) {
		data_ = len ? (char*)allocator_.allocate(len, 1) : nullptr;
		size_ = len;
		std::copy(utf8, utf8 + len, (char*)data_);
	}
	
	inline void String::clear() {
		allocator_.free((void*)data_, size_);
		data_ = nullptr;
		size_ = 0;
	}
}

template <size_t N>
inline grace::String operator+(const char(&a)[N], const grace::String& b) {
	return grace::String(a) + b;
}

#endif
