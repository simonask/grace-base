//
//  type_info_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 28/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/type_info.hpp"

using namespace grace;

SUITE(TypeInfo) {
	it("should have construct == nullptr for non-trivially-constructible types", []() {
		struct NonTriviallyConstructible {
			NonTriviallyConstructible(int) {}
		};
		auto& ti = GetTypeInfo<NonTriviallyConstructible>::Value;
		TEST(ti.is_constructible()).should == false;
	});
	
	it("should have copy_construct == nullptr for non-copy-constructible types", []() {
		struct NonCopyConstructible {
			NonCopyConstructible(const NonCopyConstructible&) = delete;
		};
		auto& ti = GetTypeInfo<NonCopyConstructible>::Value;
		TEST(ti.is_copy_constructible()).should == false;
	});
	
	it("should have move_construct == nullptr for non-move-constructible types", []() {
		struct NonMoveConstructible {
			NonMoveConstructible(NonMoveConstructible&&) = delete;
		};
		auto& ti = GetTypeInfo<NonMoveConstructible>::Value;
		TEST(ti.is_move_constructible()).should == false;
	});
	
	it("should have copy_assign == nullptr for non-copy-assignable types", []() {
		struct NonCopyAssignable {
			NonCopyAssignable& operator=(const NonCopyAssignable&) = delete;
		};
		auto& ti = GetTypeInfo<NonCopyAssignable>::Value;
		TEST(ti.is_copy_assignable()).should == false;
	});
	
	it("should have move_assign == nullptr for non-move-assignable types", []() {
		struct NonMoveAssignable {
			NonMoveAssignable& operator=(NonMoveAssignable&&) = delete;
		};
		auto& ti = GetTypeInfo<NonMoveAssignable>::Value;
		TEST(ti.is_move_or_copy_assignable()).should == false;
	});
}