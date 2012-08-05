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
	/*template struct TVector<float32, 4>;
	template struct TVector<float32, 3>;
	template struct TVector<float32, 2>;
	template struct TVector<float32, 1>;
	template struct TVector<int32, 4>;
	template struct TVector<int32, 3>;
	template struct TVector<int32, 2>;
	template struct TVector<int32, 1>;
	template struct TVector<uint32, 4>;
	template struct TVector<uint32, 3>;
	template struct TVector<uint32, 2>;
	template struct TVector<uint32, 1>;*/
	
	void throw_normalize_zero_length_vector_exception() {
		throw VectorNormalizeZeroLengthException();
	}
}
