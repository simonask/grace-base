//
//  vector_test.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 31/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_vector_test_hpp
#define grace_vector_test_hpp

#include "tests/test.hpp"
#include "geometry/vector.hpp"
#include "geometry/vector_errors.hpp"
#include <cmath>

using namespace grace;

SUITE(TVector) {
	it("should instantiate with both integers and floats of all sizes up to 4 components", []() {
		TVector<uint32,  1> u32vec1;
		TVector<uint64,  1> u64vec1;
		TVector<int32,   1> i32vec1;
		TVector<int64,   1> i64vec1;
		TVector<float32, 1> f32vec1;
		TVector<float64, 1> f64vec1;
		
		TVector<uint32,  2> u32vec2;
		TVector<uint64,  2> u64vec2;
		TVector<int32,   2> i32vec2;
		TVector<int64,   2> i64vec2;
		TVector<float32, 2> f32vec2;
		TVector<float64, 2> f64vec2;
		
		TVector<uint32,  3> u32vec3;
		TVector<int32,   3> i32vec3;
		TVector<float32, 3> f32vec3;
		
		TVector<uint32,  4> u32vec4;
		TVector<int32,   4> i32vec4;
		TVector<float32, 4> f32vec4;
	});
	
	it("should add float vectors correctly", []() {
		auto a = vec4(1,2,3,4);
		auto b = vec4(2,3,4,5);
		auto c = a + b;
		auto expected = vec4(3,5,7,9);
		TEST(c).should == approximately(expected, 0.001f);
	});
	
	it("should calculate all_equals_within correctly", []() {
		auto a = vec4(1, 1, 1, 1.4);
		auto b = vec4(1, 1, 1, 2);
		TEST(a).should != approximately(b, 0.1f);
		TEST(a).should == approximately(b, 1.f);
		a = vec4(-1, -1, -1, -1);
		b = vec4(1, 1, 1, 1);
		TEST(a).should != approximately(b, 1.f);
	});
	
	it("should correctly compare negative and positive vectors", []() {
		auto a = vec4(-1, -1, -1, -1);
		auto b = vec4(1, 1, 1, 1);
		TEST(a).should != approximately(b, 1.f);
		TEST(b).should != approximately(a, 1.f);
	});
	
	it("should correctly compare negative vectors", []() {
		auto a = vec4(-1, -1, -1, -1);
		auto b = vec4(-3, -3, -3, -3);
		TEST(a).should == approximately(b, 2.5f);
		TEST(a).should != approximately(b, 1.f);
	});
	
	it("should calculate any_equals_within correctly", []() {
		auto a = vec4(1, 1, 1, 1.7);
		auto b = vec4(2, 2, 2, 2);
		TEST(a.any_equal_within(b, .1f)).should == false;
		TEST(a.any_equal_within(b, 0.4f)).should == true;
	});
	
	it("should calculate the length of the vector4", []() {
		auto a = vec4(1,2,3,4);
		float32 expected4 = 5.47723f; // according to wolfram alpha
		TEST(std::abs(a.length() - expected4)).should < 0.0001f;
	});
	
	it("should calculate the length of the vector3", []() {
		auto b = vec3(1,2,3);
		float32 expected3 = 3.74166f; // according to wolfram alpha
		TEST(std::abs(b.length() - expected3)).should < 0.0001f;
	});
	
	it("should calculate the length of the vector2", []() {
		auto b = vec2(3, 4);
		float32 expected = 5.f;
		TEST(std::abs(b.length() - expected)).should < 0.0001f;
	});
	
	it("should normalize to a vector of length == 1", []() {
		auto a = vec4(1, 2, 3, 4);
		auto b = a.normalize();
		TEST(b.length() - 1.f).should <= 0.0001f;
	});
	
	it("should create a vector of NaN when dividing zero by zero", [](){
		auto z = vec4::zero();
		auto o = z / z;
		should_return<bool>(true, [&]() {
			return o.all_is_nan();
		});
	});
	
	it("should create a vector of Infinity when dividing one by zero", []() {
		auto z = vec4::zero();
		auto o = vec4::one() / z;
		should_return<bool>(true, [&]() {
			return o.all_is_infinity();
		});
	});
	
	it("should throw an exception when safely normalizing a zero-vector", []() {
		auto z = vec4::zero();
		vec4 result = z / z;
		should_throw_exception<VectorNormalizeZeroLengthException>([&]() {
			result = z.normalize_safe();
		});
		should_return<bool>(true, [&](){
			return result.all_is_nan();
		});
	});
	
	it("should not throw an exception when safely normalizing a non-zero-vector", []() {
		auto o = vec4::one();
		vec4 result;
		should_not_throw_exception<VectorNormalizeZeroLengthException>([&]() {
			result = o.normalize_safe();
		});
		TEST(result).should == approximately(o.normalize(), 0.0001f);
	});
	
	it("should compare vectors with relative error tolerance", []() {
		auto a = vec4(1,2,3,4);
		auto b = vec4(1,2,3,4);
		TEST(a).should == approximately(b, 0.0001f);
	});
	
	it("should cross vectors", []() {
		vec3 a = vec3(1,2,3);
		vec3 b = vec3(2,3,4);
		vec3 c = a.cross(b);
		TEST(c).should == approximately(vec3{-1, 2, -1}, 0.0001f);
		c = b.cross(a);
		TEST(c).should == approximately(vec3{1, -2, 1}, 0.0001f);
	});
	
	it("should select components of vectors using masks (4-vector)", []() {
		// Replace components that are > 2 with 10.
		vec4 v = vec4(1, 2, 3, 4);
		vec4 r = vec4::replicate(10);
		uvec4 cmp = v > vec4::two();
		vec4 result = select(cmp, r, v);
		TEST(result).should == approximately(vec4{1, 2, 10, 10}, 0.0001f);
		TEST(result).should == approximately(select(cmp, r, v), 0.0001f);
	});
	
	it("should select components of vectors using masks (2-vector)", []() {
		// Replace components that are > 2 with 10.
		vec2 v = vec2(2, 3);
		vec2 r = vec2::replicate(10);
		uvec2 cmp = v > vec2::two();
		vec2 result = select(cmp, r, v);
		TEST(result).should == approximately(vec2(2, 10), 0.0001f);
		TEST(result).should == approximately(select(cmp, r, v), 0.0001f);
	});
	
	it("should shuffle a single vector (2-vector)", []() {
		vec2 v = vec2(2,3);
		vec2 r = shuffle<Y, X>(v);
		TEST(r).should == approximately(vec2(3,2), 0.0001f);
	});
	
	it("should shuffle a single vector (4-vector)", []() {
		vec4 v = vec4(1,2,3,4);
		vec4 r = shuffle<W, Z, Y, X>(v);
		TEST(r).should == approximately(vec4(4, 3, 2, 1), 0.0001f);
	});
	
	it("should shuffle two vectors (2-vector)", []() {
		vec2 a = vec2(1,2);
		vec2 b = vec2(3,4);
		vec2 r = shuffle2<0, X, 1, Y>(a, b);
		TEST(r).should == approximately(vec2(1, 4), 0.0001f);
	});
	
	it("should shuffle two vectors (4-vectors)", []() {
		vec4 a = vec4(1,   2,  3,  4);
		vec4 b = vec4(-1, -2, -3, -4);

		vec4 r1 = shuffle2<0, X, 0, Y, 0, Z, 0, W>(a, b);
		TEST(r1).should == approximately(vec4(1, 2, 3, 4), 0.0001f);

		vec4 r2 = shuffle2<0, X, 0, Y, 0, Z, 1, W>(a, b);
		TEST(r2).should == approximately(vec4(1, 2, 3, -4), 0.0001f);

		vec4 r3 = shuffle2<0, X, 0, Y, 1, Z, 0, W>(a, b);
		TEST(r3).should == approximately(vec4(1, 2, -3, 4), 0.0001f);

		vec4 r4 = shuffle2<0, X, 0, Y, 1, Z, 1, W>(a, b);
		TEST(r4).should == approximately(vec4(1, 2, -3, -4), 0.0001f);

		vec4 r5 = shuffle2<0, X, 1, Y, 0, Z, 0, W>(a, b);
		TEST(r5).should == approximately(vec4(1, -2, 3, 4), 0.0001f);

		vec4 r6 = shuffle2<0, X, 1, Y, 0, Z, 1, W>(a, b);
		TEST(r6).should == approximately(vec4(1, -2, 3, -4), 0.0001f);

		vec4 r7 = shuffle2<0, X, 1, Y, 1, Z, 0, W>(a, b);
		TEST(r7).should == approximately(vec4(1, -2, -3, 4), 0.0001f);

		vec4 r8 = shuffle2<0, X, 1, Y, 1, Z, 1, W>(a, b);
		TEST(r8).should == approximately(vec4(1, -2, -3, -4), 0.0001f);

		vec4 r9 = shuffle2<1, X, 0, Y, 0, Z, 0, W>(a, b);
		TEST(r9).should == approximately(vec4(-1, 2, 3, 4), 0.0001f);

		vec4 r10 = shuffle2<1, X, 0, Y, 0, Z, 1, W>(a, b);
		TEST(r10).should == approximately(vec4(-1, 2, 3, -4), 0.0001f);

		vec4 r11 = shuffle2<1, X, 0, Y, 1, Z, 0, W>(a, b);
		TEST(r11).should == approximately(vec4(-1, 2, -3, 4), 0.0001f);
		
		vec4 r12 = shuffle2<1, X, 0, Y, 1, Z, 1, W>(a, b);
		TEST(r12).should == approximately(vec4(-1, 2, -3, -4), 0.0001f);
		
		vec4 r13 = shuffle2<1, X, 1, Y, 0, Z, 0, W>(a, b);
		TEST(r13).should == approximately(vec4(-1, -2, 3, 4), 0.0001f);

		vec4 r14 = shuffle2<1, X, 1, Y, 0, Z, 1, W>(a, b);
		TEST(r14).should == approximately(vec4(-1, -2, 3, -4), 0.0001f);

		vec4 r15 = shuffle2<1, X, 1, Y, 1, Z, 0, W>(a, b);
		TEST(r15).should == approximately(vec4(-1, -2, -3, 4), 0.0001f);

		vec4 r16 = shuffle2<1, X, 1, Y, 1, Z, 1, W>(a, b);
		TEST(r16).should == approximately(vec4(-1, -2, -3, -4), 0.0001f);
	});
}

#endif
