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

#include <type_traits>

namespace falling {
	class Any {
	public:
		static const size_t Size = 24;
		static const size_t Alignment = 32;
		
		Any();
		template <typename T>
		Any(T value);
		Any(NothingType);
		Any(const Any& other);
		Any(Any&& other);
		~Any();
		
		template <typename T>
		Any& operator=(T value);
		Any& operator=(NothingType);
		Any& operator=(const Any& other);
		Any& operator=(Any&& other);
		
		bool operator==(const Any& other) const;
		bool operator!=(const Any& other) const;
		
		bool is_empty() const;
		explicit operator bool() const { return is_empty(); }
		
		template <typename T>
		void assign(T value);
		void assign(NothingType);
		void assign(const Any& other);
		void assign(Any&& other);
		
		const Type* type() const { return stored_type_; }
		void clear();
		
		template <typename T, typename Function>
		const Any& when(Function function) const;
		template <typename T, typename Function>
		Any& when(Function function);
		template <typename Function>
		void otherwise(Function function) const;
		template <typename T>
		Maybe<T> get() const;
	private:
		typedef typename std::aligned_storage<Size, Alignment>::type Storage;
		Storage memory_;
		const Type* stored_type_ = nullptr;
		
		void allocate_storage();
		void deallocate_storage();
		const byte* ptr() const;
		byte* ptr();
	};
	
	inline Any::Any() {
		clear();
	}
	
	template <typename T>
	Any::Any(T value) {
		assign(std::move(value));
	}
	
	inline Any::Any(NothingType) {
		clear();
	}
	
	inline Any::Any(const Any& other) {
		assign(other);
		allocate_storage();
		stored_type_->copy_construct(ptr(), other.ptr());
	}
	
	inline Any::Any(Any&& other) {
		assign(other);
		if (stored_type_ != nullptr) {
			allocate_storage();
			stored_type_->move_construct(ptr(), other.ptr());
			other.deallocate_storage();
			other.stored_type_ = nullptr;
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
	
	template <typename T>
	void Any::assign(T value) {
		clear();
		stored_type_ = get_type<T>();
		allocate_storage();
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
		}
	}
	
	inline void Any::assign(Any&& other) {
		clear();
		stored_type_ = other.stored_type_;
		if (stored_type_ != nullptr) {
			allocate_storage();
			stored_type_->move_construct(ptr(), other.ptr());
			other.clear();
		}
	}
	
	inline void Any::clear() {
		if (stored_type_ != nullptr) {
			stored_type_->destruct(ptr(), *(IUniverse*)nullptr); // TODO: Remove IUniverse requirement, since this is undefined behavior...
			deallocate_storage();
			stored_type_ = nullptr;
		}
	}
	
	template <typename T, typename Function>
	Any& Any::when(Function function) {
		if (get_type<T>() == stored_type_) {
			T* object = reinterpret_cast<T*>(ptr());
			function(*object);
		}
		return *this;
	}
	
	template <typename T, typename Function>
	const Any& Any::when(Function function) const {
		if (get_type<T>() == stored_type_) {
			const T* object = reinterpret_cast<const T*>(ptr());
			function(*object);
		}
		return *this;
	}
	
	template <typename Function>
	void Any::otherwise(Function function) const {
		if (is_empty()) {
			function();
		}
	}
	
	template <typename T>
	Maybe<T> Any::get() const {
		if (get_type<T>() == stored_type_) {
			return Maybe<T>(*reinterpret_cast<const T*>(ptr()));
		}
		return Nothing;
	}
}

#endif
