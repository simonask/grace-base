//
//  array_ref.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 04/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "base/array_ref.hpp"
#include "base/string.hpp"

namespace falling {
	template struct ArrayRef<byte>;
	template struct ArrayRef<char>;
	template struct ArrayRef<float32>;
	template struct ArrayRef<String>;
	template struct ArrayRef<StringRef>;
	template struct ArrayRef<const Type*>;
	template struct ArrayRef<const IAttribute*>;
	template struct ArrayRef<const ISlot* const>;
}
