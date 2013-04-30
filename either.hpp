//
//  either.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 28/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_either_hpp
#define falling_either_hpp

#include "base/basic.hpp"
#include "base/type_info.hpp"

namespace falling {
	template <typename A, typename B>
	constexpr auto max(A a, B b) -> typename std::common_type<A, B>::type {
		return a > b ? a : b;
	}

	template <typename... Types> struct MaxSize;
	template <> struct MaxSize<> {
		static const size_t Value = 0;
	};
	template <typename T, typename... Rest> struct MaxSize<T, Rest...> {
		static const size_t Value = max(sizeof(T), MaxSize<Rest...>::Value);
	};
	template <typename... Types> struct MaxAlignment;
	template <> struct MaxAlignment<> {
		static const size_t Value = 0;
	};
	template <typename T, typename... Rest> struct MaxAlignment<T, Rest...> {
		static const size_t Value = max(alignof(T), MaxAlignment<Rest...>::Value);
	};
	
	template <typename... Types>
	struct AlignedUnion {
		using Type = typename std::aligned_storage<MaxSize<Types...>::Value, MaxAlignment<Types...>::Value>::type;
	};
	
	template <typename T, size_t I, typename... Types> struct IndexOfTypeImpl;
	template <typename T, size_t I, typename Current, typename... Rest> struct IndexOfTypeImpl<T, I, Current, Rest...> {
		static const size_t Value = std::is_same<T, Current>::value ? I : IndexOfTypeImpl<T, I+1, Rest...>::Value;
	};
	template <typename T, size_t I> struct IndexOfTypeImpl<T, I> {
		static const size_t Value = SIZE_T_MAX;
	};
	template <typename T, typename... Types> struct IndexOfType {
		static const size_t Value = IndexOfTypeImpl<T, 0, Types...>::Value;
	};
	
	template <typename... Types> // max 255 types!
	class Either {
	public:
		static_assert(sizeof...(Types) < UINT8_MAX, "Too many possible types for Either.");
	
		using Self = Either<Types...>;
		
		template <typename T>
		explicit Either(T value);
		Either(const Self& other);
		Either(Self&& other);
		~Either();
		template <typename T>
		Self& operator=(T value);
		Self& operator=(const Self& other);
		Self& operator=(Self&& other);
		
		template <typename T>
		typename std::enable_if<IndexOfType<T, Types...>::Value < sizeof...(Types), bool>::type
		is_a() const {
			return type_index_ == IndexOfType<T, Types...>::Value;
		}
		
		bool is_same_type_as(const Self& other) const;
		
		bool is_nothing() const {
			return is_a<NothingType>();
		}
		
		template <typename T = NothingType>
		explicit operator bool() const {
			return !is_a<T>();
		}
		
		template <bool IsConst>
		struct WhenControlFlow {
			using Self = typename std::conditional<IsConst, const Either<Types...>, Either<Types...>>::type;
			WhenControlFlow(Self& either) : either_(either) {}
			Self& either_;
			bool handled_ = false;
			
			template <typename T, typename F>
			WhenControlFlow<IsConst>& when(F f) {
				if (either_.template is_a<T>()) {
					handled_ = true;
					f(*reinterpret_cast<T*>(&either_.memory_));
				}
				return *this;
			}
			
			template <typename F>
			void otherwise(F f) {
				if (!handled_) {
					f();
				}
			}
		};
		
		template <typename T, typename F>
		WhenControlFlow<false> when(F f) {
			return WhenControlFlow<false>(*this).template when<T>(move(f));
		}
		template <typename T, typename F>
		WhenControlFlow<true> when(F f) const {
			return WhenControlFlow<true>(*this).template when<T>(move(f));
		}
		
		void swap(Self& other);
		
		const TypeInfo& type_info() const {
			return *type_infos_[type_index_];
		}
	private:
		template <bool> friend struct WhenControlFlow;
		using Storage = typename AlignedUnion<Types...>::Type;
		Storage memory_;
		uint8 type_index_;
		static constexpr const TypeInfo* const type_infos_[sizeof...(Types)] = {&GetTypeInfo<Types>::Value...};
		
