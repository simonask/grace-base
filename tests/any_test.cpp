//
//  any_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 27/02/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/any.hpp"
#include "io/resource.hpp"
#include "io/resource_ptr.hpp"
#include "io/resource_ptr_type.hpp"

using namespace grace;

struct MyResource : public Resource {
	
};

SUITE(Any) {
#pragma mark Regressions
	it("should destruct ResourcePtrs properly", []() {
		Any any;
		MyResource* r = new MyResource();
		ResourcePtr<MyResource> p(r);
		TEST(r->refcount()).should == 1;
		any = p;
		TEST(r->refcount()).should == 2;
		Any any2(move(p));
		TEST(p.get()).should == nullptr;
		TEST(r->refcount()).should == 2;
		any2.clear();
		TEST(r->refcount()).should == 1;
		any.clear();
		TEST(r->refcount()).should == 0;
		destroy(r, default_allocator());
	});
}
