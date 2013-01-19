//
//  string.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 25/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_string_hpp
#define falling_string_hpp

#include "base/basic.hpp"
#include "base/array_ref.hpp"
#include "memory/allocator.hpp"
#include "base/iterators.hpp"
#include <algorithm> // for std::copy

namespace falling {
	struct StringRef {
	public:
		constexpr StringRef() : begin_(nullptr), end_(nullptr) {}
		constexpr StringRef(Empty e) : begin_(nullptr), end_(nullptr) {}
		template <size_t N>
		constexpr StringRef(const char(&data)[N]) : begin_(data), end_(data + N - 1) { }
		constexpr StringRef(const char* begin, const char* end) : begin_(begin), end_(end) { /*ASSERT(begin_ <= end_);*/ }
		StringRef(const char* utf8) : begin_(utf8), end_(utf8 + strlen(utf8)) {}
		StringRef(const char* utf8, size_t len) : begin_(utf8), end_(utf8 + len) {}
		constexpr StringRef(const ArrayRef<char>& array) : begin_(array.data()), end_(array.data() + array.size()) {}
		StringRef(const StringRef& other) = default;
		StringRef(StringRef&& other) = default;
		StringRef& operator=(const StringRef& other) = default;
		StringRef& operator=(StringRef&& other) = default;

		ssize_t compare(StringRef other) const;
		bool operator==(StringRef other) const;
		bool operator!=(StringRef other) const;
		bool operator< (StringRef other) const;
		bool operator> (StringRef other) const;
		bool operator<=(StringRef other) const;
		bool operator>=(StringRef other) const;
		
		size_t size() const { return end_ - begin_; }
		
		char operator[](size_t idx) const { ASSERT(idx < size()); return begin_[idx]; }
		
		typedef const char value_type;
		using const_iterator = LinearMemoryIterator<char, true>;
		using iterator = const_iterator;
		const_iterator begin() const { return iterator(begin_); }
		const_iterator end() const { return iterator(end_); }
		
		const char* data() const { return begin_; }
	private:
		const char* begin_;
		const char* end_;
	};
	
	class String {
	public:
		static const size_t NPos = SIZE_MAX;
	
		explicit String(IAllocator& alloc = default_allocator()) : allocator_(alloc) {}
		String(const char* utf8, IAllocator& alloc = default_allocator());
		String(const char* utf8, size_t len, IAllocator& alloc = default_allocator());
		String(const String& other, IAllocator& alloc = default_allocator());
		explicit String(StringRef other, IAllocator& alloc = default_allocator());
		String(String&& other);
		~String();
		String& operator=(const char* utf8);
		String& operator=(StringRef other);
		String& operator=(const String& other);
		String& operator=(String&& other);
		
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
		
		StringRef substr(size_t pos, size_t len = NPos) const;
		
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
	
	ssize_t compare(StringRef a, StringRef b);
	StringRef substr(StringRef, size_t pos, size_t len = String::NPos);
	size_t find(StringRef, char letter);
	size_t find(StringRef, const StringRef& substring);
	size_t rfind(StringRef, char letter);
	size_t rfind(StringRef, const StringRef& substring);
	String reverse(StringRef str, IAllocator& alloc = default_allocator());
	String reverse(String&& str, IAllocator& alloc);
	String reverse(String&& str);
	String concatenate(StringRef a, StringRef b, IAllocator& alloc = default_allocator());
	String replace(const String& str, String::iterator pos, char new_letter);
	String replace(String&& str, String::iterator pos, char new_letter);
	String replace(const String& str, String::iterator pos, char new_letter);
	String replace(String&& str, String::iterator pos, StringRef new_substring);
	
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
	
	inline String::String(String&& other) : allocator_(other.allocator_), data_(nullptr), size_(0) {
		*this = std::move(other);
	}
	
	inline String::~String() {
		clear();
	}
	
	inline String& String::operator=(const char* utf8) {
		clear();
		assign(utf8);
		return *this;
	}
	
	inline String& String::operator=(StringRef other) {
		clear();
		assign(other.data(), other.size());
		return *this;
	}
	
	inline String& String::operator=(const String& other) {
		clear();
		assign(other.data(), other.size());
		return *this;
	}
	
	inline String& String::operator=(String&& other) {
		clear();
		if (&allocator_ == &other.allocator_) {
			data_ = other.data_;
			size_ = other.size_;
			other.data_ = nullptr;
			other.size_ = 0;
		} else {
			assign(other.data_, other.size_);
		}
		return *this;
	}
	
	inline String::operator StringRef() const {
		return StringRef(data_, data_ + size_);
	}
	
	inline char String::operator[](size_t idx) const {
		if (idx > size_) {
			throw IndexOutOfBoundsException();
		}
		return data_[idx];
	}

	inline ssize_t StringRef::compare(StringRef other) const {
		return falling::compare(*this, other);
	}
	
	inline ssize_t String::compare(StringRef other) const {
		return falling::compare(*this, other);
	}

	inline ssize_t compare(StringRef a, StringRef b) {
		// TODO: Consider just using strcmp...
		for (size_t i = 0; i < a.size() && i < b.size(); ++i) {
			signed char diff = (signed char)a[i] - (signed char)b[i];
			if (diff != 0) return diff;
		}
		return (ssize_t)a.size() - (ssize_t)b.size();
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

	inline bool StringRef::operator==(StringRef other) const {
		return (size() == other.size()) && (compare(other) == 0);
	}
	
	inline bool StringRef::operator!=(StringRef other) const {
		return (size() != other.size()) || (compare(other) != 0);
	}
	
	inline bool StringRef::operator<(StringRef other) const {
		return compare(other) < 0;
	}
	
	inline bool StringRef::operator>(StringRef other) const {
		return compare(other) > 0;
	}
	
	inline bool StringRef::operator<=(StringRef other) const {
		return compare(other) <= 0;
	}
	
	inline bool StringRef::operator>=(StringRef other) const {
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
	
	inline StringRef String::substr(size_t b, size_t len) const {
		if (b > size_) {
			throw IndexOutOfBoundsException();
		}
		if (len == NPos) {
			len = size_ - b;
		}
		size_t end_pos = b + len;
		if (end_pos > size_) {
			throw IndexOutOfBoundsException();
		}
		return StringRef(data_ + b, data_ + end_pos);
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

#if defined(__has_feature) && __has_feature(cxx_user_literals)
inline constexpr falling::StringRef operator "" _C(const char* str, size_t length) {
	return falling::StringRef(str, str+length);
}
#endif

template <size_t N>
inline falling::String operator+(const char(&a)[N], const falling::String& b) {
	return falling::String(a) + b;
}

#endif
