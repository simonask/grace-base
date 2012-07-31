//
//  vector.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 31/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/vector.hpp"
#include "base/vector_errors.hpp"

namespace falling {
	void throw_normalize_zero_length_vector_exception() {
		throw VectorNormalizeZeroLengthException();
	}
}
