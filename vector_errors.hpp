//
//  vector_errors.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 31/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_vector_errors_hpp
#define grace_vector_errors_hpp

#include "vector.hpp"
#include "base/string.hpp"

namespace grace {
	struct DLL_PUBLIC VectorNormalizeZeroLengthException {
		String what() const { return "Tried to normalize a zero-length vector."; }
	};
}

#endif
