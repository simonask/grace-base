//
//  type_info.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 28/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_type_info_hpp
#define falling_type_info_hpp

#include "base/basic.hpp"
#include <type_traits>

namespace falling {
	struct TypeInfo {
		using ConstructorFuncPtr     = void(*)(byte*);
		using DestructorFuncPtr      = void(*)(byte*);
		using CopyAssignFuncPtr      = void(*)(byte*, const byte*);
		using CopyConstructFuncPtr   = void(*)(byte*, const byte*);
		using MoveAssignFuncPtr      = void(*)(byte*, byte*);
		using MoveConstructFuncPtr   = void(*)(byte*, byte*);
		
		const std::type_info& internal_info;
		const size_t size;
		const size_t alignment;
		
		const ConstructorFuncPtr   construct;
		const DestructorFuncPtr    destruct;
		const CopyAssignFuncPtr    copy_assign;
		const CopyConstructFuncPtr copy_construct;
		const MoveAssignFuncPtr    move_assign;
		const MoveConstructFuncPtr move_construct;
		
		bool is_constructible() const { return construct != nullptr; }
		bool is_copy_assignable() const         { return copy_assign != nullptr; }
		bool is_copy_constructible() const      { return copy_construct != nullptr; }
		bool is_move_assignable() const         { return move_assign != nullptr; }
		bool is_move_constructible() const      { return move_construct != nullptr; }
		bool is_move_or_copy_constructible() const { return is_move_constructible() || is_copy_constructible(); }
		bool is_move_or_copy_assignable() const    { return is_move_assignable() || is_copy_assignable(); }
		bool is_abstract() const { return !is_constructible() && !is_move_or_copy_constructible(); }
		
		void move_or_copy_construct(byte* self, byte* other) const {
			if (move_construct) {
				move_construct(self, other);
			} else if (copy_construct) {
				copy_construct(self, other);
			}
		}
		void move_or_copy_assign(byte* self, byte* other) const {
			if (move_assign) {
				move_assign(self, other);
			} else if (copy_assign) {
				copy_assign(self, other);
			}
		}
	};
	
	template <typename T>
	void construct(byte* p) {
		new(p) T;
	}
	
	template <typename T>
	void copy_assign(byte* self, const byte* other) {
		*reinterpret_cast<T*>(self) = *reinterpret_cast<const T*>(other);
	}
	
	template <typename T>
	void copy_construct(byte* self, const byte* other) {
		new(self) T(*reinterpret_cast<const T*>(other));
	}
	
	template <typename T>
	void move_assign(byte* self, byte* other) {
		*reinterpret_cast<T*>(self) = move(*reinterpret_cast<T*>(other));
	}
	
	template <typename T>
	void move_construct(byte* self, byte* other) {
		new(self) T(move(*reinterpret_cast<T*>(other)));
	}
	
#define DEFINE_GET_FUNCTION_IF_SUPPORTED_BY_TYPE(CHECK_SUPPORT_STRUCT, FUNCTION_NAME) \
	template <typename T, bool IsSupported> struct GetFunctionIfSupportedImpl_ ## FUNCTION_NAME; \
	template <typename T> struct GetFunctionIfSupported_ ## FUNCTION_NAME { \
		static constexpr const auto Value = GetFunctionIfSupportedImpl_ ## FUNCTION_NAME <T, CHECK_SUPPORT_STRUCT<T>::value>::Value; \
	}; \
	template <typename T> struct GetFunctionIfSupportedImpl_ ## FUNCTION_NAME<T, true> { \
		static constexpr const auto Value = FUNCTION_NAME<T>; \
	}; \
	template <typename T> struct GetFunctionIfSupportedImpl_ ## FUNCTION_NAME<T, false> { \
		static constexpr const auto Value = nullptr; \
	}
#define GET_FUNCTION_IF_SUPPORTED(T, FUNCTION_NAME) \
	GetFunctionIfSupported_ ## FUNCTION_NAME<T>::Value
	
	DEFINE_GET_FUNCTION_IF_SUPPORTED_BY_TYPE(std::is_constructible, construct);
	DEFINE_GET_FUNCTION_IF_SUPPORTED_BY_TYPE(std::is_copy_constructible, copy_construct);
	DEFINE_GET_FUNCTION_IF_SUPPORTED_BY_TYPE(std::is_copy_assignable, copy_assign);
	DEFINE_GET_FUNCTION_IF_SUPPORTED_BY_TYPE(std::is_move_constructible, move_construct);
	DEFINE_GET_FUNCTION_IF_SUPPORTED_BY_TYPE(std::is_move_assignable, move_assign);
	
	template <typename T>
	struct DLL_PUBLIC GetTypeInfo {
		static constexpr const TypeInfo Value = {
			.internal_info  = typeid(T),
			.size           = sizeof(T),
			.alignment      = alignof(T),
			.construct      = GET_FUNCTION_IF_SUPPORTED(T, construct),
			.destruct       = falling::destruct<T>,
			.copy_construct = GET_FUNCTION_IF_SUPPORTED(T, copy_construct),
			.copy_assign    = GET_FUNCTION_IF_SUPPORTED(T, copy_assign),
			.move_construct = GET_FUNCTION_IF_SUPPORTED(T, move_construct),
			.move_assign    = GET_FUNCTION_IF_SUPPORTED(T, move_assign),
		};
	};
	
	template <typename T>
	constexpr const TypeInfo GetTypeInfo<T>::Value;
}

#endif
