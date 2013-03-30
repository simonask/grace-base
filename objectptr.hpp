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
	
	T* get() const { return ptr_; }
	T* operator->() const { return ptr_; }
	T& operator*() const { return *ptr_; }
	const Type* type() const { return ptr_ ? get_type(*ptr_) : get_type<T>(); }
	
	
	template <typename U>
	bool operator<(const ObjectPtr<U>& other) const { return get() < other.get(); }
private:
	T* ptr_;
};

template <typename OutputStream, typename T>
OutputStream& operator<<(OutputStream& os, const ObjectPtr<T>& ptr) {
	os << '(' << ptr.type()->name() << "*)" << (void*)ptr.get();
	return os;
}

}

#endif /* end of include guard: OBJECTPTR_HPP_WICLN6JL */
