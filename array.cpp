//
//  array.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 04/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "base/array.hpp"
#include "base/string.hpp"

namespace falling {
	template class Array<byte>;
	template class Array<float32>;
	template class Array<uint32>;
	template class Array<StringRef>;
	template class Array<String>;
	template class Array<IAttribute*>;
	template class Array<ISlot*>;
	template class Array<const Type*>;
}
