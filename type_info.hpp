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
	struct TypeInfo;

	struct UnsupportedTypeOperationError {
		const TypeInfo& type;
		const char* operation;
		const char* what() const { return operation; }
	};

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
		
		const ConstructorFuncPtr   construct_;
		const DestructorFuncPtr    destruct_;
		const CopyAssignFuncPtr    copy_assign_;
		const CopyConstructFuncPtr copy_construct_;
		const MoveAssignFuncPtr    move_assign_;
		const MoveConstructFuncPtr move_construct_;
		
		bool is_constructible() const { return construct_ != nullptr; }
		bool is_copy_assignable() const         { return copy_assign_ != nullptr; }
		bool is_copy_constructible() const      { return copy_construct_ != nullptr; }
		bool is_move_assignable() const         { return move_assign_ != nullptr; }
		bool is_move_constructible() const      { return move_construct_ != nullptr; }
		bool is_move_or_copy_constructible() const { return is_move_constructible() || is_copy_constructible(); }
		bool is_move_or_copy_assignable() const    { return is_move_assignable() || is_copy_assignable(); }
		bool is_abstract() const { return !is_constructible() && !is_move_or_copy_constructible(); }
		
		void construct(byte* self) const {
			construct_ ? construct_(self) : unsupported("construct");
		}
		void destruct(byte* self) const {
			destruct_ ? destruct_(self) : unsupported("destruct");
		}
		void copy_assign(byte* self, const byte* other) const {
			copy_assign_ ? copy_assign_(self, other) : unsupported("copy_assign");
		}
		void copy_construct(byte* self, const byte* other) const {
			copy_construct_ ? copy_construct_(self, other) : unsupported("copy_construct");
		}
		void move_assign(byte* self, byte* other) const {
			move_assign_ ? move_assign_(self, other) : unsupported("move_assign");
		}
		void move_construct(byte* self, byte* other) const {
			move_construct_ ? move_construct_(self, other) : unsupported("move_construct");
		}
		
		void move_or_copy_construct(byte* self, byte* other) const {
			if (move_construct_) {
				move_construct(self, other);
			} else {
				copy_construct(self, other);
			}
		}
		void move_or_copy_assign(byte* self, byte* other) const {
			if (move_assign_) {
				move_assign(self, other);
			} else {
				copy_assign(self, other);
			}
		}
		
	private:
		void unsupported(const char* op) const {
			throw UnsupportedTypeOperationError{*this, op};
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
			.construct_     = GET_FUNCTION_IF_SUPPORTED(T, construct),
			.destruct_      = falling::destruct<T>,
			.copy_construct_= GET_FUNCTION_IF_SUPPORTED(T, copy_construct),
			.copy_assign_   = GET_FUNCTION_IF_SUPPORTED(T, copy_assign),
			.move_construct_= GET_FUNCTION_IF_SUPPORTED(T, move_construct),
			.move_assign_   = GET_FUNCTION_IF_SUPPORTED(T, move_assign),
		};
	};
	
	template <>
	struct DLL_PUBLIC GetTypeInfo<void> {
		static constexpr const TypeInfo Value = {
			.internal_info = typeid(void),
			.size = 0,
			.alignment = 0,
			.construct_ = nullptr,
			.destruct_ = nullptr,
			.copy_construct_ = nullptr,
			.copy_assign_ = nullptr,
			.move_construct_ = nullptr,
			.move_assign_ = nullptr,
		};
	};
	
	template <typename T>
	constexpr const TypeInfo GetTypeInfo<T>::Value;
}

#endif
