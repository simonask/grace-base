//
//  vector.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 31/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "geometry/vector.hpp"
#include "geometry/vector_errors.hpp"
#include "base/raise.hpp"

namespace grace {
	template struct TVector<float32, 4>;
	template struct TVector<float32, 3>;
	template struct TVector<float32, 2>;
	template struct TVector<int32, 4>;
	template struct TVector<int32, 3>;
	template struct TVector<int32, 2>;
	template struct TVector<uint32, 4>;
	template struct TVector<uint32, 3>;
	template struct TVector<uint32, 2>;
	
	void throw_normalize_zero_length_vector_exception() {
		raise<VectorNormalizeZeroLengthException>();
	}
}
