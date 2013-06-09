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
		
		explicit Function(IAllocator& alloc = default_allocator()) : allocator_(alloc) {}
		Function(FunctionPointerType ptr, IAllocator& alloc = default_allocator()) : allocator_(alloc) { assign(ptr); }
		Function(const Function<R(Args...)>& other) : allocator_(other.allocator_) { assign(other); }
		Function(const Function<R(Args...)>& other, IAllocator& alloc) : allocator_(alloc) { assign(other); }
		Function(Function<R(Args...)>&& other) : allocator_(other.allocator_) { assign(move(other)); }
		Function(Function<R(Args...)>&& other, IAllocator& alloc) : allocator_(alloc) { assign(move(other)); }
		template <typename T>
		Function(T function_object, IAllocator& alloc = default_allocator()) : allocator_(alloc) { assign(move(function_object)); }
		~Function() { this->clear(); }
		Function<R(Args...)>& operator=(FunctionPointerType funptr) { assign(funptr); return *this; }
		Function<R(Args...)>& operator=(const Function<R(Args...)>& other) { assign(other); return *this; }
		Function<R(Args...)>& operator=(Function<R(Args...)>&& other) { assign(move(other)); return *this; }
		Function<R(Args...)>& operator=(NothingType) { this->clear(); return *this; }
		template <typename T>
		Function<R(Args...)>& operator=(T function_object) { assign(move(function_object)); return *this; }
		
		R operator()(Args... args) const {
			switch (type_) {
				case InvokerType::Empty: return empty_invoker(std::forward<Args>(args)...);
				case InvokerType::FunctionPointer: return function_pointer_invoker(std::forward<Args>(args)...);
				case InvokerType::VirtualInvokerHeap: return virtual_invoker(std::forward<Args>(args)...);
			}
		}
		
		void clear() {
			if (is_virtual_invoker()) {
				destroy(payload.vptr, allocator_);
			}
			payload.fptr = nullptr;
			type_ = InvokerType::Empty;
		}
		
		bool is_empty() const {
			return type_ == InvokerType::Empty;
		}
		explicit operator bool() const { return !is_empty(); }
		bool is_function_pointer() const {
			return type_ == InvokerType::FunctionPointer;
		}
		bool is_virtual_invoker() const {
			return type_ == InvokerType::VirtualInvokerHeap;
		}
	private:
		enum class InvokerType : uint8 {
			Empty,
			FunctionPointer,
			VirtualInvokerHeap,
			// TODO: Add type with "small function optimization"
		};
	
		struct ICaller {
			virtual ~ICaller() {}
			virtual R invoke(Args... args) = 0;
			virtual ICaller* duplicate(IAllocator& alloc) const = 0;
		};
		template <typename T>
		struct Caller : ICaller  {
			Caller(T value) : value(move(value)) {}
			T value;
			R invoke(Args... args) final {
				return value(std::forward<Args>(args)...);
			}
			ICaller* duplicate(IAllocator& alloc) const {
				return new(alloc, alignof(Caller<T>)) Caller<T>{value};
			}
		};
		
		// A member function pointer to one of our own methods, that will be called without any conditionals upon invocation.
		using InvokerFunction = R(*)(const Self*, Args...);
		union {
			FunctionPointerType fptr;
			ICaller* vptr;
		} payload = {nullptr};
		IAllocator& allocator_;
		InvokerType type_ = InvokerType::Empty;

		
		R empty_invoker(Args... args) const {
			throw EmptyFunctionCallError();
		}
		R function_pointer_invoker(Args... args) const {
			return payload.fptr(std::forward<Args>(args)...);
		}
		R virtual_invoker(Args... args) const {
			return payload.vptr->invoke(std::forward<Args>(args)...);
		}
		
		void set_virtual_invoker(ICaller* caller) {
			payload.vptr = caller;
			type_ = InvokerType::VirtualInvokerHeap;
		}
		void set_function_pointer(FunctionPointerType function_pointer) {
			payload.fptr = function_pointer;
			type_ = InvokerType::FunctionPointer;
		}
		
		void assign(const Function<R(Args...)>& other) {
			if (&other == this) return;
			clear();
			type_ = other.type_;
			if (is_virtual_invoker()) {
				payload.vptr = other.payload.vptr->duplicate(allocator_);
			} else {
				payload.fptr = other.payload.fptr;
			}
		}
		void assign(Function<R(Args...)>&& other) {
			if (&other == this) return;
			clear();
			type_ = other.type_;
			if (is_virtual_invoker()) {
				if (&allocator_ == &other.allocator_) {
					payload.vptr = other.payload.vptr;
					other.payload.vptr = nullptr;
				} else {
					payload.vptr = other.payload.vptr->duplicate(allocator_);
					other.clear();
				}
			} else {
				payload.fptr = other.payload.fptr;
				other.payload.fptr = nullptr;
			}
		}
		void assign(FunctionPointerType ptr) {
			clear();
			set_function_pointer(ptr);
		}
		
		template <typename T>
		void assign(T value) {
			clear();
			set_virtual_invoker(new(allocator_, alignof(Caller<T>)) Caller<T>{move(value)});
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
