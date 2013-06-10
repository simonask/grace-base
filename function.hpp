//
//  function.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 21/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_function_hpp
#define grace_function_hpp

#include "base/basic.hpp"
#include "base/maybe.hpp"
#include "memory/allocator.hpp"
#include "base/string.hpp"
#include "base/either.hpp"

#include <type_traits>

namespace grace {
	template <typename Signature>
	class Function;
	
	struct EmptyFunctionCallError {
		StringRef what() const { return "Called empty function object."; }
	};
	
	template <typename R, typename... Args>
	class Function<R(Args...)> {
	public:
		using FunctionPointerType = R(*)(Args...);
		using Self = Function<R(Args...)>;
		
		Function() {}
		Function(FunctionPointerType ptr) { assign(ptr); }
		Function(const Function<R(Args...)>& other) { assign(other); }
		Function(const Function<R(Args...)>& other, IAllocator& alloc) { assign(other, alloc); }
		Function(Function<R(Args...)>&& other) { assign(move(other)); }
		Function(Function<R(Args...)>&& other, IAllocator& alloc) { assign(move(other), alloc); }
		template <typename T>
		Function(T function_object, IAllocator& alloc = default_allocator()) { assign(move(function_object), alloc); }
		~Function() { this->clear(); }
		Function<R(Args...)>& operator=(const Function<R(Args...)>& other) { assign(other); return *this; }
		Function<R(Args...)>& operator=(Function<R(Args...)>&& other) { assign(move(other)); return *this; }
		Function<R(Args...)>& operator=(NothingType) { this->clear(); return *this; }
		template <typename T>
		Function<R(Args...)>& operator=(T function_object) { assign(move(function_object)); return *this; }
		
		template <typename... A>
		ALWAYS_INLINE R invoke(A&&... args) const {
			return (this->*invoke_)(std::forward<A>(args)...);
		}
		
		template <typename... A>
		ALWAYS_INLINE R operator()(A&&... args) const {
			return invoke(std::forward<A>(args)...);
		}
		
		explicit operator bool() const {
			return type_ == InvokerType::Empty;
		}
	private:
		using InvokerPad = R(Function<R(Args...)>::*)(Args...) const;
		
		R empty_invoker_pad(Args...) const {
			throw EmptyFunctionCallError();
		}
	
		struct CallerBase {
			CallerBase(IAllocator& alloc) : allocator(alloc) {}
			virtual ~CallerBase() {}
			virtual R invoke(Args&&...) = 0;
			virtual CallerBase* duplicate(IAllocator& alloc) const = 0;
			IAllocator& allocator;
		};
		
		template <typename T>
		struct Caller : CallerBase {
			T object;
			Caller(const T& object, IAllocator& alloc) : CallerBase(alloc), object(object) {}
			Caller(T&& object, IAllocator& alloc) : CallerBase(alloc), object(move(object)) {}
			ALWAYS_INLINE R invoke(Args&&... args) final {
				return object(std::forward<Args>(args)...);
			}
			CallerBase* duplicate(IAllocator& alloc) const {
				return new(alloc) Caller<T>(object, alloc);
			}
		};
		
		enum class InvokerType : uint8 {
			Empty,
			Thin, // Just a function pointer (overhead: two indirect jumps).
			Fat,  // A full-featured caller  (overhead: one indirect jump, one virtual call, memory allocation).
		};
		union {
			FunctionPointerType fptr;
			CallerBase* cptr;
		} payload = {nullptr};
		InvokerPad invoke_ = &Function<R(Args...)>::empty_invoker_pad;
		InvokerType type_ = InvokerType::Empty;
		
		R fptr_invoker_pad(Args... args) const {
			return payload.fptr(std::forward<Args>(args)...);
		}
		
		R cptr_invoker_pad(Args... args) const {
			return payload.cptr->invoke(std::forward<Args>(args)...);
		}
		
		void assign_fptr(FunctionPointerType fptr) {
			type_ = InvokerType::Thin;
			payload.fptr = fptr;
			invoke_ = &Function<R(Args...)>::fptr_invoker_pad;
		}
		
		void assign_cptr_impl(CallerBase* caller) {
			type_ = InvokerType::Fat;
			payload.cptr = caller;
			invoke_ = &Function<R(Args...)>::cptr_invoker_pad;
		}
		
