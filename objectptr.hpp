#pragma once
#ifndef OBJECTPTR_HPP_WICLN6JL
#define OBJECTPTR_HPP_WICLN6JL

#include "type/type.hpp"
#include "object/object.hpp"

namespace falling {

/*
	RULES ABOUT ObjectPtr<T>:
	
	1) You may not hold a non-null ObjectPtr<T> outside an Object without registering it
	   with the Universe.
	
	2) ObjectPtr<T>-typed attributes on Objects are automatically registered.
	
	3) Unregistered references may be held by objects, as long as they're not used (set or
	   dereferenced) before initialize() is called on the object holding them.
*/


template <typename T = Object>
struct ObjectPtr {
	typedef T PointeeType;
	
	ObjectPtr() : ptr_(nullptr) {}
	explicit ObjectPtr(T* ptr) : ptr_(ptr) {}
	ObjectPtr(NullPtr null) : ptr_(nullptr) {}
	template <typename U>
	ObjectPtr(ObjectPtr<U> other) { ptr_ = other.get(); }
	ObjectPtr(const ObjectPtr<T>& other) { ptr_ = other.ptr_; }
	ObjectPtr(ObjectPtr<T>&& other) { ptr_ = other.ptr_; }
	template <typename U>
	ObjectPtr<T>& operator=(typename std::enable_if<HasReflection<U>::Value, U*>::type other) { ptr_ = other; return *this; }
	template <typename U>
	ObjectPtr<T>& operator=(ObjectPtr<U> other) { ptr_ = other.get(); return *this; }
	ObjectPtr<T>& operator=(const ObjectPtr<T>& other) { ptr_ = other.ptr_; return *this; }
	template <typename U>
	bool operator==(ObjectPtr<U> other) const { return ptr_ == other.get(); }
	bool operator==(const ObjectPtr<T>& other) const { return ptr_ == other.ptr_; }
	template <typename U>
	bool operator!=(ObjectPtr<U> other) const { return ptr_ != other.get(); }
	bool operator!=(const ObjectPtr<T>& other) const { return ptr_ != other.ptr_; }
	explicit operator bool() const { return ptr_ != nullptr; }
	
	template <typename U>
	ObjectPtr<U> cast() const;
	
	T* get() const { return ptr_; }
	T* operator->() const { return ptr_; }
	T& operator*() const { return *ptr_; }
	const Type* type() const { return ptr_ ? get_type(*ptr_) : get_type<T>(); }
	
	
	template <typename U>
	bool operator<(const ObjectPtr<U>& other) const { return get() < other.get(); }
private:
	T* ptr_;
};

template <typename To, typename From>
ObjectPtr<To>
aspect_cast(ObjectPtr<From> ptr) {
	return ObjectPtr<To>(aspect_cast<To>(ptr.get()));
}

template <typename From>
ObjectPtr<>
aspect_cast(ObjectPtr<From> ptr, const DerivedType* type) {
	return ObjectPtr<>(aspect_cast(ptr.get(), type));
}

template <typename T>
template <typename U>
ObjectPtr<U> ObjectPtr<T>::cast() const {
	return ObjectPtr<U>(aspect_cast<U>(*this));
}

template <typename OutputStream, typename T>
OutputStream& operator<<(OutputStream& os, const ObjectPtr<T>& ptr) {
	os << '(' << ptr.type()->name() << "*)" << (void*)ptr.get();
	return os;
}



struct ObjectPtrRootBase {
	virtual void set(ObjectPtr<> root) const = 0;
	virtual ObjectPtr<> get() const = 0;
	virtual void* ptr() const = 0;
};
template <typename T>
struct ObjectPtrRoot : ObjectPtrRootBase {
	ObjectPtr<T>* root;
	ObjectPtrRoot(ObjectPtr<T>* root) : root(root) {}
	void set(ObjectPtr<> r) const final {
		*root = aspect_cast<T>(r);
	}
	ObjectPtr<> get() const final {
		return *root;
	}
	void* ptr() const final {
		return root;
	}
};


void register_object_root_impl(ObjectPtrRootBase* root_descriptor, IUniverse& universe);
void unregister_object_root_impl(void* root, IUniverse& universe);
	
template <typename T>
void register_object_root(ObjectPtr<T>* root, IUniverse& universe) {
#if !defined(DISABLE_EDITOR_FEATURES)
	register_object_root_impl(new(default_allocator()) ObjectPtrRoot<T>(root), universe);
#endif
}
template <typename T>
void unregister_object_root(ObjectPtr<T>* root, IUniverse& universe) {
#if !defined(DISABLE_EDITOR_FEATURES)
	unregister_object_root_impl(root, universe);
#endif
}

}

#endif /* end of include guard: OBJECTPTR_HPP_WICLN6JL */
