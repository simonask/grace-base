//
//  function.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 21/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_function_hpp
#define falling_function_hpp

#include "base/basic.hpp"
#include "base/maybe.hpp"
#include "memory/allocator.hpp"
#include <type_traits>

namespace falling {
	template <typename Signature>
	class Function;
	
	template <typename R, typename... Args>
	class Function<R(Args...)> {
	public:
		using FunctionPointerType = R(*)(Args...);
		
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
		
		template <typename Result = R>
		typename std::enable_if<std::is_default_constructible<Result>::value, Result>::type
		operator()(Args... args) const {
			if (!this->is_empty()) {
				return invoke(std::forward<Args>(args)...);
			}
			return R();
		}
		
		template <typename Result = R>
		typename std::enable_if<!std::is_same<void, Result>::value && !std::is_default_constructible<Result>::value, Maybe<Result>>::type
		operator()(Args... args) const {
			if (!is_empty()) {
				return invoke(std::forward<Args>(args)...);
			}
			return Nothing;
		}
		
		template <typename Result = R>
		typename std::enable_if<std::is_same<void, Result>::value, void>::type
		operator()(Args... args) const {
			if (!is_empty()) {
				invoke(std::forward<Args>(args)...);
			}
		}
		
		void clear() {
			if (get_payload_type() == VirtualInvokerTag) {
				destroy(get_virtual_invoker(), allocator_);
			}
			payload_ = 0;
		}
		
		bool is_empty() const { return payload_ == 0; }
		explicit operator bool() const { return !is_empty(); }
		bool is_function_pointer() const {
			return get_payload_type() == FunctionPointerTag;
		}
		bool is_virtual_invoker() const {
			return get_payload_type() == VirtualInvokerTag;
		}
	private:
		enum PointerTags {
			FunctionPointerTag       = 0x0,
			VirtualInvokerTag        = 0x1,
			PointerTagMask           = 0x1
		};
	
		struct ICaller {
			virtual ~ICaller() {}
			virtual R invoke(Args... args) = 0;
			virtual ICaller* duplicate(IAllocator& alloc) const = 0;
		};
		template <typename T>
		struct ALIGNED(8) Caller : ICaller  {
			Caller(T value) : value(move(value)) {}
			T value;
			R invoke(Args... args) final {
				return value(std::forward<Args>(args)...);
			}
			ICaller* duplicate(IAllocator& alloc) const {
				return new(alloc, alignof(Caller<T>)) Caller<T>{value};
			}
		};
		
		uintptr_t payload_ = 0;
		IAllocator& allocator_;
		
		PointerTags get_payload_type() const {
			return (PointerTags)(payload_ & PointerTagMask);
		}
		uintptr_t get_payload() const {
			return payload_ & ~PointerTagMask;
		}
		void set_virtual_invoker(ICaller* caller) {
			payload_ = (uintptr_t)caller | VirtualInvokerTag;
		}
		void set_function_pointer(FunctionPointerType function_pointer) {
			payload_ = (uintptr_t)function_pointer | FunctionPointerTag;
		}
		
		ICaller* get_virtual_invoker() const {
			ASSERT(get_payload_type() == VirtualInvokerTag);
			return (ICaller*)get_payload();
		}
		
		FunctionPointerType get_function_pointer() const {
			ASSERT(get_payload_type() == FunctionPointerTag);
			return (R(*)(Args...))get_payload();
		}
		
		void assign(const Function<R(Args...)>& other) {
			clear();
			if (!other.is_empty()) {
				if (other.is_function_pointer()) {
					payload_ = other.payload_;
				} else {
					set_virtual_invoker(other.get_virtual_invoker()->duplicate(allocator_));
				}
			}
		}
		void assign(Function<R(Args...)>&& other) {
			clear();
			if (other.is_function_pointer() || &allocator_ == &other.allocator_) {
				payload_ = other.payload_;
				other.payload_ = 0;
			} else {
				set_virtual_invoker(other.get_virtual_invoker()->duplicate(allocator_));
				other.clear();
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
		
		R invoke(Args... args) const {
			uintptr_t payload = get_payload();
			switch (get_payload_type()) {
				case FunctionPointerTag: {
					return ((FunctionPointerType)payload)(std::forward<Args>(args)...);
				}
				case VirtualInvokerTag: {
					return ((ICaller*)payload)->invoke(std::forward<Args>(args)...);
				}
				default: ASSERT(false); 
			}
		}
	};
	
	template <typename T, typename R, typename... Args>
	Function<R(Args...)> bind_method(T* self, R(T::*method)(Args...)) {
		auto f = [=](Args... args) {
			(self->*method)(std::forward<Args>(args)...);
		};
		return Function<R(Args...)>(move(f));
	}
	
	template <typename T, typename R, typename... Args>
	Function<R(Args...)> bind_method(const T* self, R(T::*method)(Args...) const) {
		auto f = [=](Args... args) {
			(self->*method)(std::forward<Args>(args)...);
		};
		return Function<R(Args...)>(move(f));
	}
}

#endif
