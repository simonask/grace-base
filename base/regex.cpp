//
//  regex.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 28/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/regex.hpp"
#include "io/string_stream.hpp"
#include "base/stack_array.hpp"
#include "base/raise.hpp"
#include <regex.h>

namespace grace {
	struct RegexError : ErrorBase<RegexError> {};
	
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
		o |= REG_ENHANCED;
#if defined(__APPLE__)
		// Faster copyless Apple alternative:
		// TODO: Consider using REG_PEND option instead of regncomp
		int err = ::regncomp((regex_t*)regex_, pattern_.data(), pattern_.size(), o);
#else
		COPY_STRING_REF_TO_CSTR_BUFFER(pattern_buffer, pattern_);
		int err = ::regcomp((regex_t*)regex_, pattern_buffer.data(), o);
#endif
		check_error(err);
	}
	
	bool Regex::match(StringRef haystack) const {
		if (regex_ == nullptr) return false;
		regmatch_t matches[1];
#if defined(__APPLE__)
		// Faster copyless Apple alternative:
		int err = ::regnexec((regex_t*)regex_, haystack.data(), haystack.size(), 1, matches, 0);
		if (err == REG_EMPTY) return false;
#else
		COPY_STRING_REF_TO_CSTR_BUFFER(haystack_buffer, haystack);
		int err = ::regexec((regex_t*)regex_, haystack_buffer.data(), 1, matches, 0);
#endif
		if (err == REG_NOMATCH) return false;
		check_error(err);
		return true;
	}
	
	Regex::SearchResults Regex::search(StringRef haystack, IAllocator& alloc) const {
		Array<Array<StringRef>> matches(alloc);
		search(haystack, [&](ArrayRef<StringRef> match) {
			matches.emplace_back(match, alloc);
		});
		return std::move(matches);
	}

	void Regex::search(StringRef haystack, Function<void(ArrayRef<StringRef>)> callback) const {
		static const size_t MAX_SUBGROUPS = 10;
		if (regex_ != nullptr) {
			regmatch_t matches[MAX_SUBGROUPS];
			DEFINE_STACK_ARRAY(StringRef, match_groups, MAX_SUBGROUPS);
			const char* p = haystack.data();
			const char* end = p + haystack.size();
			while (p < end) {
#if defined(__APPLE__)
				// Faster copyless Apple alternative:
				int err = ::regnexec((regex_t*)regex_, p, end - p, MAX_SUBGROUPS, matches, 0);
#else
				COPY_STRING_REF_TO_CSTR_BUFFER(p_buffer, StringRef(p, end));
				int err = ::regexec((regex_t*)regex_, p_buffer.data(), MAX_SUBGROUPS, matches, 0);
#endif
				if (err == REG_NOMATCH) {
					break;
				}

				size_t num_matches = 0;
				for (size_t i = 0; i < MAX_SUBGROUPS; ++i) {
					if (matches[i].rm_so == -1) {
						break;
					}
					match_groups[i] = StringRef(p + matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so);
					++num_matches;
				}

				ArrayRef<StringRef> match_group_list(match_groups.data(), match_groups.data() + num_matches);
				callback(match_group_list);

				p = p + matches[0].rm_eo;
			}
		}
	}
	
	void Regex::check_error(int err) const {
		if (err != 0) {
			switch (err) {
				case REG_BADPAT:   raise<RegexError>("Bad pattern.");
				case REG_ECOLLATE: raise<RegexError>("Invalid collating element.");
				case REG_ECTYPE:   raise<RegexError>("Invalid character class.");
				case REG_EESCAPE:  raise<RegexError>("Invalid escape sequence.");
				case REG_ESUBREG:  raise<RegexError>("Invalid backreference number.");
				case REG_EBRACK:   raise<RegexError>("Unbalanced brackets.");
				case REG_EBRACE:   raise<RegexError>("Unbalanced braces.");
				case REG_EPAREN:   raise<RegexError>("Unbalanced parens.");
				case REG_BADBR:    raise<RegexError>("Invalid repetition count.");
				case REG_ERANGE:   raise<RegexError>("Invalid character in range.");
				case REG_ESPACE:   raise<RegexError>("Out of memory.");
				case REG_BADRPT:   raise<RegexError>("Invalid repeat operator.");
#if defined(__APPLE__)
				case REG_ASSERT:   UNREACHABLE();
				case REG_INVARG:   raise<RegexError>("Negative-length string.");
				case REG_ILLSEQ:   raise<RegexError>("Bad multibyte character.");
#endif
				default: UNREACHABLE();
			}
		}
	}
	
	String replace(StringRef haystack, const Regex& pattern, StringRef replacement, IAllocator& alloc) {
		return replace_with(haystack, pattern, [&](ArrayRef<StringRef> matches) {
			return String(replacement);
		});
	}

	String replace_with(StringRef haystack, const Regex& pattern, Function<String(ArrayRef<StringRef>)> replacement, IAllocator& alloc) {
		StringStream ss(alloc);
		replace_with_through(haystack, pattern, ss, [&](FormattedStream& os, ArrayRef<StringRef> matches) {
			os << replacement(matches);
		});
		return ss.string(alloc);
	}

	FormattedStream& replace_with_through(StringRef haystack, const Regex& pattern, FormattedStream& os, Function<void(FormattedStream& os, ArrayRef<StringRef>)> replacement) {
		const char* end = haystack.data() + haystack.size();
		const char* last_end = haystack.data();
		pattern.search(haystack, [&](ArrayRef<StringRef> match_groups) {
			StringRef whole_match = match_groups[0];
			os << StringRef(last_end, whole_match.data() - last_end);
			replacement(os, match_groups);
			last_end = whole_match.data() + whole_match.size();
		});
		os << StringRef(last_end, end - last_end);
		return os;
	}
}
