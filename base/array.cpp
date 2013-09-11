//
//  array.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 04/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "base/array.hpp"
#include "base/string.hpp"
#include "base/raise.hpp"
#include "base/exceptions.hpp"

namespace grace {
	template class Array<byte>;
	template class Array<float32>;
	template class Array<uint32>;
	template class Array<StringRef>;
	template class Array<String>;
	template class Array<IAttribute*>;
	template class Array<ISlot*>;
	template class Array<const IType*>;

	namespace detail {
		void array_index_out_of_bounds(size_t idx, size_t max) {
			raise<IndexOutOfBoundsException>("Requested index {0} from Array of size {1}.", idx, max);
		}
	}
}