		void assign_move(Self&& other);
		void assign_copy(const Self& other);
		template <typename T>
		void assign_move(T&& other);
		template <typename T>
		void assign_copy(const T& other);
	};
	
	template <typename... Types>
	constexpr const TypeInfo* const Either<Types...>::type_infos_[sizeof...(Types)];
	
	template <typename... Types>
	template <typename T>
	Either<Types...>::Either(T value) {
		static const auto I = IndexOfType<T, Types...>::Value;
		static_assert(I < sizeof...(Types), "T is not a valid type for this Either.");
		type_index_ = I;
		type_info().move_or_copy_construct((byte*)&memory_, (byte*)&value);
	}
	
	template <typename... Types>
	Either<Types...>::Either(const Self& other) {
		assign_copy(other);
	}
	
	template <typename... Types>
	Either<Types...>::Either(Self&& other) {
		assign_move(move(other));
	}
	
	template <typename... Types>
	Either<Types...>::~Either() {
		type_info().destruct((byte*)&memory_);
	}
	
	template <typename... Types>
	template <typename T>
	Either<Types...>& Either<Types...>::operator=(T value) {
		assign_move(move(value));
		return *this;
	}
	
	template <typename... Types>
	Either<Types...>& Either<Types...>::operator=(const Self& other) {
		assign_copy(other);
		return *this;
	}
	
	template <typename... Types>
	Either<Types...>& Either<Types...>::operator=(Self&& other) {
		assign_move(move(other));
		return *this;
	}
	
	template <typename... Types>
	void Either<Types...>::swap(Self& other) {
		Storage tmp_;
		if (is_same_type_as(other)) {
			type_info().move_or_copy_construct((byte*)&tmp_, (byte*)&other.memory_);
			type_info().move_or_copy_assing((byte*)&other.memory_, (byte*)&memory_);
			type_info().move_or_copy_assign((byte*)memory_, (byte*)&tmp_);
			type_info().destruct((byte*)&tmp_);
		} else {
			other.type_info().move_or_copy_construct((byte*)&tmp_, (byte*)&other.memory_);
			other.type_info().destruct((byte*)&other.memory_);
			type_info().move_or_copy_construct((byte*)&other.memory_, (byte*)&memory_);
			type_info().destruct((byte*)&memory_);
			other.type_info().move_or_copy_construct((byte*)&memory_, (byte*)&tmp_);
			std::swap(type_index_, other.type_index_);
		}
	}
	
	template <typename... Types>
	void Either<Types...>::assign_move(Self&& other) {
		swap(other);
	}
	
	template <typename... Types>
	void Either<Types...>::assign_copy(const Self &other) {
		if (is_same_type_as(other)) {
			type_info().copy_assign((byte*)&memory_, (const byte*)&other.memory_);
		} else {
			type_info().destruct((byte*)memory_);
			type_index_ = other.type_index_;
			type_info().copy_construct((byte*)&memory_, (const byte*)&other.memory_);
		}
	}
	
	template <typename... Types>
	template <typename T>
	void Either<Types...>::assign_move(T&& value) {
		static const size_t I = IndexOfType<T, Types...>::Value;
		if (is_a<T>()) {
			type_info().move_or_copy_assign((byte*)&memory_, (byte*)&value);
		} else {
			type_info().destruct((byte*)&memory_);
			type_index_ = I;
			type_info().move_or_copy_construct((byte*)&memory_, (byte*)&value);
		}
	}
	
	template <typename... Types>
	template <typename T>
	void Either<Types...>::assign_copy(const T& value) {
		static const size_t I = IndexOfType<T, Types...>::Value;
		if (is_a<T>()) {
			type_info().copy_assign((byte*)&memory_, (const byte*)&value);
		} else {
			type_info().destruct((byte*)&memory_);
			type_index_ = I;
			type_info().copy_construct((byte*)&memory_, (const byte*)&value);
		}
	}
}

#endif
