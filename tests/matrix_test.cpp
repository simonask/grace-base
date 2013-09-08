//
//  matrix_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "geometry/matrix.hpp"
#include "geometry/2d.hpp"

using namespace grace;

SUITE(Matrix) {
	it("should correctly transform 2-vectors with the identity 2x2-matrix", []() {
		matrix22 identity = matrix22::identity();
		vec2 v = vec2(5, 6);
		vec2 r = matrix_transform(identity, v);
		TEST(r).should == approximately(v, 0.0001f);
	});
	
	it("should correctly transform 3-vectors with the identity 3x3-matrix", []() {
		matrix33 identity = matrix33::identity();
		vec3 v = vec3(4, 5, 6);
		vec3 r = matrix_transform(identity, v);
		TEST(r).should == approximately(v, 0.0001f);
	});
	
	it("should correctly transform 4-vectors with the identity 4x4-matrix", []() {
		matrix44 identity = matrix44::identity();
		vec4 v = vec4(4, 5, 6, 7);
		vec4 r = matrix_transform(identity, v);
		TEST(r).should == approximately(v, 0.0001f);
	});
	
	it("should correctly transform 2-vectors with a rotation 2x2-matrix", []() {
		// Rotate 90° to the right.
		matrix22 rot = make_rotation_matrix22(-PiOver2);
		vec2 v = vec2(1, 0);
		vec2 r = matrix_transform(rot, v);
		TEST(r).should == approximately(vec2{0, 1}, 0.0001f);
	});
	
	it("should correctly transform 2-vectors with the identity 3x3-matrix", []() {
		// Rotate 90° to the right.
		matrix33 identity = matrix33::identity();
		vec2 v = vec2(1, 0);
		vec2 r = matrix_transform(identity, v);
		TEST(r).should == approximately(v, 0.0001f);
	});
	
	it("should correctly transform 3-vectors with the identity 4x4-matrix", []() {
		matrix44 identity = matrix44::identity();
		vec3 v = vec3(1, 2, 3);
		vec3 r = matrix_transform(identity, v);
		TEST(r).should == approximately(v, 0.0001f);
	});
	
	it("should correctly transform 2-vectors with a rotation 3x3-matrix", []() {
		// Rotate 90° to the right.
		matrix33 rot = make_rotation_matrix33_z(Radians(-PiOver2));
		vec2 v = vec2(1, 0);
		vec2 r = matrix_transform(rot, v);
		TEST(r).should == approximately(vec2{0, 1}, 0.0001f);
	});
	
	it("should correctly transform 2-vectors with a translation 3x3-matrix", []() {
		matrix33 trans = make_2d_translation_matrix33(vec2(1, 1));
		vec2 v = vec2(2, 2);
		vec2 r = matrix_transform(trans, v);
		TEST(r).should == approximately(vec2{3, 3}, 0.0001f);
	});
	
	it("should multiply the 3x3-identity with itself and give identity", []() {
		matrix33 id1 = matrix33::identity();
		matrix33 id2 = matrix33::identity();
		TEST(id1).should == id2;
		matrix33 r = id1 * id2;
		TEST(r).should == matrix33::identity();
	});
	
	it("should correctly transform 2-vectors with a translation+rotation matrix", []() {
		matrix33 translate = make_2d_translation_matrix33(vec2(1,1));
		matrix33 rotate = make_rotation_matrix33_z(Radians(-PiOver2)); // Rotate 90° to the right
		matrix33 transform = translate * rotate; // first translate, then rotate
		vec2 v = vec2(2, 3);
		float32 l = v.length();
		vec2 r = matrix_transform(transform, v);
		float32 l2 = r.length();
		TEST(r).should == approximately(vec2{-2,3}, 0.0001f);
	});
}
