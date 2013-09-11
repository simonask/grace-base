//
//  exceptions.h
//  grace
//
//  Created by Simon Ask Ulsnes on 09/07/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef __grace__exceptions__
#define __grace__exceptions__

#include "base/error.hpp"

namespace grace {
	struct DLL_PUBLIC IndexOutOfBoundsException : ErrorBase<IndexOutOfBoundsException> {};
	struct DLL_PUBLIC BadAlignmentError : ErrorBase<BadAlignmentError> {};
	struct DLL_PUBLIC OutOfMemoryError : ErrorBase<OutOfMemoryError> {};
}

#endif /* defined(__grace__exceptions__) */
