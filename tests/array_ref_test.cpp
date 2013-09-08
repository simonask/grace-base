//
//  array_ref_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 27/07/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "tests/test.hpp"
#include "base/array_ref.hpp"
#include "base/array.hpp"

SUITE(ArrayRef) {
	using namespace grace;
	
	feature("convert to array of const", []() {
		Array<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		ArrayRef<int> numbers_ref = numbers;
		ArrayRef<const int> numbers_cref = numbers_ref;
	});
	
	feature("convert to array of const pointers", []() {
		int n = 1;
		Array<int*> number_ptrs = {&n, &n, &n};
		ArrayRef<int*> number_ptrs_ref = number_ptrs;
		ArrayRef<const int*> number_ptrs_cref = number_ptrs_ref;
		ArrayRef<int const* const> number_cptrs_ref = number_ptrs_ref;
	});
	
	feature("convert to array of pointers to base class", []() {
		struct Foo {
			int n;
		};
		struct Bar : Foo {
			int m;
		};
		
		Bar bar;
		Array<Bar*> bars = {&bar, &bar, &bar};
		ArrayRef<Bar*> bars_ref = bars;
		ArrayRef<Foo*> foos_ref = bars_ref;
	});
}
