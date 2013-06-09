//
//  pointer_type.h
//  grace
//
//  Created by Simon Ask Ulsnes on 02/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __grace__pointer_type__
#define __grace__pointer_type__

#include "type/type.hpp"
#include <typeinfo>

namespace grace {
	class PointerType : public IType {
	public:
		PointerType(IAllocator& alloc, const Type* pointee_type, bool is_const);
		PointerType(IAllocator& alloc, StringRef name, bool is_const);
		
		// PointerType interface
		const Type* pointee_type() const { return pointee_type_; }
		bool is_const() const { return is_const_; }
		
		// Type information
		StringRef name() const { return name_; }
		size_t size() const final { return sizeof(void*); }
		size_t alignment() const final { return alignof(void*); }
		
		// Type reflection
		void deserialize_raw(byte* place, const ArchiveNode&, IUniverse&) const final;
		void serialize_raw(const byte* place, ArchiveNode&, IUniverse&) const;
		void construct(byte* place, IUniverse&) const;
		void copy_construct(byte* to, const byte* from) const;
		void move_construct(byte* to, byte* from) const;
		void destruct(byte* place, IUniverse&) const;
		bool is_abstract() const final { return false; }
		bool is_copy_constructible() const final { return true; }
		bool is_move_constructible() const final { return true; }
		bool deferred_instantiation() const final { return false; }
	private:
		String name_;
		const Type* pointee_type_;
		bool is_const_;
	};
	
	template <typename T>
	struct BuildTypeInfo<T*> {
		template <typename U = T>
		static typename std::enable_if<CanGetType<U>::Value, const PointerType*>::type
		build() {
			static PointerType* p = new_static PointerType(static_allocator(), get_type<T>(), false);
			return p;
		}
		template <typename U = T>
		static typename std::enable_if<!CanGetType<U>::Value, const PointerType*>::type
		build() {
			static PointerType* p = new_static PointerType(static_allocator(), typeid(T).name(), false);
			return p;
		}
	};
	
	template <typename T>
	struct BuildTypeInfo<const T*> {
		template <typename U = T>
		static typename std::enable_if<CanGetType<U>::Value, const PointerType*>::type
		build() {
			static PointerType* p = new_static PointerType(static_allocator(), get_type<T>(), true);
			return p;
		}
		template <typename U = T>
		static typename std::enable_if<!CanGetType<U>::Value, const PointerType*>::type
		build() {
			static PointerType* p = new_static PointerType(static_allocator(), typeid(T).name(), true);
			return p;
		}
	};
}

#endif /* defined(__grace__pointer_type__) */
