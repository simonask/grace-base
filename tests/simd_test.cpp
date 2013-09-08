//
//  simd_test.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 07/06/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "tests/test.hpp"
#include "base/simd.hpp"

// Generating the code to test shuffles takes a very long time
// and consumes a *lot* of memory, so let's disable these tests
// by default to maintain a usable workflow.
#define TEST_SHUFFLES 0

using namespace grace;
using namespace grace::simd;

template <Axis X_, Axis Y_, Axis Z_, Axis W_>
struct TestShuffleVec4_Impl {
	template <typename V>
	static void test(V v) {
		auto r = shuffle<X_,Y_,Z_,W_>(v);
		TEST(get<X>(r)).should == get<X_>(v);
		TEST(get<Y>(r)).should == get<Y_>(v);
		TEST(get<Z>(r)).should == get<Z_>(v);
		TEST(get<W>(r)).should == get<W_>(v);
	}
};

template <Axis X_, Axis Y_, Axis Z_>
struct TestShuffleVec4_AllW {
	template <typename V>
	static void test(V v) {
		TestShuffleVec4_Impl<X_, Y_, Z_, X>::test(v);
		TestShuffleVec4_Impl<X_, Y_, Z_, Y>::test(v);
		TestShuffleVec4_Impl<X_, Y_, Z_, Z>::test(v);
		TestShuffleVec4_Impl<X_, Y_, Z_, W>::test(v);
	}
};

template <Axis X_, Axis Y_>
struct TestShuffleVec4_AllZ {
	template <typename V>
	static void test(V v) {
		TestShuffleVec4_AllW<X_, Y_, X>::test(v);
		TestShuffleVec4_AllW<X_, Y_, Y>::test(v);
		TestShuffleVec4_AllW<X_, Y_, Z>::test(v);
		TestShuffleVec4_AllW<X_, Y_, W>::test(v);
	}
};

template <Axis X_>
struct TestShuffleVec4_AllY {
	template <typename V>
	static void test(V v) {
		TestShuffleVec4_AllZ<X_, X>::test(v);
		TestShuffleVec4_AllZ<X_, Y>::test(v);
		TestShuffleVec4_AllZ<X_, Z>::test(v);
		TestShuffleVec4_AllZ<X_, W>::test(v);
	}
};

struct TestShuffleVec4 {
	template <typename V>
	static void test(V v) {
		TestShuffleVec4_AllY<X>::test(v);
		TestShuffleVec4_AllY<Y>::test(v);
		TestShuffleVec4_AllY<Z>::test(v);
		TestShuffleVec4_AllY<W>::test(v);
	}
};

template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, Axis Z_, size_t WVector, Axis W_>
struct TestShuffle2Vec4_Impl {
	template <typename V>
	static void test(V a, V b) {
		V v[] = {a, b};
		auto r = shuffle2<XVector,X_,YVector,Y_,ZVector,Z_,WVector,W_>(a,b);
		TEST(get<X>(r)).should == get<X_>(v[XVector]);
		TEST(get<Y>(r)).should == get<Y_>(v[YVector]);
		TEST(get<Z>(r)).should == get<Z_>(v[ZVector]);
		TEST(get<W>(r)).should == get<W_>(v[WVector]);
	}
};

template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, Axis Z_, size_t WVector>
struct TestShuffle2Vec4_AllW {
	template <typename V>
	static void test(V a, V b) {
		TestShuffle2Vec4_Impl<XVector,X_,YVector,Y_,ZVector,Z_,WVector,X>::test(a,b);
		TestShuffle2Vec4_Impl<XVector,X_,YVector,Y_,ZVector,Z_,WVector,Y>::test(a,b);
		TestShuffle2Vec4_Impl<XVector,X_,YVector,Y_,ZVector,Z_,WVector,Z>::test(a,b);
		TestShuffle2Vec4_Impl<XVector,X_,YVector,Y_,ZVector,Z_,WVector,W>::test(a,b);
	}
};

template <size_t XVector, Axis X_, size_t YVector, Axis Y_, size_t ZVector, size_t WVector>
struct TestShuffle2Vec4_AllZ {
	template <typename V>
	static void test(V a, V b) {
		TestShuffle2Vec4_AllW<XVector,X_,YVector,Y_,ZVector,X,WVector>::test(a,b);
		TestShuffle2Vec4_AllW<XVector,X_,YVector,Y_,ZVector,Y,WVector>::test(a,b);
		TestShuffle2Vec4_AllW<XVector,X_,YVector,Y_,ZVector,Z,WVector>::test(a,b);
		TestShuffle2Vec4_AllW<XVector,X_,YVector,Y_,ZVector,W,WVector>::test(a,b);
	}
};

template <size_t XVector, Axis X_, size_t YVector, size_t ZVector, size_t WVector>
struct TestShuffle2Vec4_AllY {
	template <typename V>
	static void test(V a, V b) {
		TestShuffle2Vec4_AllZ<XVector,X_,YVector,X,ZVector,WVector>::test(a,b);
		TestShuffle2Vec4_AllZ<XVector,X_,YVector,Y,ZVector,WVector>::test(a,b);
		TestShuffle2Vec4_AllZ<XVector,X_,YVector,Z,ZVector,WVector>::test(a,b);
		TestShuffle2Vec4_AllZ<XVector,X_,YVector,W,ZVector,WVector>::test(a,b);
	}
};

template <size_t XVector, size_t YVector, size_t ZVector, size_t WVector>
struct TestShuffle2Vec4_AllX {
	template <typename V>
	static void test(V a, V b) {
		TestShuffle2Vec4_AllY<XVector,X,YVector,ZVector,WVector>::test(a,b);
		TestShuffle2Vec4_AllY<XVector,Y,YVector,ZVector,WVector>::test(a,b);
		TestShuffle2Vec4_AllY<XVector,Z,YVector,ZVector,WVector>::test(a,b);
		TestShuffle2Vec4_AllY<XVector,W,YVector,ZVector,WVector>::test(a,b);
	}
};

template <size_t XVector, size_t YVector, size_t ZVector>
struct TestShuffle2Vec4_AllWVector {
	template <typename V>
	static void test(V a, V b) {
		TestShuffle2Vec4_AllX<XVector,YVector,ZVector,0>::test(a,b);
		TestShuffle2Vec4_AllX<XVector,YVector,ZVector,1>::test(a,b);
	}
};

template <size_t XVector, size_t YVector>
struct TestShuffle2Vec4_AllZVector {
	template <typename V>
	static void test(V a, V b) {
		TestShuffle2Vec4_AllWVector<XVector,YVector,0>::test(a,b);
		TestShuffle2Vec4_AllWVector<XVector,YVector,1>::test(a,b);
	}
};

template <size_t XVector>
struct TestShuffle2Vec4_AllYVector {
	template <typename V>
	static void test(V a, V b) {
		TestShuffle2Vec4_AllZVector<XVector,0>::test(a,b);
		TestShuffle2Vec4_AllZVector<XVector,1>::test(a,b);
	}
};

struct TestShuffle2Vec4 {
	template <typename V>
	static void test(V a, V b) {
		TestShuffle2Vec4_AllYVector<0>::test(a,b);
		TestShuffle2Vec4_AllYVector<1>::test(a,b);
	}
};

