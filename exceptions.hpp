//
//  exceptions.h
//  grace
//
//  Created by Simon Ask Ulsnes on 09/07/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef __grace__exceptions__
#define __grace__exceptions__

#include "base/basic.hpp"

namespace grace {
	struct StringRef;
	
	struct IException {
		virtual StringRef what() const = 0;
	};
	
	struct DLL_PUBLIC IndexOutOfBoundsException : IException {
		StringRef what() const final;
	};
	
	struct DLL_PUBLIC BadAlignmentError : IException {
		StringRef what() const final;
	};
	
	struct DLL_PUBLIC OutOfMemoryError : IException {
		StringRef what() const final;
	};
}

#endif /* defined(__grace__exceptions__) */
