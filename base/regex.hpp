//
//  regex.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 02/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_regex_hpp
#define grace_regex_hpp

#include "base/string.hpp"
#include "base/basic.hpp"
#include "base/array.hpp"
#include "io/formatted_stream.hpp"
#include "base/function.hpp"

namespace grace {
	enum RegexOptions : uint32 {
		None = 0,
		CaseInsensitive = 1 << ('i' - 'a'),
		NewlineSensitive = 1 << ('m' - 'a'),
	};
	
	struct Regex {
		Regex(IAllocator& alloc = default_allocator()) : pattern_(alloc) {}
		Regex(const char* utf8, IAllocator& alloc = default_allocator());
		Regex(const char* utf8, const char* options, IAllocator& alloc = default_allocator());
		Regex(const char* utf8, uint32 options, IAllocator& alloc = default_allocator());
		Regex(StringRef pattern, IAllocator& alloc = default_allocator());
		Regex(StringRef pattern, const char* options, IAllocator& alloc = default_allocator());
		Regex(StringRef pattern, uint32 options, IAllocator& alloc = default_allocator());
		Regex(Regex&& other);
		Regex(const Regex&, IAllocator& alloc = default_allocator());
		~Regex();
		Regex& operator=(Regex&& other);
		Regex& operator=(const Regex&);
		
		IAllocator& allocator() const;
		StringRef pattern() const;
		uint32 options() const;
		bool is_case_insensitive() const;
		bool is_newline_sensitive() const;
		
		struct SearchResults {
			Array<Array<StringRef>> matches;
			SearchResults(Array<Array<StringRef>>&& m) : matches(std::move(m)) {}
			SearchResults(const SearchResults&) = default;
			SearchResults(SearchResults&&) = default;
			
			using iterator = Array<Array<StringRef>>::const_iterator;
			iterator begin() const { return matches.begin(); }
			iterator end()   const { return matches.end(); }
			StringRef operator[](size_t idx) const { return matches[idx][0]; }
			size_t size() const { return matches.size(); }
		};
		
		bool match(StringRef haystack) const;
		SearchResults search(StringRef haystack, IAllocator& alloc = default_allocator()) const;
		void search(StringRef haystack, Function<void(ArrayRef<StringRef> match_groups)> callback) const;
	private:
		String pattern_;
		uint32 options_ = RegexOptions::None;
		void* regex_ = nullptr;
		
		void compile_regex();
		void parse_options(const char*);
		void check_error(int) const;
	};
	
	String replace(StringRef haystack, const Regex& pattern, StringRef replacement, IAllocator& alloc = default_allocator());
	String replace_with(StringRef haystack, const Regex& pattern, Function<String(ArrayRef<StringRef>)> replacement, IAllocator& alloc = default_allocator());
	FormattedStream& replace_with_through(StringRef haystack, const Regex& pattern, FormattedStream&, Function<void(FormattedStream&, ArrayRef<StringRef>)> replacement);
		
	inline IAllocator& Regex::allocator() const {
		return pattern_.allocator();
	}
		
	inline StringRef Regex::pattern() const {
		return pattern_;
	}
		
	inline uint32 Regex::options() const {
		return options_;
	}
		
		inline bool Regex::is_case_insensitive() const {
			return (options_ & RegexOptions::CaseInsensitive) != 0;
		}
		
		inline bool Regex::is_newline_sensitive() const {
			return (options_ & RegexOptions::NewlineSensitive) != 0;
		}
	
	class FormattedStream;
	inline FormattedStream& operator<<(FormattedStream& os, const Regex& rx) {
		os << '/';
		os << rx.pattern();
		os << '/';
		if (rx.is_case_insensitive()) {
			os << 'i';
		}
		if (rx.is_newline_sensitive()) {
			os << 'm';
		}
		return os;
	}
}

#endif
