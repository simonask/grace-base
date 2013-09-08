//
//  aspect_cast_test.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 05/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_aspect_cast_test_hpp
#define grace_aspect_cast_test_hpp

#include "tests/test.hpp"
#include "type/type.hpp"
#include "object/object.hpp"
#include "object/reflect.hpp"
#include "object/universe.hpp"
#include "object/composite_type.hpp"
#include "memory/unique_ptr.hpp"

using namespace grace;

struct A {
	int a;
};

struct B : A {
	int b;
};

struct VA {
	virtual ~VA() {}
	int a;
};

struct VB : VA {
	virtual ~VB() {}
	int b;
};

__attribute__((noinline)) VA* create_vb_as_va() {
	return new VB;
}

struct OA : public Object {
	REFLECT;
public:
	int a;
};
BEGIN_TYPE_INFO(OA)
property(&OA::a, "a", "");
END_TYPE_INFO()

struct OB : public OA {
	REFLECT;
public:
	int b;
};
BEGIN_TYPE_INFO(OB)
super<OA>();
property(&OB::b, "b", "");
END_TYPE_INFO()

struct OC : public OA {
	REFLECT;
public:
	int c;
};
BEGIN_TYPE_INFO(OC)
super<OA>();
property(&OC::c, "c", "");
END_TYPE_INFO()

struct OI {
	virtual void foo() = 0;
};

struct OD : public OA, public OI {
	REFLECT;
	void foo() final { }
};
BEGIN_TYPE_INFO(OD)
	super<OA>();
END_TYPE_INFO()

struct OE : public OD {
	REFLECT;
};
BEGIN_TYPE_INFO(OE)
super<OD>();
END_TYPE_INFO()


SUITE(aspect_cast) {
	it("should be able to statically downcast non-Object types", []() {
		// This would crash if runtime-casting was performed:
		B* b = reinterpret_cast<B*>((void*)0x1);
		A* a = aspect_cast<A>(b);
		TEST(a).should != nullptr;
	});
	
	it("should be able to dynamically upcast non-Object types", []() {
		VA* a = create_vb_as_va();
		VB* b = aspect_cast<VB>(a);
		TEST(b).should != nullptr;
		destroy(a, default_allocator());
	});
	
	it("should be able to statically downcast Object types", []() {
		// This would crash if runtime-casting was performed:
		OB* b = reinterpret_cast<OB*>((void*)0x1);
		OA* a = aspect_cast<OA>(b);
		TEST(a).should != nullptr;
	});
	
	it("should be able to dynamically upcast Object types", []() {
		TestUniverse universe;
		ObjectPtr<> root = universe.create_object_and_set_as_root(get_type<OB>(), "OB01");
		ObjectPtr<OA> a(static_cast<OA*>(root.get())); // Go around aspect_cast...
		ObjectPtr<OB> b = aspect_cast<OB>(a);
		TEST(b).should != nullptr;
	});
	
	it("should be able to side-cast from composite object to contained aspect", []() {
		auto type = make_unique<CompositeType>(default_allocator(), default_allocator(), "MyComposite");
		TestUniverse universe;
		type->add_aspect(get_type<OB>());
		type->add_aspect(get_type<OC>());
		ObjectPtr<> o = universe.create_object_and_set_as_root(type.get(), "MyComposite01");
		ObjectPtr<OB> b = aspect_cast<OB>(o);
		TEST(b).should != nullptr;
		ObjectPtr<OC> c = aspect_cast<OC>(o);
		TEST(c).should != nullptr;
	});
	
	it("should be able to side-cast from aspect to another aspect", []() {
		auto type = make_unique<CompositeType>(default_allocator(), default_allocator(), "MyComposite");
		TestUniverse universe;
		type->add_aspect(get_type<OB>());
		type->add_aspect(get_type<OC>());
		ObjectPtr<> o = universe.create_object_and_set_as_root(type.get(), "MyComposite01");
		ObjectPtr<OB> b = aspect_cast<OB>(o);
		TEST(b).should != nullptr;
		ObjectPtr<OC> c = aspect_cast<OC>(b);
		TEST(c).should != nullptr;
	});
	
	it("should be able to upcast from aspect to basetype of composite", []() {
		auto type = make_unique<CompositeType>(default_allocator(), default_allocator(), "MyComposite", get_type<OC>());
		TestUniverse universe;
		type->add_aspect(get_type<OB>());
		ObjectPtr<> o = universe.create_object_and_set_as_root(type.get(), "MyComposite01");
		ObjectPtr<OB> b = aspect_cast<OB>(o);
		TEST(b).should != nullptr;
		ObjectPtr<OC> c = aspect_cast<OC>(b);
		TEST(c).should != nullptr;
	});
	
	it("should downcast to interface", []() {
		TestUniverse universe;
		ObjectPtr<OA> oa = universe.create<OD>("OD");
		OI* oi = aspect_cast<OI>(oa);
		TEST(oi).should != nullptr;
	});
	
	it("should upcast to interface", []() {
		TestUniverse universe;
		ObjectPtr<OE> oe = universe.create<OE>("OE");
		OI* oi = aspect_cast<OI>(oe);
		TEST(oi).should != nullptr;
	});
	
	it("should downcast to interface from composite", []() {
		TestUniverse universe;
		CompositeType* ct = universe.create_composite_type(get_type<Object>());
		ct->add_aspect(get_type<OA>());
		ct->add_aspect(get_type<OE>());
		ct->freeze();
		ObjectPtr<> obj = universe.create_object(ct, "Comp01");
		OI* oi = aspect_cast<OI>(obj);
		TEST(oi).should != nullptr;
	});
	
	it("should sidecast to interface from composite", []() {
		TestUniverse universe;
		CompositeType* ct = universe.create_composite_type(get_type<Object>());
		ct->add_aspect(get_type<OA>());
		ct->add_aspect(get_type<OE>());
		ct->freeze();
		ObjectPtr<> obj = universe.create_object(ct, "Comp01");
		ObjectPtr<OA> oa = aspect_cast<OA>(obj);
		OI* oi = aspect_cast<OI>(oa);
		TEST(oi).should != nullptr;
	});
}

#endif
