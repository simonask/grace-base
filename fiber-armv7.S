//
//  fiber-armv7.s
//  falling
//
//  Created by Simon Ask Ulsnes on 23/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//
#if defined(__arm__)
.arm
.text
.global falling_launch_fiber
falling_launch_fiber:
	mov r4, sp
	mov sp, a2
	blx a3
	mov sp, r4
	bx lr
#endif