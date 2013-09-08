//
//  geometry_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 20/05/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "geometry/2d.hpp"

using namespace grace;

SUITE(Geometry) {
	it("should calculate bounds of rotated square", [] {
		Rect r = {0, 0, 10, 10};
		Rect rotated_bounds = bounds_of_rotated_rectangle(r, 45_deg);
		TEST(r.area()).should == 100.f;
		float32 expected_width = r.size.width * 2.f / Sqrt2;
		TEST(rotated_bounds.size.width).should == approximately(expected_width, 0.001f);
		TEST(rotated_bounds.size.height).should == rotated_bounds.size.width;
	});
	
	feature("find line segment intersection", [] {
		vec2 p0 = {0, 0};
		vec2 p1 = {10, 10};
		vec2 q0 = {0, 10};
		vec2 q1 = {10, 0};
		auto m = line_intersection(p0, p1, q0, q1);
		TEST(m.is_set()).should == true;
		TEST(*m).should == approximately(vec2{5, 5}, 0.1f);
	});
	
	feature("find no intersection between parallel line segments", [] {
		vec2 p0 = {0, 0};
		vec2 p1 = {5, 5};
		vec2 q0 = {1, 1};
		vec2 q1 = {6, 6};
		auto m = line_intersection(p0, p1, q0, q1);
		TEST(m.is_set()).should == false;
	});
	
	feature("find no intersection between non-overlapping line segments", []() {
		vec2 p0 = {0, 0};
		vec2 p1 = {10, 10};
		vec2 q0 = {0, 10};
		vec2 q1 = {10, 11};
		auto m = line_intersection(p0, p1, q0, q1);
		TEST(m.is_set()).should == false;
	});
}