SUITE(SIMD) {
	using fvec2_t = typename GetVectorType<float32, 2>::Type;
	using ivec2_t = typename GetVectorType<int32,   2>::Type;
	using uvec2_t = typename GetVectorType<uint32,  2>::Type;
	using fvec4_t = typename GetVectorType<float32, 4>::Type;
	using ivec4_t = typename GetVectorType<int32,   4>::Type;
	using uvec4_t = typename GetVectorType<uint32,  4>::Type;

	feature("unaligned_load", []() {
		float32 srcf[] = {0.f, 1.f, 2.f, 3.f, 4.f};
		int32 srci[] = {0, 1, 2, 3, 4};
		uint32 srcu[] = {0, 1, 2, 3, 4};
	
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;

		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		
		unaligned_load(vf2, srcf+1);
		unaligned_load(vi2, srci+1);
		unaligned_load(vu2, srcu+1);
		unaligned_load(vf4, srcf+1);
		unaligned_load(vi4, srci+1);
		unaligned_load(vu4, srcu+1);

		TEST(get<X>(vf2)).should == 1;
		TEST(get<Y>(vf2)).should == 2;

		TEST(get<X>(vi2)).should == 1;
		TEST(get<Y>(vi2)).should == 2;

		TEST(get<X>(vu2)).should == 1;
		TEST(get<Y>(vu2)).should == 2;
	
		TEST(get<X>(vf4)).should == 1;
		TEST(get<Y>(vf4)).should == 2;
		TEST(get<Z>(vf4)).should == 3;
		TEST(get<W>(vf4)).should == 4;

		TEST(get<X>(vi4)).should == 1;
		TEST(get<Y>(vi4)).should == 2;
		TEST(get<Z>(vi4)).should == 3;
		TEST(get<W>(vi4)).should == 4;

		TEST(get<X>(vu4)).should == 1;
		TEST(get<Y>(vu4)).should == 2;
		TEST(get<Z>(vu4)).should == 3;
		TEST(get<W>(vu4)).should == 4;
	});
	
	feature("aligned_load", []() {
		float32 ALIGNED(16) srcf[] = {1.f, 2.f, 3.f, 4.f};
		int32 ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32 ALIGNED(16) srcu[] = {1, 2, 3, 4};
	
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;

		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		
		aligned_load(vf2, srcf);
		aligned_load(vi2, srci);
		aligned_load(vu2, srcu);
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);
		aligned_load(vu4, srcu);

		TEST(get<X>(vf2)).should == 1;
		TEST(get<Y>(vf2)).should == 2;

		TEST(get<X>(vi2)).should == 1;
		TEST(get<Y>(vi2)).should == 2;

		TEST(get<X>(vu2)).should == 1;
		TEST(get<Y>(vu2)).should == 2;
	
		TEST(get<X>(vf4)).should == 1;
		TEST(get<Y>(vf4)).should == 2;
		TEST(get<Z>(vf4)).should == 3;
		TEST(get<W>(vf4)).should == 4;

		TEST(get<X>(vi4)).should == 1;
		TEST(get<Y>(vi4)).should == 2;
		TEST(get<Z>(vi4)).should == 3;
		TEST(get<W>(vi4)).should == 4;

		TEST(get<X>(vu4)).should == 1;
		TEST(get<Y>(vu4)).should == 2;
		TEST(get<Z>(vu4)).should == 3;
		TEST(get<W>(vu4)).should == 4;
	});

	feature("set", []() {
		fvec2_t vf2 = simd::set(1.f, 2.f);
		ivec2_t vi2 = simd::set((int32)1, (int32)2);
		uvec2_t vu2 = simd::set((uint32)1, (uint32)2);

		fvec4_t vf4 = simd::set(1.f, 2.f, 3.f, 4.f);
		ivec4_t vi4 = simd::set((int32)1, (int32)2, (int32)3, (int32)4);
		uvec4_t vu4 = simd::set((uint32)1, (uint32)2, (uint32)3, (uint32)4);

		TEST(get<X>(vf2)).should == 1;
		TEST(get<Y>(vf2)).should == 2;

		TEST(get<X>(vi2)).should == 1;
		TEST(get<Y>(vi2)).should == 2;

		TEST(get<X>(vu2)).should == 1;
		TEST(get<Y>(vu2)).should == 2;
	
		TEST(get<X>(vf4)).should == 1;
		TEST(get<Y>(vf4)).should == 2;
		TEST(get<Z>(vf4)).should == 3;
		TEST(get<W>(vf4)).should == 4;

		TEST(get<X>(vi4)).should == 1;
		TEST(get<Y>(vi4)).should == 2;
		TEST(get<Z>(vi4)).should == 3;
		TEST(get<W>(vi4)).should == 4;

		TEST(get<X>(vu4)).should == 1;
		TEST(get<Y>(vu4)).should == 2;
		TEST(get<Z>(vu4)).should == 3;
		TEST(get<W>(vu4)).should == 4;
	});

	feature("replicate", []() {
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;

		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		
		replicate(vf2, 10);
		replicate(vi2, 10);
		replicate(vu2, 10);
		replicate(vf4, 10);
		replicate(vi4, 10);
		replicate(vu4, 10);

		TEST(get<X>(vf2)).should == 10;
		TEST(get<Y>(vf2)).should == 10;

		TEST(get<X>(vi2)).should == 10;
		TEST(get<Y>(vi2)).should == 10;

		TEST(get<X>(vu2)).should == 10;
		TEST(get<Y>(vu2)).should == 10;
	
		TEST(get<X>(vf4)).should == 10;
		TEST(get<Y>(vf4)).should == 10;
		TEST(get<Z>(vf4)).should == 10;
		TEST(get<W>(vf4)).should == 10;

		TEST(get<X>(vi4)).should == 10;
		TEST(get<Y>(vi4)).should == 10;
		TEST(get<Z>(vi4)).should == 10;
		TEST(get<W>(vi4)).should == 10;

		TEST(get<X>(vu4)).should == 10;
		TEST(get<Y>(vu4)).should == 10;
		TEST(get<Z>(vu4)).should == 10;
		TEST(get<W>(vu4)).should == 10;
	});

	feature("convert (type)", []() {
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;

		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;

		vf2 = set(1.3f, 2.f);
		convert(vf2, vi2);
		TEST(get<X>(vi2)).should == 1;
		TEST(get<Y>(vi2)).should == 2;

		vf2 = set(-1.f, 2.5f);
		convert(vf2, vu2);
		TEST(get<X>(vu2)).should == UINT32_MAX;
		TEST(get<Y>(vu2)).should == 2;

		vi2 = set((int32)-4, (int32)90);
		convert(vi2, vf2);
		TEST(get<X>(vf2)).should == -4.f;
		TEST(get<Y>(vf2)).should == 90.f;

		vf4 = set(1.f, 2.5f, 3.f, 4.f);
		convert(vf4, vi4);
		TEST(get<X>(vi4)).should == 1;
		TEST(get<Y>(vi4)).should == 2;
		TEST(get<Z>(vi4)).should == 3;
		TEST(get<W>(vi4)).should == 4;

		vf4 = set(-1.f, 2.f, 3.2f, 4.f);
		convert(vf4, vu4);
		TEST(get<X>(vu4)).should == UINT32_MAX;
		TEST(get<Y>(vu4)).should == 2;
		TEST(get<Z>(vu4)).should == 3;
		TEST(get<W>(vu4)).should == 4;

		vi4 = set((int32)-4, (int32)90, (int32)5, (int32)6);
		convert(vi4, vf4);
		TEST(get<X>(vf4)).should == -4.f;
		TEST(get<Y>(vf4)).should == 90.f;
		TEST(get<Z>(vf4)).should == 5.f;
		TEST(get<W>(vf4)).should == 6.f;
	});

	//feature("convert (size)");

	feature("add", []() {
		float32 ALIGNED(16) srcf[] = {1.f, 2.f, 3.f, 4.f};
		int32 ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32 ALIGNED(16) srcu[] = {1, 2, 3, 4};
	
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;

		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		
		aligned_load(vf2, srcf);
		aligned_load(vi2, srci);
		aligned_load(vu2, srcu);
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);
		aligned_load(vu4, srcu);

		vf2 = add(vf2, vf2);
		vi2 = add(vi2, vi2);
		vu2 = add(vu2, vu2);
		vf4 = add(vf4, vf4);
		vi4 = add(vi4, vi4);
		vu4 = add(vu4, vu4);

		TEST(get<X>(vf2)).should == 2;
		TEST(get<Y>(vf2)).should == 4;

		TEST(get<X>(vi2)).should == 2;
		TEST(get<Y>(vi2)).should == 4;

		TEST(get<X>(vu2)).should == 2;
		TEST(get<Y>(vu2)).should == 4;
	
		TEST(get<X>(vf4)).should == 2;
		TEST(get<Y>(vf4)).should == 4;
		TEST(get<Z>(vf4)).should == 6;
		TEST(get<W>(vf4)).should == 8;

		TEST(get<X>(vi4)).should == 2;
		TEST(get<Y>(vi4)).should == 4;
		TEST(get<Z>(vi4)).should == 6;
		TEST(get<W>(vi4)).should == 8;

		TEST(get<X>(vu4)).should == 2;
		TEST(get<Y>(vu4)).should == 4;
		TEST(get<Z>(vu4)).should == 6;
		TEST(get<W>(vu4)).should == 8;
	});

	feature("sub", []() {
		float32 ALIGNED(16) srcf[] = {1.f, 2.f, 3.f, 4.f};
		int32 ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32 ALIGNED(16) srcu[] = {1, 2, 3, 4};
	
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;

		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		
		aligned_load(vf2, srcf);
		aligned_load(vi2, srci);
		aligned_load(vu2, srcu);
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);
		aligned_load(vu4, srcu);

		vf2 = sub(vf2, vf2);
		vi2 = sub(vi2, vi2);
		vu2 = sub(vu2, vu2);
		vf4 = sub(vf4, vf4);
		vi4 = sub(vi4, vi4);
		vu4 = sub(vu4, vu4);

		TEST(get<X>(vf2)).should == 0;
		TEST(get<Y>(vf2)).should == 0;

		TEST(get<X>(vi2)).should == 0;
		TEST(get<Y>(vi2)).should == 0;

		TEST(get<X>(vu2)).should == 0;
		TEST(get<Y>(vu2)).should == 0;
	
		TEST(get<X>(vf4)).should == 0;
		TEST(get<Y>(vf4)).should == 0;
		TEST(get<Z>(vf4)).should == 0;
		TEST(get<W>(vf4)).should == 0;

		TEST(get<X>(vi4)).should == 0;
		TEST(get<Y>(vi4)).should == 0;
		TEST(get<Z>(vi4)).should == 0;
		TEST(get<W>(vi4)).should == 0;

		TEST(get<X>(vu4)).should == 0;
		TEST(get<Y>(vu4)).should == 0;
		TEST(get<Z>(vu4)).should == 0;
		TEST(get<W>(vu4)).should == 0;
	});

	feature("mul", []() {
		float32 ALIGNED(16) srcf[] = {1.f, 2.f, 3.f, 4.f};
		int32 ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32 ALIGNED(16) srcu[] = {1, 2, 3, 4};
	
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;

		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		
		aligned_load(vf2, srcf);
		aligned_load(vi2, srci);
		aligned_load(vu2, srcu);
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);
		aligned_load(vu4, srcu);

		vf2 = mul(vf2, vf2);
		vi2 = mul(vi2, vi2);
		vu2 = mul(vu2, vu2);
		vf4 = mul(vf4, vf4);
		vi4 = mul(vi4, vi4);
		vu4 = mul(vu4, vu4);

		TEST(get<X>(vf2)).should == 1;
		TEST(get<Y>(vf2)).should == 4;

		TEST(get<X>(vi2)).should == 1;
		TEST(get<Y>(vi2)).should == 4;

		TEST(get<X>(vu2)).should == 1;
		TEST(get<Y>(vu2)).should == 4;
	
		TEST(get<X>(vf4)).should == 1;
		TEST(get<Y>(vf4)).should == 4;
		TEST(get<Z>(vf4)).should == 9;
		TEST(get<W>(vf4)).should == 16;

		TEST(get<X>(vi4)).should == 1;
		TEST(get<Y>(vi4)).should == 4;
		TEST(get<Z>(vi4)).should == 9;
		TEST(get<W>(vi4)).should == 16;

		TEST(get<X>(vu4)).should == 1;
		TEST(get<Y>(vu4)).should == 4;
		TEST(get<Z>(vu4)).should == 9;
		TEST(get<W>(vu4)).should == 16;
	});

	feature("div", []() {
		float32 ALIGNED(16) srcf[] = {1.f, 2.f, 3.f, 4.f};
		int32 ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32 ALIGNED(16) srcu[] = {1, 2, 3, 4};
	
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;

		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		
		aligned_load(vf2, srcf);
		aligned_load(vi2, srci);
		aligned_load(vu2, srcu);
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);
		aligned_load(vu4, srcu);

		vf2 = div(vf2, vf2);
		vi2 = div(vi2, vi2);
		vu2 = div(vu2, vu2);
		vf4 = div(vf4, vf4);
		vi4 = div(vi4, vi4);
		vu4 = div(vu4, vu4);

		TEST(get<X>(vf2)).should == 1;
		TEST(get<Y>(vf2)).should == 1;

		TEST(get<X>(vi2)).should == 1;
		TEST(get<Y>(vi2)).should == 1;

		TEST(get<X>(vu2)).should == 1;
		TEST(get<Y>(vu2)).should == 1;
	
		TEST(get<X>(vf4)).should == 1;
		TEST(get<Y>(vf4)).should == 1;
		TEST(get<Z>(vf4)).should == 1;
		TEST(get<W>(vf4)).should == 1;

		TEST(get<X>(vi4)).should == 1;
		TEST(get<Y>(vi4)).should == 1;
		TEST(get<Z>(vi4)).should == 1;
		TEST(get<W>(vi4)).should == 1;

		TEST(get<X>(vu4)).should == 1;
		TEST(get<Y>(vu4)).should == 1;
		TEST(get<Z>(vu4)).should == 1;
		TEST(get<W>(vu4)).should == 1;
	});

	feature("divrem", []() {
		int32 ALIGNED(16) srci_divisor[] = {1, 2, 3, 4};
		uint32 ALIGNED(16) srcu_divisor[] = {1, 2, 3, 4};
		int32 ALIGNED(16) srci_dividend[] = {3, 6, 9, 12};
		uint32 ALIGNED(16) srcu_dividend[] = {3, 6, 9, 12};
	
		ivec2_t vi2_a;
		ivec2_t vi2_b;
		uvec2_t vu2_a;
		uvec2_t vu2_b;
		ivec4_t vi4_a;
		ivec4_t vi4_b;
		uvec4_t vu4_a;
		uvec4_t vu4_b;
		
		aligned_load(vi2_a, srci_dividend);
		aligned_load(vi2_b, srci_divisor);
		aligned_load(vu2_a, srcu_dividend);
		aligned_load(vu2_b, srcu_divisor);
		aligned_load(vi4_a, srci_dividend);
		aligned_load(vi4_b, srci_divisor);
		aligned_load(vu4_a, srcu_dividend);
		aligned_load(vu4_b, srcu_divisor);

		ivec2_t ri2;
		ivec2_t di2 = divrem(vi2_a, vi2_b, &ri2);
		uvec2_t ru2;
		uvec2_t du2 = divrem(vu2_a, vu2_b, &ru2);
		ivec4_t ri4;
		ivec4_t di4 = divrem(vi4_a, vi4_b, &ri4);
		uvec4_t ru4;
		uvec4_t du4 = divrem(vu4_a, vu4_b, &ru4);

		TEST(get<X>(di2)).should == 3;
		TEST(get<Y>(di2)).should == 3;

		TEST(get<X>(du2)).should == 3;
		TEST(get<Y>(du2)).should == 3;
	
		TEST(get<X>(di4)).should == 3;
		TEST(get<Y>(di4)).should == 3;
		TEST(get<Z>(di4)).should == 3;
		TEST(get<W>(di4)).should == 3;

		TEST(get<X>(du4)).should == 3;
		TEST(get<Y>(du4)).should == 3;
		TEST(get<Z>(du4)).should == 3;
		TEST(get<W>(du4)).should == 3;

		TEST(get<X>(ri2)).should == 0;
		TEST(get<Y>(ri2)).should == 0;

		TEST(get<X>(ru2)).should == 0;
		TEST(get<Y>(ru2)).should == 0;
	
		TEST(get<X>(ri4)).should == 0;
		TEST(get<Y>(ri4)).should == 0;
		TEST(get<Z>(ri4)).should == 0;
		TEST(get<W>(ri4)).should == 0;

		TEST(get<X>(ru4)).should == 0;
		TEST(get<Y>(ru4)).should == 0;
		TEST(get<Z>(ru4)).should == 0;
		TEST(get<W>(ru4)).should == 0;
	});

	feature("bitwise_and", []() {
		uvec2_t u2_a;
		uvec2_t u2_b;
		ivec2_t i2_a;
		ivec2_t i2_b;
		uvec4_t u4_a;
		uvec4_t u4_b;
		ivec4_t i4_a;
		ivec4_t i4_b;
		replicate(u2_a, 0xffff0000);
		replicate(i2_a, 0xffff0000);
		replicate(u4_a, 0xffff0000);
		replicate(i4_a, 0xffff0000);
		replicate(u2_b, 0x00ffff00);
		replicate(i2_b, 0x00ffff00);
		replicate(u4_b, 0x00ffff00);
		replicate(i4_b, 0x00ffff00);

		auto ru2 = bitwise_and(u2_a, u2_b);
		auto ri2 = bitwise_and(i2_a, i2_b);
		auto ru4 = bitwise_and(u4_a, u4_b);
		auto ri4 = bitwise_and(i4_a, i4_b);

		TEST(get<X>(ri2)).should == 0x00ff0000;
		TEST(get<Y>(ri2)).should == 0x00ff0000;

		TEST(get<X>(ru2)).should == 0x00ff0000;
		TEST(get<Y>(ru2)).should == 0x00ff0000;
	
		TEST(get<X>(ri4)).should == 0x00ff0000;
		TEST(get<Y>(ri4)).should == 0x00ff0000;
		TEST(get<Z>(ri4)).should == 0x00ff0000;
		TEST(get<W>(ri4)).should == 0x00ff0000;

		TEST(get<X>(ru4)).should == 0x00ff0000;
		TEST(get<Y>(ru4)).should == 0x00ff0000;
		TEST(get<Z>(ru4)).should == 0x00ff0000;
		TEST(get<W>(ru4)).should == 0x00ff0000;
	});

	feature("bitwise_or", []() {
		uvec2_t u2_a;
		uvec2_t u2_b;
		ivec2_t i2_a;
		ivec2_t i2_b;
		uvec4_t u4_a;
		uvec4_t u4_b;
		ivec4_t i4_a;
		ivec4_t i4_b;
		replicate(u2_a, 0xffff0000);
		replicate(i2_a, 0xffff0000);
		replicate(u4_a, 0xffff0000);
		replicate(i4_a, 0xffff0000);
		replicate(u2_b, 0x00ffff00);
		replicate(i2_b, 0x00ffff00);
		replicate(u4_b, 0x00ffff00);
		replicate(i4_b, 0x00ffff00);

		auto ru2 = bitwise_or(u2_a, u2_b);
		auto ri2 = bitwise_or(i2_a, i2_b);
		auto ru4 = bitwise_or(u4_a, u4_b);
		auto ri4 = bitwise_or(i4_a, i4_b);

		TEST(get<X>(ri2)).should == 0xffffff00;
		TEST(get<Y>(ri2)).should == 0xffffff00;

		TEST(get<X>(ru2)).should == 0xffffff00;
		TEST(get<Y>(ru2)).should == 0xffffff00;
	
		TEST(get<X>(ri4)).should == 0xffffff00;
		TEST(get<Y>(ri4)).should == 0xffffff00;
		TEST(get<Z>(ri4)).should == 0xffffff00;
		TEST(get<W>(ri4)).should == 0xffffff00;

		TEST(get<X>(ru4)).should == 0xffffff00;
		TEST(get<Y>(ru4)).should == 0xffffff00;
		TEST(get<Z>(ru4)).should == 0xffffff00;
		TEST(get<W>(ru4)).should == 0xffffff00;
	});

	feature("bitwise_xor", []() {
		uvec2_t u2_a;
		uvec2_t u2_b;
		ivec2_t i2_a;
		ivec2_t i2_b;
		uvec4_t u4_a;
		uvec4_t u4_b;
		ivec4_t i4_a;
		ivec4_t i4_b;
		replicate(u2_a, 0xffff0000);
		replicate(i2_a, 0xffff0000);
		replicate(u4_a, 0xffff0000);
		replicate(i4_a, 0xffff0000);
		replicate(u2_b, 0x00ffff00);
		replicate(i2_b, 0x00ffff00);
		replicate(u4_b, 0x00ffff00);
		replicate(i4_b, 0x00ffff00);

		auto ru2 = bitwise_xor(u2_a, u2_b);
		auto ri2 = bitwise_xor(i2_a, i2_b);
		auto ru4 = bitwise_xor(u4_a, u4_b);
		auto ri4 = bitwise_xor(i4_a, i4_b);

		TEST(get<X>(ri2)).should == 0xff00ff00;
		TEST(get<Y>(ri2)).should == 0xff00ff00;

		TEST(get<X>(ru2)).should == 0xff00ff00;
		TEST(get<Y>(ru2)).should == 0xff00ff00;
	
		TEST(get<X>(ri4)).should == 0xff00ff00;
		TEST(get<Y>(ri4)).should == 0xff00ff00;
		TEST(get<Z>(ri4)).should == 0xff00ff00;
		TEST(get<W>(ri4)).should == 0xff00ff00;

		TEST(get<X>(ru4)).should == 0xff00ff00;
		TEST(get<Y>(ru4)).should == 0xff00ff00;
		TEST(get<Z>(ru4)).should == 0xff00ff00;
		TEST(get<W>(ru4)).should == 0xff00ff00;
	});

	feature("bitwise_not", []() {
		uvec2_t u2;
		ivec2_t i2;
		uvec4_t u4;
		ivec4_t i4;
		replicate(u2, 0xffff0000);
		replicate(i2, 0xffff0000);
		replicate(u4, 0xffff0000);
		replicate(i4, 0xffff0000);

		auto ru2 = bitwise_not(u2);
		auto ri2 = bitwise_not(i2);
		auto ru4 = bitwise_not(u4);
		auto ri4 = bitwise_not(i4);

		TEST(get<X>(ri2)).should == 0x0000ffff;
		TEST(get<Y>(ri2)).should == 0x0000ffff;

		TEST(get<X>(ru2)).should == 0x0000ffff;
		TEST(get<Y>(ru2)).should == 0x0000ffff;
	
		TEST(get<X>(ri4)).should == 0x0000ffff;
		TEST(get<Y>(ri4)).should == 0x0000ffff;
		TEST(get<Z>(ri4)).should == 0x0000ffff;
		TEST(get<W>(ri4)).should == 0x0000ffff;

		TEST(get<X>(ru4)).should == 0x0000ffff;
		TEST(get<Y>(ru4)).should == 0x0000ffff;
		TEST(get<Z>(ru4)).should == 0x0000ffff;
		TEST(get<W>(ru4)).should == 0x0000ffff;
	});

	feature("cmp_lt", []() {
		float32 ALIGNED(16) srcf_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		float32 ALIGNED(16) srcf_b[] = {5, 4, 3, 2};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		fvec2_t vf2_a;
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		fvec4_t vf4_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		fvec2_t vf2_b;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		fvec4_t vf4_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vf2_a, srcf_a);
		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vf4_a, srcf_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vf2_b, srcf_b);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vf4_b, srcf_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		auto rf2 = cmp_lt(vf2_a, vf2_b);
		auto ri2 = cmp_lt(vi2_a, vi2_b);
		auto ru2 = cmp_lt(vu2_a, vu2_b);
		auto rf4 = cmp_lt(vf4_a, vf4_b);
		auto ri4 = cmp_lt(vi4_a, vi4_b);
		auto ru4 = cmp_lt(vu4_a, vu4_b);

		TEST(get<X>(rf2)).should == 0xffffffff;
		TEST(get<Y>(rf2)).should == 0xffffffff;

		TEST(get<X>(ri2)).should == 0xffffffff;
		TEST(get<Y>(ri2)).should == 0xffffffff;

		TEST(get<X>(ru2)).should == 0xffffffff;
		TEST(get<Y>(ru2)).should == 0xffffffff;
	
		TEST(get<X>(rf4)).should == 0xffffffff;
		TEST(get<Y>(rf4)).should == 0xffffffff;
		TEST(get<Z>(rf4)).should == 0;
		TEST(get<W>(rf4)).should == 0;

		TEST(get<X>(ri4)).should == 0xffffffff;
		TEST(get<Y>(ri4)).should == 0xffffffff;
		TEST(get<Z>(ri4)).should == 0;
		TEST(get<W>(ri4)).should == 0;

		TEST(get<X>(ru4)).should == 0xffffffff;
		TEST(get<Y>(ru4)).should == 0xffffffff;
		TEST(get<Z>(ru4)).should == 0;
		TEST(get<W>(ru4)).should == 0;
	});

	feature("cmp_lte", []() {
				float32 ALIGNED(16) srcf_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		float32 ALIGNED(16) srcf_b[] = {5, 4, 3, 2};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		fvec2_t vf2_a;
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		fvec4_t vf4_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		fvec2_t vf2_b;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		fvec4_t vf4_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vf2_a, srcf_a);
		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vf4_a, srcf_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vf2_b, srcf_b);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vf4_b, srcf_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		auto rf2 = cmp_lte(vf2_a, vf2_b);
		auto ri2 = cmp_lte(vi2_a, vi2_b);
		auto ru2 = cmp_lte(vu2_a, vu2_b);
		auto rf4 = cmp_lte(vf4_a, vf4_b);
		auto ri4 = cmp_lte(vi4_a, vi4_b);
		auto ru4 = cmp_lte(vu4_a, vu4_b);

		TEST(get<X>(rf2)).should == 0xffffffff;
		TEST(get<Y>(rf2)).should == 0xffffffff;

		TEST(get<X>(ri2)).should == 0xffffffff;
		TEST(get<Y>(ri2)).should == 0xffffffff;

		TEST(get<X>(ru2)).should == 0xffffffff;
		TEST(get<Y>(ru2)).should == 0xffffffff;
	
		TEST(get<X>(rf4)).should == 0xffffffff;
		TEST(get<Y>(rf4)).should == 0xffffffff;
		TEST(get<Z>(rf4)).should == 0xffffffff;
		TEST(get<W>(rf4)).should == 0;

		TEST(get<X>(ri4)).should == 0xffffffff;
		TEST(get<Y>(ri4)).should == 0xffffffff;
		TEST(get<Z>(ri4)).should == 0xffffffff;
		TEST(get<W>(ri4)).should == 0;

		TEST(get<X>(ru4)).should == 0xffffffff;
		TEST(get<Y>(ru4)).should == 0xffffffff;
		TEST(get<Z>(ru4)).should == 0xffffffff;
		TEST(get<W>(ru4)).should == 0;
	});

	feature("cmp_gt", []() {
		float32 ALIGNED(16) srcf_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		float32 ALIGNED(16) srcf_b[] = {5, 4, 3, 2};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		fvec2_t vf2_a;
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		fvec4_t vf4_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		fvec2_t vf2_b;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		fvec4_t vf4_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vf2_a, srcf_a);
		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vf4_a, srcf_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vf2_b, srcf_b);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vf4_b, srcf_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		auto rf2 = cmp_gt(vf2_a, vf2_b);
		auto ri2 = cmp_gt(vi2_a, vi2_b);
		auto ru2 = cmp_gt(vu2_a, vu2_b);
		auto rf4 = cmp_gt(vf4_a, vf4_b);
		auto ri4 = cmp_gt(vi4_a, vi4_b);
		auto ru4 = cmp_gt(vu4_a, vu4_b);

		TEST(get<X>(rf2)).should == 0;
		TEST(get<Y>(rf2)).should == 0;

		TEST(get<X>(ri2)).should == 0;
		TEST(get<Y>(ri2)).should == 0;

		TEST(get<X>(ru2)).should == 0;
		TEST(get<Y>(ru2)).should == 0;
	
		TEST(get<X>(rf4)).should == 0;
		TEST(get<Y>(rf4)).should == 0;
		TEST(get<Z>(rf4)).should == 0;
		TEST(get<W>(rf4)).should == 0xffffffff;

		TEST(get<X>(ri4)).should == 0;
		TEST(get<Y>(ri4)).should == 0;
		TEST(get<Z>(ri4)).should == 0;
		TEST(get<W>(ri4)).should == 0xffffffff;

		TEST(get<X>(ru4)).should == 0;
		TEST(get<Y>(ru4)).should == 0;
		TEST(get<Z>(ru4)).should == 0;
		TEST(get<W>(ru4)).should == 0xffffffff;
	});

	feature("cmp_gte", []() {
		float32 ALIGNED(16) srcf_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		float32 ALIGNED(16) srcf_b[] = {5, 4, 3, 2};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		fvec2_t vf2_a;
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		fvec4_t vf4_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		fvec2_t vf2_b;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		fvec4_t vf4_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vf2_a, srcf_a);
		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vf4_a, srcf_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vf2_b, srcf_b);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vf4_b, srcf_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		auto rf2 = cmp_gte(vf2_a, vf2_b);
		auto ri2 = cmp_gte(vi2_a, vi2_b);
		auto ru2 = cmp_gte(vu2_a, vu2_b);
		auto rf4 = cmp_gte(vf4_a, vf4_b);
		auto ri4 = cmp_gte(vi4_a, vi4_b);
		auto ru4 = cmp_gte(vu4_a, vu4_b);

		TEST(get<X>(rf2)).should == 0;
		TEST(get<Y>(rf2)).should == 0;

		TEST(get<X>(ri2)).should == 0;
		TEST(get<Y>(ri2)).should == 0;

		TEST(get<X>(ru2)).should == 0;
		TEST(get<Y>(ru2)).should == 0;
	
		TEST(get<X>(rf4)).should == 0;
		TEST(get<Y>(rf4)).should == 0;
		TEST(get<Z>(rf4)).should == 0xffffffff;
		TEST(get<W>(rf4)).should == 0xffffffff;

		TEST(get<X>(ri4)).should == 0;
		TEST(get<Y>(ri4)).should == 0;
		TEST(get<Z>(ri4)).should == 0xffffffff;
		TEST(get<W>(ri4)).should == 0xffffffff;

		TEST(get<X>(ru4)).should == 0;
		TEST(get<Y>(ru4)).should == 0;
		TEST(get<Z>(ru4)).should == 0xffffffff;
		TEST(get<W>(ru4)).should == 0xffffffff;
	});

	feature("cmp_eq", []() {
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		auto ri2 = cmp_eq(vi2_a, vi2_b);
		auto ru2 = cmp_eq(vu2_a, vu2_b);
		auto ri4 = cmp_eq(vi4_a, vi4_b);
		auto ru4 = cmp_eq(vu4_a, vu4_b);

		TEST(get<X>(ri2)).should == 0;
		TEST(get<Y>(ri2)).should == 0;

		TEST(get<X>(ru2)).should == 0;
		TEST(get<Y>(ru2)).should == 0;

		TEST(get<X>(ri4)).should == 0;
		TEST(get<Y>(ri4)).should == 0;
		TEST(get<Z>(ri4)).should == 0xffffffff;
		TEST(get<W>(ri4)).should == 0;

		TEST(get<X>(ru4)).should == 0;
		TEST(get<Y>(ru4)).should == 0;
		TEST(get<Z>(ru4)).should == 0xffffffff;
		TEST(get<W>(ru4)).should == 0;
	});

	feature("cmp_neq", []() {
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		auto ri2 = cmp_neq(vi2_a, vi2_b);
		auto ru2 = cmp_neq(vu2_a, vu2_b);
		auto ri4 = cmp_neq(vi4_a, vi4_b);
		auto ru4 = cmp_neq(vu4_a, vu4_b);

		TEST(get<X>(ri2)).should == 0xffffffff;
		TEST(get<Y>(ri2)).should == 0xffffffff;

		TEST(get<X>(ru2)).should == 0xffffffff;
		TEST(get<Y>(ru2)).should == 0xffffffff;

		TEST(get<X>(ri4)).should == 0xffffffff;
		TEST(get<Y>(ri4)).should == 0xffffffff;
		TEST(get<Z>(ri4)).should == 0;
		TEST(get<W>(ri4)).should == 0xffffffff;

		TEST(get<X>(ru4)).should == 0xffffffff;
		TEST(get<Y>(ru4)).should == 0xffffffff;
		TEST(get<Z>(ru4)).should == 0;
		TEST(get<W>(ru4)).should == 0xffffffff;
	});

	feature("shuffle vec2", []() {
		float32 ALIGNED(16) srcf[] = {1, 2};
		int32   ALIGNED(16) srci[] = {1, 2};
		uint32  ALIGNED(16) srcu[] = {1, 2};
		fvec2_t vf;
		ivec2_t vi;
		uvec2_t vu;
		aligned_load(vf, srcf);
		aligned_load(vi, srci);
		aligned_load(vu, srcu);

		// Y,X
		auto rf_yx = shuffle<Y,X>(vf);
		auto ri_yx = shuffle<Y,X>(vi);
		auto ru_yx = shuffle<Y,X>(vu);

		TEST(get<X>(rf_yx)).should == 2;
		TEST(get<Y>(rf_yx)).should == 1;

		TEST(get<X>(ri_yx)).should == 2;
		TEST(get<Y>(ri_yx)).should == 1;

		TEST(get<X>(ru_yx)).should == 2;
		TEST(get<Y>(ru_yx)).should == 1;

		// X,X
		auto rf_xx = shuffle<X,X>(vf);
		auto ri_xx = shuffle<X,X>(vi);
		auto ru_xx = shuffle<X,X>(vu);

		TEST(get<X>(rf_xx)).should == 1;
		TEST(get<Y>(rf_xx)).should == 1;

		TEST(get<X>(ri_xx)).should == 1;
		TEST(get<Y>(ri_xx)).should == 1;

		TEST(get<X>(ru_xx)).should == 1;
		TEST(get<Y>(ru_xx)).should == 1;

		// Y,Y
		auto rf_yy = shuffle<Y,Y>(vf);
		auto ri_yy = shuffle<Y,Y>(vi);
		auto ru_yy = shuffle<Y,Y>(vu);

		TEST(get<X>(rf_yy)).should == 2;
		TEST(get<Y>(rf_yy)).should == 2;

		TEST(get<X>(ri_yy)).should == 2;
		TEST(get<Y>(ri_yy)).should == 2;

		TEST(get<X>(ru_yy)).should == 2;
		TEST(get<Y>(ru_yy)).should == 2;
	});

