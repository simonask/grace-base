//
//  any.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 23/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_any_hpp
#define falling_any_hpp

#include "base/basic.hpp"
#include "type/type.hpp"
#include "base/maybe.hpp"
#include "memory/allocator.hpp"

#include <type_traits>

namespace falling {
	template <bool> struct AnyWhenImpl;
	
#if defined(DEBUG)
	struct IAnyDebugInfo {
		virtual ~IAnyDebugInfo() {}
		virtual IAnyDebugInfo* duplicate(IAllocator&, void* new_object) const = 0;
	};
	template <typename T>
	struct AnyDebugInfo : IAnyDebugInfo {
		T* object;
		IAnyDebugInfo* duplicate(IAllocator& alloc, void* new_object) const {
			AnyDebugInfo<T>* other = new(alloc) AnyDebugInfo<T>;
			other->object = reinterpret_cast<T*>(new_object);
			return other;
		}
	};
#endif
	

	class Any {
	public:
		static const size_t Size = 16;
		static const size_t Alignment = 16;
		
		Any();
		explicit Any(IAllocator& alloc);
		template <typename T>
		Any(T value, IAllocator& alloc = default_allocator());
		Any(NothingType, IAllocator& alloc = default_allocator());
		Any(Any&& other);
		Any(const Any& other);
		Any(Any&& other, IAllocator& alloc);
		Any(const Any& other, IAllocator& alloc);
		~Any();
		
		template <typename T>
		Any& operator=(T value);
		Any& operator=(NothingType);
		Any& operator=(const Any& other);
		Any& operator=(Any&& other);
		
		bool operator==(const Any& other) const;
		bool operator!=(const Any& other) const;
		
		IAllocator& allocator() const { return allocator_; }
		
		bool is_empty() const;
		explicit operator bool() const { return is_empty(); }
		
		template <typename T>
		void assign(T value);
		void assign(NothingType);
		void assign(const Any& other);
		void assign(Any&& other);
		
		const Type* type() const { return stored_type_; }
		template <typename T>
		bool is_a() const;
		void clear();
		
		template <typename T, typename Function>
		AnyWhenImpl<true> when(Function function) const;
		template <typename T, typename Function>
		AnyWhenImpl<false> when(Function function);
		template <typename Function>
		void otherwise(Function function) const;
		template <typename T>
		Maybe<T> get() const;
		template <typename T>
		T unsafe_get() const;
		
		
		const byte* ptr() const;
		byte* ptr();
		
		static Any take_memory(IAllocator&, const Type* type, void* memory);
	private:
		typedef typename std::aligned_storage<Size, Alignment>::type Storage;
		Storage memory_;
		const Type* stored_type_ = nullptr;
		IAllocator& allocator_;
		
#if defined(DEBUG)
		IAnyDebugInfo* debug_info_ = nullptr;
		void clear_debug_info() {
			destroy(debug_info_, allocator_);
			debug_info_ = nullptr;
		}
		template <typename T>
		void set_debug_info() {
			clear_debug_info();
			auto di = new(allocator_) AnyDebugInfo<T>;
			di->object = reinterpret_cast<T*>(ptr());
			debug_info_ = di;
		}
#else
		void clear_debug_info() {}
		template <typename T>
		void set_debug_info() {}
#endif
		
		void allocate_storage();
		void deallocate_storage();
		
		friend struct AnyType;
	};
	
	struct AnyType : TypeFor<Any> {
		void deserialize(Any& place, const ArchiveNode& n, IUniverse& u) const;
		void serialize(const Any& place, ArchiveNode&, IUniverse&) const;
		StringRef name() const { return "Any"; }
		size_t size() const { return sizeof(Any); }
	};
	
	template <>
	struct BuildTypeInfo<Any> {
		static const AnyType* build();
	};
	
	inline Any::Any() : allocator_(default_allocator()) {
		clear();
	}
	
	inline Any::Any(IAllocator& alloc) : allocator_(alloc) {
		clear();
	}
	
	template <typename T>
	Any::Any(T value, IAllocator& alloc) : allocator_(alloc) {
		assign(std::move(value));
	}
	
	inline Any::Any(NothingType, IAllocator& alloc) : allocator_(alloc) {
		clear();
	}
	