		template <typename T>
		void assign_cptr(T&& function_object, IAllocator& alloc) {
			assign_cptr_impl(new(alloc) Caller<T>(move(function_object), alloc));
		}
		
		void clear() {
			if (type_ == InvokerType::Fat && payload.cptr) {
				destroy(payload.cptr, payload.cptr->allocator);
				payload.cptr = nullptr;
			} else if (type_ == InvokerType::Thin) {
				payload.fptr = nullptr;
			}
			type_ = InvokerType::Empty;
			invoke_ = &Function<R(Args...)>::empty_invoker_pad;
		}
	
		template <typename T>
		typename std::enable_if<std::is_convertible<T, FunctionPointerType>::value, void>::type
		assign(const T& function_object) {
			clear();
			assign_fptr((FunctionPointerType)function_object);
		}
		
		template <typename T>
		typename std::enable_if<std::is_convertible<T, FunctionPointerType>::value, void>::type
		assign(const T& function_object, IAllocator&) {
			clear();
			assign_fptr((FunctionPointerType)function_object);
		}
		
		template <typename T>
		typename std::enable_if<!std::is_convertible<T, FunctionPointerType>::value, void>::type
		assign(T&& function_object, IAllocator& alloc) {
			clear();
			assign_cptr(move(function_object), alloc);
		}
		
		void assign(const Function<R(Args...)>& other) {
			if (&other == this) return;
			clear();
			type_ = other.type_;
			
			switch (type_) {
				case InvokerType::Fat: {
					assign_cptr_impl(other.payload.cptr->duplicate(other.payload.cptr->allocator));
					return;
				}
				case InvokerType::Thin: {
					assign_fptr(other.payload.fptr);
					return;
				}
				default: return;
			}
		}
		
		void assign(const Function<R(Args...)>& other, IAllocator& alloc) {
			if (&other == this) return;
			clear();
			type_ = other.type_;
			
			switch (type_) {
				case InvokerType::Fat: {
					assign_cptr_impl(other.payload.cptr->duplicate(alloc));
					return;
				}
				case InvokerType::Thin: {
					assign_fptr(other.payload.fptr);
					return;
				}
				default: return;
			}
		}
		
		void assign(Function<R(Args...)>&& other) {
			if (&other == this) return;
			clear();
			type_ = other.type_;
			invoke_ = other.invoke_;
			
			switch (type_) {
				case InvokerType::Fat: {
					payload.cptr = other.payload.cptr;
					other.payload.cptr = nullptr;
					break;
				}
				case InvokerType::Thin: {
					payload.fptr = other.payload.fptr;
					other.payload.fptr = nullptr;
					break;
				}
				default: break;
			}
			other.type_ = InvokerType::Empty;
			other.invoke_ = &Function<R(Args...)>::empty_invoker_pad;
		}
		
		void assign(Function<R(Args...)>&& other, IAllocator& alloc) {
			if (&other == this) return;
			clear();
			type_ = other.type_;
			invoke_ = other.invoke_;
			
			switch (type_) {
				case InvokerType::Fat: {
					if (&alloc == &other.payload.cptr->allocator) {
						payload.cptr = other.payload.cptr;
						other.payload.cptr = nullptr;
						other.invoke_ = &Function<R(Args...)>::empty_invoker_pad;
						other.payload.cptr = nullptr;
					} else {
						assign_cptr_impl(other.payload.cptr->duplicate(alloc));
						other.clear();
					}
					break;
				}
				case InvokerType::Thin: {
					payload.fptr = other.payload.fptr;
					other.type_ = InvokerType::Empty;
					other.invoke_ = &Function<R(Args...)>::empty_invoker_pad;
					other.payload.fptr = nullptr;
					break;
				}
				default: break;
			}
		}
		
	};
	
	template <typename T, typename R, typename... Args>
	Function<R(Args...)> bind_method(T* self, R(T::*method)(Args...)) {
		auto f = [=](Args... args) {
			return (self->*method)(std::forward<Args>(args)...);
		};
		return Function<R(Args...)>(move(f));
	}
	
	template <typename T, typename R, typename... Args>
	Function<R(Args...)> bind_method(const T* self, R(T::*method)(Args...) const) {
		auto f = [=](Args... args) {
			return (self->*method)(std::forward<Args>(args)...);
		};
		return Function<R(Args...)>(move(f));
	}
}

#endif
