//
//  math_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 31/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"

#include "base/math.hpp"

using namespace grace;

SUITE(Math) {
	it("should approximate sin(1) within acceptable range", []() {
		vec4 theta = vec4::replicate(1.f);
		vec4 r = sin(theta);
		vec4 correct = vec4::replicate(0.8414709848078965066525023216302989996225630607983710);
		TEST(r).should == approximately(correct, 0.001f);
	});

	it("should approximate sin(2) within acceptable range", []() {
		vec4 theta = vec4::replicate(2.f);
		vec4 r = sin(theta);
		vec4 correct = vec4::replicate(0.9092974268256816953960198659117448427022549714478902);
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should approximate sin(π) within acceptable range", []() {
		vec4 theta = vec4::replicate(Pi);
		vec4 r = sin(theta);
		vec4 correct = vec4::zero();
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should approximate sin(π/2) within acceptable range", []() {
		vec4 theta = vec4::replicate(PiOver2);
		vec4 r = sin(theta);
		vec4 correct = vec4::one();
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should approximate sin(2π) within acceptable range", []() {
		vec4 theta = vec4::replicate(Pi*2);
		vec4 r = sin(theta);
		vec4 correct = vec4::zero();
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should calculate all of sin([π, 2π, π/2, 0]) correctly in parallel", []() {
		vec4 theta = vec4(Pi, Pi*2, PiOver2, 0);
		vec4 r = sin(theta);
		vec4 correct = vec4(0.f, 0.f, 1.f, 0.f);
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should approximate cos(1) within acceptable range", []() {
		vec4 theta = vec4::replicate(1.f);
		vec4 r = cos(theta);
		vec4 correct = vec4::replicate(0.5403023058681397174009366074429766037323104206179222);
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should approximate cos(2) within acceptable range", []() {
		vec4 theta = vec4::replicate(2.f);
		vec4 r = cos(theta);
		vec4 correct = vec4::replicate(-0.41614683654714238699756822950076218976600077107554);
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should approximate cos(π) within acceptable range", []() {
		vec4 theta = vec4::replicate(Pi);
		vec4 r = cos(theta);
		vec4 correct = -vec4::one();
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should approximate cos(π/2) within acceptable range", []() {
		vec4 theta = vec4::replicate(PiOver2);
		vec4 r = cos(theta);
		vec4 correct = vec4::zero();
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should approximate cos(2π) within acceptable range", []() {
		vec4 theta = vec4::replicate(Pi*2);
		vec4 r = cos(theta);
		vec4 correct = vec4::one();
		TEST(r).should == approximately(correct, 0.001f);
	});
	
	it("should calculate all of cos([π, 2π, π/2, 0]) correctly in parallel", []() {
		vec4 theta = vec4(Pi, Pi*2, PiOver2, 0);
		vec4 r = cos(theta);
		vec4 correct = vec4(-1.f, 1.f, 0.f, 1.f);
		TEST(r).should == approximately(correct, 0.001f);
	});
}
