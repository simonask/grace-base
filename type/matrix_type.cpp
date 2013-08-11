//
//  matrix_type.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "type/matrix_type.hpp"

namespace grace {
	String MatrixType::build_type_name_for_matrix(IAllocator& alloc, size_t cols, size_t rows) {
		StringStream ss;
		ss << "matrix" << cols << rows;
		return ss.string(alloc);
	}
}