	// TODO: Use delegating constructor when available.
	inline Any::Any(const Any& other) : allocator_(other.allocator_) {
		assign(other);
		allocate_storage();
		stored_type_->copy_construct(ptr(), other.ptr());
	}
	
	// TODO: Use delegating constructor when available.
	inline Any::Any(Any&& other) : allocator_(other.allocator_) {
		stored_type_ = other.stored_type_;
		if (stored_type_ != nullptr) {
			allocate_storage(); // TODO: Take pointer from other.
			stored_type_->move_construct(ptr(), other.ptr());
			other.deallocate_storage();
			other.stored_type_ = nullptr;
#if defined(DEBUG)
			debug_info_ = other.debug_info_;
			other.debug_info_ = nullptr;
#endif
		}
	}
	
	inline Any::Any(const Any& other, IAllocator& alloc) : allocator_(alloc) {
		assign(other);
	}
	
	inline Any::Any(Any&& other, IAllocator& alloc) : allocator_(alloc) {
		stored_type_ = other.stored_type_;
		if (stored_type_ != nullptr) {
			allocate_storage(); // TODO: Check if alloc is same, and take pointer if so.
			stored_type_->move_construct(ptr(), other.ptr());
			other.deallocate_storage();
			other.stored_type_ = nullptr;
#if defined(DEBUG)
			debug_info_ = other.debug_info_;
			other.debug_info_ = nullptr;
#endif
		}
	}
	
	inline Any::~Any() {
		clear();
	}
	
	template <typename T>
	Any& Any::operator=(T value) {
		assign(std::move(value));
		return *this;
	}
	
	inline Any& Any::operator=(NothingType) {
		clear();
		return *this;
	}
	
	inline Any& Any::operator=(const Any& other) {
		assign(other);
		return *this;
	}
	
	inline Any& Any::operator=(Any&& other) {
		assign(std::move(other));
		return *this;
	}
	
	inline bool Any::is_empty() const {
		return stored_type_ == nullptr;
	}
	
	template <>
	inline bool Any::is_a<Any>() const {
		return true;
	}
	
	template <typename T>
	inline bool Any::is_a() const {
		return stored_type_ == get_type<T>();
	}
	
	template <>
	inline void Any::assign<Any>(Any value) {
		this->operator=(move(value));
	}
	
	template <typename T>
	void Any::assign(T value) {
		clear();
		stored_type_ = get_type<T>();
		allocate_storage();
		set_debug_info<T>();
		stored_type_->move_construct(ptr(), reinterpret_cast<byte*>(&value));
	}
	
	inline void Any::assign(NothingType) {
		clear();
	}
	
	inline void Any::assign(const Any& other) {
		clear();
		stored_type_ = other.stored_type_;
		if (stored_type_ != nullptr) {
			allocate_storage();
			stored_type_->copy_construct(ptr(), other.ptr());
#if defined(DEBUG)
			debug_info_ = other.debug_info_->duplicate(allocator_, ptr());
#endif
		}
	}
	
	inline void Any::assign(Any&& other) {
		clear();
		stored_type_ = other.stored_type_;
		if (stored_type_ != nullptr) {
			allocate_storage();
			stored_type_->move_construct(ptr(), other.ptr());
#if defined(DEBUG)
			debug_info_ = other.debug_info_;
			other.debug_info_ = nullptr;
#endif
			other.clear();
		}
	}
	
	inline void Any::clear() {
		if (stored_type_ != nullptr) {
			stored_type_->destruct(ptr(), *(IUniverse*)nullptr); // TODO: Remove IUniverse requirement, since this is undefined behavior...
			deallocate_storage();
			stored_type_ = nullptr;
			clear_debug_info();
		}
	}
	
