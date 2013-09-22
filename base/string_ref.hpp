#pragma once
#ifndef GRACE_STRING_REF_HPP_INCLUDED
#define GRACE_STRING_REF_HPP_INCLUDED

#include "base/basic.hpp"
#include "base/iterators.hpp"
#include "base/array_ref.hpp"

namespace grace {
	namespace detail {
		void string_index_out_of_bounds_exception(size_t idx, size_t sz);
	}
	
	struct StringRef {
	public:
		constexpr StringRef() : begin_(nullptr), end_(nullptr) {}
		constexpr StringRef(Empty e) : begin_(nullptr), end_(nullptr) {}
		template <size_t N>
		constexpr StringRef(const char(&data)[N]) : begin_(data), end_(data + N - 1) { }
		constexpr StringRef(const char* begin, const char* end) : begin_(begin), end_(end) { /*ASSERT(begin_ <= end_);*/ }
		StringRef(const char* utf8) : begin_(utf8), end_(utf8 ? utf8 + strlen(utf8) : utf8) {}
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

	ssize_t compare(StringRef a, StringRef b);

	inline ssize_t StringRef::compare(StringRef other) const {
		return grace::compare(*this, other);
	}

	inline ssize_t compare(StringRef a, StringRef b) {
		// TODO: Consider just using strcmp...
		for (size_t i = 0; i < a.size() && i < b.size(); ++i) {
			signed char diff = (signed char)a[i] - (signed char)b[i];
			if (diff != 0) return diff;
		}
		return (ssize_t)a.size() - (ssize_t)b.size();
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
}

#if defined(__has_feature) && __has_feature(cxx_user_literals)
inline constexpr grace::StringRef operator "" _C(const char* str, size_t length) {
	return grace::StringRef(str, str+length);
}
#endif

#endif
