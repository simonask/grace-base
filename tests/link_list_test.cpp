//
//  link_list_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 25/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"

using namespace grace;

struct InPlaceListElement : LinkListItem<InPlaceListElement> {
	int n;
	InPlaceListElement() : n(0) {}
	InPlaceListElement(int n) : n(n) {}
};

SUITE(LinkList) {
	it("should add elements without deriving from LinkListItem", []() {
		ScratchAllocator scratch;
		LinkList<int> list(scratch);
		for (int i: {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
			list.push_back(i);
		}
		int x = 1;
		for (int& n: list) {
			TEST(n).should == x;
			x++;
		}
	});
	
	it("should add elements deriving from LinkListItem", []() {
		ScratchAllocator scratch;
		LinkList<InPlaceListElement> list(scratch);
		for (int i: {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
			list.push_back(i);
		}
	});
}