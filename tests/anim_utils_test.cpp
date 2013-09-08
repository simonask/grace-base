//
//  anim_utils_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/anim_utils.hpp"
#include "base/simd.hpp"
#include "tests/test.hpp"

using namespace grace;

SUITE(anim_utils) {
	it("should lerp float32 correctly", []() {
		float32 half = anim_lerp(0.f, 100.f, 0.5);
		TEST(half).should == 50.f;
		
		float32 start = 0.f;
		float32 target = 100.f;
		float32 current = start;
		for (size_t i = 0; i < 100; ++i) {
			float64 progress_ratio = 1.0 / (float64)(100 - i);
			current = anim_lerp(current, target, progress_ratio);
			float64 expected = (float64)(i + 1);
			float64 diff = expected - current;
			TEST(abs(diff)).should < 0.0001;
		}
		TEST(target).should == 100.f;
	});
	
	it("should lerp float64 correctly", []() {
		float64 half = anim_lerp(0.0, 100.0, 0.5);
		TEST(half).should == 50.0;
		
		float64 start = 0.0;
		float64 target = 100.0;
		float64 current = start;
		for (size_t i = 0; i < 100; ++i) {
			float64 progress_ratio = 1.0 / (float64)(100 - i);
			current = anim_lerp(current, target, progress_ratio);
			float64 expected = (float64)(i + 1);
			float64 diff = expected - current;
			TEST(abs(diff)).should < 0.0001;
		}
		TEST(target).should == 100.0;
	});
	
	it("should lerp int32 correctly", []() {
		int32 start = 0;
		int32 target = 100;
		int32 current = start;
		float64 accum = 0;
		for (size_t i = 0; i < 50; ++i) {
			float64 progress_ratio = 1.0 / (float64)(50 - i);
			current = anim_lerp(current, target, progress_ratio, accum);
		}
		TEST(current).should == target;
	});
	
	it("should lerp int32 correctly with increments less than 1", []() {
		int32 start = 0;
		int32 target = 100;
		int32 current = start;
		float64 accum = 0;
		for (size_t i = 0; i < 200; ++i) {
			float64 progress_ratio = 1.0 / (float64)(200 - i);
			current = anim_lerp(current, target, progress_ratio, accum);
		}
		TEST(current).should == target;
	});
	
	it("should lerp int64 correctly", []() {
		int64 start = 0;
		int64 target = 100;
		int64 current = start;
		float64 accum = 0;
		for (size_t i = 0; i < 50; ++i) {
			float64 progress_ratio = 1.0 / (float64)(50 - i);
			current = anim_lerp(current, target, progress_ratio, accum);
		}
		TEST(current).should == target;
	});
	
	it("should lerp int64 correctly with increments less than 1", []() {
		int64 start = 0;
		int64 target = 100;
		int64 current = start;
		float64 accum = 0;
		for (size_t i = 0; i < 200; ++i) {
			float64 progress_ratio = 1.0 / (float64)(200 - i);
			current = anim_lerp(current, target, progress_ratio, accum);
		}
		TEST(current).should == target;
	});
	
	it("should lerp vec2 correctly", []() {
		vec2 start = vec2::zero();
		vec2 target = vec2(50.f, 200.f);
		vec2 current = start;
		for (size_t i = 0; i < 100; ++i) {
			float64 progress_ratio = 1.0 / (float64)(100 - i);
			current = anim_lerp(current, target, progress_ratio);
		}
		TEST(current).should == approximately(target, 0.0001f);
	});
	
	it("should lerp vec3 correctly", []() {
		vec3 start = vec3::zero();
		vec3 target = vec3(50.f, 200.f, 99.f);
		vec3 current = start;
		for (size_t i = 0; i < 100; ++i) {
			float64 progress_ratio = 1.0 / (float64)(100 - i);
			current = anim_lerp(current, target, progress_ratio);
		}
		TEST(current).should == approximately(target, 0.0001f);
	});
	
	it("should lerp vec4 correctly", []() {
		vec4 start = vec4::zero();
		vec4 target = vec4(50.f, 200.f, 99.f, 1234.f);
		vec4 current = start;
		for (size_t i = 0; i < 100; ++i) {
			float64 progress_ratio = 1.0 / (float64)(100 - i);
			current = anim_lerp(current, target, progress_ratio);
		}
		TEST(current).should == approximately(target, 0.0001f);
	});
	
	it("should lerp matrix22 correctly");
	it("should lerp matrix33 correctly");
	it("should lerp matrix43 correctly");
	it("should lerp matrix44 correctly");
}
