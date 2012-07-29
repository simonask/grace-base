//
//  color.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 01/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/color.hpp"

namespace falling {
#define DEF_COLOR(NAME, R, G, B) const Color Color::NAME = Color(R, G, B, 1.f)
	DEF_COLOR(White, 1.f, 1.f, 1.f);
	DEF_COLOR(Black, 0, 0, 0);
	DEF_COLOR(Red, 1, 0, 0);
	DEF_COLOR(Green, 0, 1, 0);
	DEF_COLOR(Blue, 0, 0, 1);
	DEF_COLOR(Pink, 1.f, 0.f, 1.f);
	DEF_COLOR(Cyan, 0, 1, 1);
	DEF_COLOR(Yellow, 1, 1, 0);
	const Color Color::Transparent = Color(0, 0, 0, 0);
}
