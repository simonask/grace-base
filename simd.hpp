//
//  simd.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 27/05/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_simd_hpp
#define falling_simd_hpp

// TODO: Provide implementations for lower-than-SSE4.2 architectures and ARM NEON.


#if defined(__SSE__) && defined(__SSE2__)
#define USE_SSE 1
#endif


namespace falling {
	enum Axis {
		X = 0,
		Y = 1,
		Z = 2,
		W = 3,
	};
}

#if defined(USE_SSE)
#include "base/simd_sse.hpp"
#else
#include "base/simd_plain.hpp"
#endif


#endif
