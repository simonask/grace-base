//
//  vector_errors.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 31/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_vector_errors_hpp
#define falling_vector_errors_hpp

#include "vector.hpp"
#include <string>

namespace falling {
	struct VectorNormalizeZeroLengthException {
		std::string what() const { return "Tried to normalize a zero-length vector."; }
	};
}

#endif
