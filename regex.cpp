//
//  regex.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 28/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/regex.hpp"
#include "io/string_stream.hpp"
#include <regex.h>

namespace grace {
	void Regex::parse_options(const char* options) {
		for (const char* o = options; *o; ++o) {
			switch (*o) {
				case 'i': options_ |= RegexOptions::CaseInsensitive; break;
				case 'm': options_ |= RegexOptions::NewlineSensitive; break;
				default: break;
			}
		}
	}
	
	Regex::Regex(const char* utf8, IAllocator& alloc) : pattern_(utf8, alloc) {
		compile_regex();
	}
	
	Regex::Regex(const char* utf8, const char* options, IAllocator& alloc) : pattern_(utf8, alloc) {
		parse_options(options);
		compile_regex();
	}
	
	Regex::Regex(const char* utf8, uint32 options, IAllocator& alloc) : pattern_(utf8, alloc), options_(options) {
		compile_regex();
	}
	
	Regex::Regex(StringRef pattern, IAllocator& alloc) : pattern_(pattern, alloc) {
		compile_regex();
	}
	
	Regex::Regex(StringRef pattern, const char* options, IAllocator& alloc) : pattern_(pattern, alloc) {
		parse_options(options);
		compile_regex();
	}
	
	Regex::Regex(StringRef pattern, uint32 options, IAllocator& alloc) : pattern_(pattern, alloc), options_(options) {
		compile_regex();
	}
	
	Regex::Regex(Regex&& other)
	: pattern_(move(other.pattern_))
	, regex_(other.regex_)
	, options_(other.options_)
	{
		other.regex_ = nullptr;
		other.options_ = RegexOptions::None;
	}
	
	Regex::Regex(const Regex& other, IAllocator& alloc)
	: pattern_(other.pattern_, alloc)
	, options_(other.options_)
	{
		compile_regex();
	}
	
	Regex::~Regex() {
		if (regex_) {
			regfree((regex_t*)regex_);
			allocator().free(regex_, sizeof(regex_t));
		}
	}
	
	Regex& Regex::operator=(Regex&& other) {
		if (regex_) {
			regfree((regex_t*)regex_);
			allocator().free(regex_, sizeof(regex_t));
			regex_ = nullptr;
		}
		regex_ = other.regex_;
		other.regex_ = nullptr;
		pattern_ = move(other.pattern_);
		options_ = other.options_;
		other.options_ = RegexOptions::None;
		return *this;
	}
	
	Regex& Regex::operator=(const Regex& other) {
		pattern_ = other.pattern_;
		options_ = other.options_;
		compile_regex();
		return *this;
	}
	
	void Regex::compile_regex() {
		if (regex_) {
			regfree((regex_t*)regex_);
		} else {
			regex_ = allocator().allocate(sizeof(regex_t), alignof(regex_t));
		}
		int o = 0;
		if (is_case_insensitive())  o |= REG_ICASE;
		if (is_newline_sensitive()) o |= REG_NEWLINE;
		o |= REG_EXTENDED;
		// TODO: Consider using REG_PEND option instead of regncomp
		int err = ::regncomp((regex_t*)regex_, pattern_.data(), pattern_.size(), o);
		check_error(err);
	}
	
	bool Regex::match(StringRef haystack) const {
		if (regex_ == nullptr) return false;
		regmatch_t matches[1];
		int err = ::regnexec((regex_t*)regex_, haystack.data(), haystack.size(), 1, matches, 0);
		if (err == REG_NOMATCH) return false;
		if (err == REG_EMPTY) return false;
		check_error(err);
		return true;
	}
	
	Regex::SearchResults Regex::search(StringRef haystack, IAllocator& alloc) const {
		Array<StringRef> matches(alloc);
		if (regex_ != nullptr) {
			regmatch_t match;
			const char* p = haystack.data();
			const char* end = p + haystack.size();
			while (p < end) {
				int err = ::regnexec((regex_t*)regex_, p, end - p, 1, &match, 0);
				if (err == REG_NOMATCH) {
					break;
				}
				matches.push_back(StringRef(p + match.rm_so, match.rm_eo - match.rm_so));
				p = p + match.rm_eo;
			}
		}
		return move(matches);
	}
	
	void Regex::check_error(int err) const {
		if (err != 0) {
			switch (err) {
				case REG_BADPAT: throw RegexError("Bad pattern.");
				case REG_ECOLLATE: throw RegexError("Invalid collating element.");
				case REG_ECTYPE: throw RegexError("Invalid character class.");
				case REG_EESCAPE: throw RegexError("Invalid escape sequence.");
				case REG_ESUBREG: throw RegexError("Invalid backreference number.");
				case REG_EBRACK: throw RegexError("Unbalanced brackets.");
				case REG_EBRACE: throw RegexError("Unbalanced braces.");
				case REG_EPAREN: throw RegexError("Unbalanced parens.");
				case REG_BADBR:  throw RegexError("Invalid repetition count.");
				case REG_ERANGE: throw RegexError("Invalid character in range.");
				case REG_ESPACE: throw RegexError("Out of memory.");
				case REG_BADRPT: throw RegexError("Invalid repeat operator.");
				case REG_ASSERT: UNREACHABLE();
				case REG_INVARG: throw RegexError("Negative-length string.");
				case REG_ILLSEQ: throw RegexError("Bad multibyte character.");
				default: UNREACHABLE();
			}
		}
	}
	
	String replace(StringRef haystack, Regex pattern, StringRef replacement, IAllocator& alloc) {
		auto results = pattern.search(haystack, alloc);
		StringStream ss(alloc);
		const char* p = haystack.data();
		const char* end = p + haystack.size();
		for (auto& match: results) {
			if (match.data() > p) {
				ss.write((const byte*)p, match.data() - p);
			}
			p = match.data() + match.size();
			ss << replacement;
		}
		ss.write((const byte*)p, end - p);
		return ss.string(alloc);
	}
}