	template <typename T> struct AnyWhenFunctionCaller;
	template <> struct AnyWhenFunctionCaller<Any> {
		template <typename Function>
		static bool call(Any& any, Function f) {
			f(any);
			return true;
		}
		template <typename Function>
		static bool call(const Any& any, Function f) {
			f(any);
			return true;
		}
	};
	template <> struct AnyWhenFunctionCaller<NothingType> {
		template <typename Function>
		static bool call(Any& any, Function f) {
			if (any.is_empty()) {
				f();
				return true;
			}
			return false;
		}
		template <typename Function>
		static bool call(const Any& any, Function f) {
			if (any.is_empty()) {
				f();
				return true;
			}
			return false;
		}
	};
	template <typename T> struct AnyWhenFunctionCaller {
		template <typename Function>
		static bool call(Any& any, Function f) {
			const Type* t = get_type<typename RemoveConstRef<T>::Type>();
			if (any.type() == t) {
				T& v = *reinterpret_cast<typename RemoveConstRef<T>::Type*>(any.ptr());
				f(v);
				return true;
			}
			return false;
		}
		template <typename Function>
		static bool call(const Any& any, Function f) {
			const Type* t = get_type<typename RemoveConstRef<T>::Type>();
			if (any.type() == t) {
				const T& v = *reinterpret_cast<const typename RemoveConstRef<T>::Type*>(any.ptr());
				f(v);
				return true;
			}
			return false;
		}
	};
	
	template <bool IsConst>
	struct AnyWhenImpl {
		using Self = AnyWhenImpl<IsConst>;
		using AnyType = typename std::conditional<IsConst, const Any, Any>::type;
		using AnyTypeRef = AnyType&;
		
		template <typename T, typename Function>
		Self when(Function function) {
			if (was_handled) return *this;
			was_handled = AnyWhenFunctionCaller<T>::call(any, std::forward<Function>(function));
			return *this;
		}
		
		template <typename Function>
		void otherwise(Function f) {
			if (!was_handled) {
				f();
			}
		}
	private:
		AnyTypeRef any;
		bool was_handled = false;
		friend class Any;
		AnyWhenImpl(AnyTypeRef ref) : any(ref) {}
		AnyWhenImpl(const AnyWhenImpl<IsConst>& other) = default;
	};
	
	template <typename T, typename Function>
	AnyWhenImpl<false> Any::when(Function function) {
		return AnyWhenImpl<false>(*this).when<T>(std::forward<Function>(function));
	}
	
	template <typename T, typename Function>
	AnyWhenImpl<true> Any::when(Function function) const {
		return AnyWhenImpl<true>(*this).when<T>(std::forward<Function>(function));
	}
	
	template <typename Function>
	void Any::otherwise(Function function) const {
		if (is_empty()) {
			function();
		}
	}
	
	template <>
	inline Maybe<Any> Any::get<Any>() const {
		return *this;
	}
	
	template <typename T>
	Maybe<T> Any::get() const {
		if ((const Type*)get_type<T>() == stored_type_) {
			return Maybe<T>(*reinterpret_cast<const T*>(ptr()));
		}
		return Nothing;
	}
	
	template <typename T>
	T Any::unsafe_get() const {
		ASSERT((const Type*)get_type<T>() != (const Type*)get_type<Any>());
		ASSERT(get_type<T>() == stored_type_);
		return *reinterpret_cast<const T*>(ptr());
	}
	
	inline const byte* Any::ptr() const {
		if (stored_type_ != nullptr) {
			if (stored_type_->size() > Size) {
				byte const* const* ptrptr = reinterpret_cast<byte const* const*>(&memory_);
				return *ptrptr;
			} else {
				return reinterpret_cast<byte const*>(&memory_);
			}
		}
		return nullptr;
	}
	
	inline byte* Any::ptr() {
		if (stored_type_ != nullptr) {
			if (stored_type_->size() > Size) {
				byte** ptrptr = reinterpret_cast<byte**>(&memory_);
				return *ptrptr;
			} else {
				return reinterpret_cast<byte*>(&memory_);
			}
		}
		return nullptr;
	}
	
	inline void Any::allocate_storage() {
		ASSERT(stored_type_ != nullptr);
		size_t sz = stored_type_->size();
		if (sz > Size) {
			byte* memory = (byte*)allocator_.allocate(sz, stored_type_->alignment()); // TODO: Type::alignment support?
			*reinterpret_cast<byte**>(&memory_) = memory;
		} else {
			ASSERT(stored_type_->alignment() <= Alignment);
		}
	}
	
	inline void Any::deallocate_storage() {
		ASSERT(stored_type_ != nullptr);
		size_t sz = stored_type_->size();
		if (sz > Size) {
			byte* memory = *reinterpret_cast<byte**>(&memory_);
			allocator_.free(memory, sz);
		}
	}
}

#endif