#if TEST_SHUFFLES
	feature("shuffle vec4", []() {
		float32 ALIGNED(16) srcf[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu[] = {1, 2, 3, 4};
		fvec4_t vf;
		ivec4_t vi;
		uvec4_t vu;
		aligned_load(vf, srcf);
		aligned_load(vi, srci);
		aligned_load(vu, srcu);
		
		TestShuffleVec4::test(vf);
		TestShuffleVec4::test(vi);
		TestShuffleVec4::test(vu);
	});

	feature("shuffle2 vec4", []() {
		float32 ALIGNED(16) srcf_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		float32 ALIGNED(16) srcf_b[] = {5, 6, 7, 8};
		int32   ALIGNED(16) srci_b[] = {5, 6, 7, 8};
		uint32  ALIGNED(16) srcu_b[] = {5, 6, 7, 8};
		fvec4_t vf_a;
		ivec4_t vi_a;
		uvec4_t vu_a;
		aligned_load(vf_a, srcf_a);
		aligned_load(vi_a, srci_a);
		aligned_load(vu_a, srcu_a);
		fvec4_t vf_b;
		ivec4_t vi_b;
		uvec4_t vu_b;
		aligned_load(vf_b, srcf_b);
		aligned_load(vi_b, srci_b);
		aligned_load(vu_b, srcu_b);
		
		TestShuffle2Vec4::test(vf_a, vf_b);
		TestShuffle2Vec4::test(vi_a, vi_b);
		TestShuffle2Vec4::test(vu_a, vu_b);
	});
#endif
	
	feature("select", []() {
		float32 ALIGNED(16) srcf_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		float32 ALIGNED(16) srcf_b[] = {5, 4, 3, 2};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		fvec2_t vf2_a;
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		fvec4_t vf4_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		fvec2_t vf2_b;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		fvec4_t vf4_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vf2_a, srcf_a);
		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vf4_a, srcf_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vf2_b, srcf_b);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vf4_b, srcf_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		uvec2_t u2mask = set((uint32)0xffffffff, (uint32)0);
		uvec4_t u4mask = set((uint32)0xffffffff, (uint32)0, (uint32)0xffffffff, (uint32)0);

		auto rf2 = select(u2mask, vf2_a, vf2_b);
		auto ri2 = select(u2mask, vi2_a, vi2_b);
		auto ru2 = select(u2mask, vu2_a, vu2_b);
		auto rf4 = select(u4mask, vf4_a, vf4_b);
		auto ri4 = select(u4mask, vi4_a, vi4_b);
		auto ru4 = select(u4mask, vu4_a, vu4_b);

		TEST(get<X>(rf2)).should == 1;
		TEST(get<Y>(rf2)).should == 4;

		TEST(get<X>(ri2)).should == 1;
		TEST(get<Y>(ri2)).should == 4;

		TEST(get<X>(ru2)).should == 1;
		TEST(get<Y>(ru2)).should == 4;
	
		TEST(get<X>(rf4)).should == 1;
		TEST(get<Y>(rf4)).should == 4;
		TEST(get<Z>(rf4)).should == 3;
		TEST(get<W>(rf4)).should == 2;

		TEST(get<X>(ri4)).should == 1;
		TEST(get<Y>(ri4)).should == 4;
		TEST(get<Z>(ri4)).should == 3;
		TEST(get<W>(ri4)).should == 2;

		TEST(get<X>(ru4)).should == 1;
		TEST(get<Y>(ru4)).should == 4;
		TEST(get<Z>(ru4)).should == 3;
		TEST(get<W>(ru4)).should == 2;
	});

	feature("hadd2", []() {
		float32 ALIGNED(16) srcf[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu[] = {1, 2, 3, 4};
		fvec2_t vf2;
		ivec2_t vi2;
		uvec2_t vu2;
		aligned_load(vf2, srcf);
		aligned_load(vi2, srci);
		aligned_load(vu2, srcu);

		auto rf2 = hadd2(vf2);
		auto ri2 = hadd2(vi2);
		auto ru2 = hadd2(vu2);

		TEST(get<X>(rf2)).should == 3;
		TEST(get<Y>(rf2)).should == 3;

		TEST(get<X>(ri2)).should == 3;
		TEST(get<Y>(ri2)).should == 3;

		TEST(get<X>(ru2)).should == 3;
		TEST(get<Y>(ru2)).should == 3;
	});

	feature("hadd3", []() {
		float32 ALIGNED(16) srcf[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu[] = {1, 2, 3, 4};
		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);
		aligned_load(vu4, srcu);

		auto rf4 = hadd3(vf4);
		auto ri4 = hadd3(vi4);
		auto ru4 = hadd3(vu4);

		TEST(get<X>(rf4)).should == 6;
		TEST(get<Y>(rf4)).should == 6;
		TEST(get<Z>(rf4)).should == 6;

		TEST(get<X>(ri4)).should == 6;
		TEST(get<Y>(ri4)).should == 6;
		TEST(get<Z>(ri4)).should == 6;

		TEST(get<X>(ru4)).should == 6;
		TEST(get<Y>(ru4)).should == 6;
		TEST(get<Z>(ru4)).should == 6;
	});

	feature("hadd4", []() {
		float32 ALIGNED(16) srcf[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu[] = {1, 2, 3, 4};
		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);
		aligned_load(vu4, srcu);

		auto rf4 = hadd4(vf4);
		auto ri4 = hadd4(vi4);
		auto ru4 = hadd4(vu4);

		TEST(get<X>(rf4)).should == 10;
		TEST(get<Y>(rf4)).should == 10;
		TEST(get<Z>(rf4)).should == 10;
		TEST(get<W>(rf4)).should == 10;

		TEST(get<X>(ri4)).should == 10;
		TEST(get<Y>(ri4)).should == 10;
		TEST(get<Z>(ri4)).should == 10;
		TEST(get<W>(ri4)).should == 10;

		TEST(get<X>(ru4)).should == 10;
		TEST(get<Y>(ru4)).should == 10;
		TEST(get<Z>(ru4)).should == 10;
		TEST(get<W>(ru4)).should == 10;
	});

	//feature("sqrt");
	//feature("rsqrt");

	feature("abs", []() {
		float32 ALIGNED(16) srcf[] = {1, -2, 3, -4};
		int32   ALIGNED(16) srci[] = {1, -2, 3, -4};
		uint32  ALIGNED(16) srcu[] = {1, UINT32_MAX, 3, 4};
		fvec4_t vf4;
		ivec4_t vi4;
		uvec4_t vu4;
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);
		aligned_load(vu4, srcu);

		auto rf4 = abs(vf4);
		auto ri4 = abs(vi4);
		auto ru4 = abs(vu4);

		TEST(get<X>(rf4)).should == 1;
		TEST(get<Y>(rf4)).should == 2;
		TEST(get<Z>(rf4)).should == 3;
		TEST(get<W>(rf4)).should == 4;

		TEST(get<X>(ri4)).should == 1;
		TEST(get<Y>(ri4)).should == 2;
		TEST(get<Z>(ri4)).should == 3;
		TEST(get<W>(ri4)).should == 4;

		TEST(get<X>(ru4)).should == 1;
		TEST(get<Y>(ru4)).should == UINT32_MAX;
		TEST(get<Z>(ru4)).should == 3;
		TEST(get<W>(ru4)).should == 4;
	});

	feature("neg", []() {
		float32 ALIGNED(16) srcf[] = {1, -2, 3, -4};
		int32   ALIGNED(16) srci[] = {1, -2, 3, -4};
		fvec4_t vf4;
		ivec4_t vi4;
		aligned_load(vf4, srcf);
		aligned_load(vi4, srci);

		auto rf4 = neg(vf4);
		auto ri4 = neg(vi4);

		TEST(get<X>(rf4)).should == -1;
		TEST(get<Y>(rf4)).should == 2;
		TEST(get<Z>(rf4)).should == -3;
		TEST(get<W>(rf4)).should == 4;

		TEST(get<X>(ri4)).should == -1;
		TEST(get<Y>(ri4)).should == 2;
		TEST(get<Z>(ri4)).should == -3;
		TEST(get<W>(ri4)).should == 4;
	});

	feature("round", []() {
		float32 ALIGNED(16) srcf[] = {1.2, -2.8, 3.8, -4.2};
		fvec4_t vf4;
		aligned_load(vf4, srcf);

		auto rf4 = round(vf4);

		TEST(get<X>(rf4)).should == 1;
		TEST(get<Y>(rf4)).should == -3;
		TEST(get<Z>(rf4)).should == 4;
		TEST(get<W>(rf4)).should == -4;
	});

	feature("ceil", []() {
		float32 ALIGNED(16) srcf[] = {1.2, -2.8, 3.8, -4.2};
		fvec4_t vf4;
		aligned_load(vf4, srcf);

		auto rf4 = ceil(vf4);

		TEST(get<X>(rf4)).should == 2;
		TEST(get<Y>(rf4)).should == -2;
		TEST(get<Z>(rf4)).should == 4;
		TEST(get<W>(rf4)).should == -4;
	});

	feature("floor", []() {
		float32 ALIGNED(16) srcf[] = {1.2, -2.8, 3.8, -4.2};
		fvec4_t vf4;
		aligned_load(vf4, srcf);

		auto rf4 = floor(vf4);

		TEST(get<X>(rf4)).should == 1;
		TEST(get<Y>(rf4)).should == -3;
		TEST(get<Z>(rf4)).should == 3;
		TEST(get<W>(rf4)).should == -5;
	});

	feature("min", []() {
		float32 ALIGNED(16) srcf_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		float32 ALIGNED(16) srcf_b[] = {5, 4, 3, 2};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		fvec2_t vf2_a;
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		fvec4_t vf4_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		fvec2_t vf2_b;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		fvec4_t vf4_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vf2_a, srcf_a);
		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vf4_a, srcf_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vf2_b, srcf_b);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vf4_b, srcf_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		auto rf2 = min(vf2_a, vf2_b);
		auto ri2 = min(vi2_a, vi2_b);
		auto ru2 = min(vu2_a, vu2_b);
		auto rf4 = min(vf4_a, vf4_b);
		auto ri4 = min(vi4_a, vi4_b);
		auto ru4 = min(vu4_a, vu4_b);

		TEST(get<X>(rf2)).should == 1;
		TEST(get<Y>(rf2)).should == 2;

		TEST(get<X>(ri2)).should == 1;
		TEST(get<Y>(ri2)).should == 2;

		TEST(get<X>(ru2)).should == 1;
		TEST(get<Y>(ru2)).should == 2;
	
		TEST(get<X>(rf4)).should == 1;
		TEST(get<Y>(rf4)).should == 2;
		TEST(get<Z>(rf4)).should == 3;
		TEST(get<W>(rf4)).should == 2;

		TEST(get<X>(ri4)).should == 1;
		TEST(get<Y>(ri4)).should == 2;
		TEST(get<Z>(ri4)).should == 3;
		TEST(get<W>(ri4)).should == 2;

		TEST(get<X>(ru4)).should == 1;
		TEST(get<Y>(ru4)).should == 2;
		TEST(get<Z>(ru4)).should == 3;
		TEST(get<W>(ru4)).should == 2;
	});

	feature("max", []() {
		float32 ALIGNED(16) srcf_a[] = {1, 2, 3, 4};
		int32   ALIGNED(16) srci_a[] = {1, 2, 3, 4};
		uint32  ALIGNED(16) srcu_a[] = {1, 2, 3, 4};
		float32 ALIGNED(16) srcf_b[] = {5, 4, 3, 2};
		int32   ALIGNED(16) srci_b[] = {5, 4, 3, 2};
		uint32  ALIGNED(16) srcu_b[] = {5, 4, 3, 2};
	
		fvec2_t vf2_a;
		ivec2_t vi2_a;
		uvec2_t vu2_a;
		fvec4_t vf4_a;
		ivec4_t vi4_a;
		uvec4_t vu4_a;
		fvec2_t vf2_b;
		ivec2_t vi2_b;
		uvec2_t vu2_b;
		fvec4_t vf4_b;
		ivec4_t vi4_b;
		uvec4_t vu4_b;

		aligned_load(vf2_a, srcf_a);
		aligned_load(vi2_a, srci_a);
		aligned_load(vu2_a, srcu_a);
		aligned_load(vf4_a, srcf_a);
		aligned_load(vi4_a, srci_a);
		aligned_load(vu4_a, srcu_a);
		aligned_load(vf2_b, srcf_b);
		aligned_load(vi2_b, srci_b);
		aligned_load(vu2_b, srcu_b);
		aligned_load(vf4_b, srcf_b);
		aligned_load(vi4_b, srci_b);
		aligned_load(vu4_b, srcu_b);

		auto rf2 = max(vf2_a, vf2_b);
		auto ri2 = max(vi2_a, vi2_b);
		auto ru2 = max(vu2_a, vu2_b);
		auto rf4 = max(vf4_a, vf4_b);
		auto ri4 = max(vi4_a, vi4_b);
		auto ru4 = max(vu4_a, vu4_b);

		TEST(get<X>(rf2)).should == 5;
		TEST(get<Y>(rf2)).should == 4;

		TEST(get<X>(ri2)).should == 5;
		TEST(get<Y>(ri2)).should == 4;

		TEST(get<X>(ru2)).should == 5;
		TEST(get<Y>(ru2)).should == 4;
	
		TEST(get<X>(rf4)).should == 5;
		TEST(get<Y>(rf4)).should == 4;
		TEST(get<Z>(rf4)).should == 3;
		TEST(get<W>(rf4)).should == 4;

		TEST(get<X>(ri4)).should == 5;
		TEST(get<Y>(ri4)).should == 4;
		TEST(get<Z>(ri4)).should == 3;
		TEST(get<W>(ri4)).should == 4;

		TEST(get<X>(ru4)).should == 5;
		TEST(get<Y>(ru4)).should == 4;
		TEST(get<Z>(ru4)).should == 3;
		TEST(get<W>(ru4)).should == 4;
	});

	feature("num_true", []() {
		uvec2_t u2 = set((uint32)0xffffffff, (uint32)0);
		TEST(num_true(u2)).should == 1;
		uvec4_t u4 = set((uint32)0xffffffff, (uint32)0, (uint32)0xffffffff, (uint32)0);
		TEST(num_true(u4)).should == 2;
	});

	/*feature("all_true4");
	feature("all_true3");
	feature("all_true2");
	feature("any_true4");
	feature("any_true3");
	feature("any_true2");*/
}
