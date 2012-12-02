//
//  regex.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 02/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_regex_hpp
#define falling_regex_hpp

#include <regex>
#include "base/string.hpp"

#include "base/basic.hpp"

namespace falling {
	struct Regex;
}

namespace std {
	void swap(falling::Regex&, falling::Regex&);
}

namespace falling {
	/*
	 This class is mostly necessary because std::regex doesn't save a
	 string representation of the regex, so it can't be printed.
	 */
	struct Regex {
		Regex(String definition, std::regex::flag_type flags = std::regex_constants::ECMAScript);
		Regex(Regex&&) = default;
		Regex(const Regex&) = default;
		Regex& operator=(Regex&&) = default;
		Regex& operator=(const Regex&) = default;
		
		operator const String&() const { return definition; }
		operator const std::regex&() const { return regex; }
		
		FORWARD_TO_MEMBER(mark_count, regex, std::regex)
		FORWARD_TO_MEMBER(flags, regex, std::regex)
		FORWARD_TO_MEMBER(getloc, regex, std::regex)
		
		template <typename BidirectionalIterator>
		bool match(BidirectionalIterator begin, BidirectionalIterator end) const;
		template <typename BidirectionalIterator>
		bool match(BidirectionalIterator begin, BidirectionalIterator end, std::match_results<BidirectionalIterator>& out_results) const;
		template <typename BidirectionalIterator>
		bool search(BidirectionalIterator begin, BidirectionalIterator end) const;
		
		// TODO:
		template <typename OutputIterator, typename BidirectionalIterator>
		OutputIterator replace(OutputIterator out, BidirectionalIterator begin, BidirectionalIterator end);
	private:
		String definition;
		std::regex regex;
		friend void std::swap(Regex&, Regex&);
	};
	
	inline Regex::Regex(String definition, std::regex::flag_type flags)
	: definition(std::move(definition))
	, regex(this->definition.data(), this->definition.size(), flags)
	{}
	
	template <typename BidirectionalIterator>
	bool Regex::match(BidirectionalIterator begin, BidirectionalIterator end) const {
		return std::regex_match(begin, end, regex);
	}
	
	template <typename BidirectionalIterator>
	bool Regex::match(BidirectionalIterator begin, BidirectionalIterator end, std::match_results<BidirectionalIterator>& out_results) const {
		return std::regex_match(begin, end, out_results, regex);
	}
	
	template <typename BidirectionalIterator>
	bool Regex::search(BidirectionalIterator begin, BidirectionalIterator end) const {
		return std::regex_search(begin, end, regex);
	}
	
	
}

namespace std {
	inline void swap(falling::Regex& lhs, falling::Regex& rhs) {
		std::swap(lhs.definition, rhs.definition);
		std::swap(lhs.regex, rhs.regex);
	}
}

#endif
