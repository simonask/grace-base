//
//  exceptions.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 09/07/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "base/exceptions.hpp"
#include "base/string.hpp"

namespace grace {
	StringRef IndexOutOfBoundsException::what() const {
		return "Index out of bounds.";
	}
	
	StringRef OutOfMemoryError::what() const {
		return "Out of memory";
	}
	
	StringRef BadAlignmentError::what() const {
		return "Bad alignment";
	}
	
	
}
