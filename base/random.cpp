//
//  random.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/07/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "base/random.hpp"
#include "base/time.hpp"

namespace grace {
	Random::Random() : engine_(system_now().nanoseconds_since_epoch() & UINT32_MAX) {}
}
