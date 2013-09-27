//
//  either.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 28/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_either_hpp
#define grace_either_hpp

#include "base/basic.hpp"
#include "base/type_info.hpp"
#include "base/type_traits.hpp"

namespace grace {
	namespace detail {
		template <typename EitherType> struct WhenEitherControlFlow;
	}

	template <typename... Types> // max 255 types!
	class Either {
	public:
		static_assert(sizeof...(Types) < UINT8_MAX, "Too many possible types for Either.");
	
		using Self = Either<Types...>;
		static const bool IsCopyConstructible = AreAllCopyConstructible<Types...>::Value;
		static const bool IsCopyAssignable = AreallCopyAssignable<Types...>::Value;
		static const bool IsMoveConstructible = AreAllMoveConstructible<Types...>::Value;
		static const bool IsMoveAssignable = AreallMoveAssignable<Types...>::Value;
		
		template <typename T, typename = typename std::enable_if<IndexOfType<T, Types...>::Value < sizeof...(Types)>::type>
		Either(const T& value) {
			static const auto I = IndexOfType<T, Types...>::Value;
			static_assert(I < sizeof...(Types), "T is not a valid type for this Either.");
			construct_copy(value);
		}
		
		template <typename T, typename = typename std::enable_if<IndexOfType<T, Types...>::Value < sizeof...(Types)>::type>
		Either(T&& value) {
			static const auto I = IndexOfType<T, Types...>::Value;
			static_assert(I < sizeof...(Types), "T is not a valid type for this Either.");
			construct_move(move(value));
		}
		
		
		Either(const Self& other) {
			static_assert(IsCopyConstructible, "Cannot copy-construct this Either, because one of the inner types does not support it.");
			construct_copy_opaque(other);
		}
		Either(Self&& other) {
			static_assert(IsMoveConstructible, "Cannot move-construct this Either, because one of the inner types does not support it.");
			construct_move_opaque(move(other));
		}
		
		~Either();
		
		template <typename T, typename = typename std::enable_if<IsMoveAssignable && IndexOfType<T, Types...>::Value < sizeof...(Types)>::type>
		Self& operator=(T&& value) {
			assign_move<T>(move(value));
			return *this;
		}
		
		template <typename T, typename = typename std::enable_if<IsCopyAssignable && IndexOfType<T, Types...>::Value < sizeof...(Types)>::type>
		Self& operator=(const T& value) {
			assign_copy<T>(value);
			return *this;
		}
		
		Self& operator=(const Self& other) {
			static_assert(IsCopyAssignable, "Cannot copy-assign this Either, because one of the inner types does not support it.");
			assign_copy_opaque(other);
			return *this;
		}
		
		Self& operator=(Self&& other) {
			static_assert(IsMoveAssignable, "Cannot move-assign this Either, because one of the inner types does not support it.");
			assign_move_opaque(move(other));
			return *this;
		}
		
		template <typename T>
		typename std::enable_if<IndexOfType<T, Types...>::Value < sizeof...(Types), bool>::type
		is_a() const {
			return type_index_ == IndexOfType<T, Types...>::Value;
		}

		template <typename T>
		const T& get() const {
			ASSERT(is_a<T>());
			return *reinterpret_cast<const T*>(memory());
		}

		template <typename T>
		T& get() {
			ASSERT(is_a<T>());
			return *reinterpret_cast<T*>(memory());
		}
		
		bool is_same_type_as(const Self& other) const {
			return type_index_ == other.type_index_;
		}
		
		bool is_nothing() const {
			return is_a<NothingType>();
		}
		bool is_empty() const {
			return is_nothing();
		}
		void clear() const {
			assign_copy(Nothing);
		}
		
		template <typename T = NothingType>
		explicit operator bool() const {
			return !is_a<T>();
		}

		template <typename T, typename F>
		detail::WhenEitherControlFlow<Self> when(F&& f);
		template <typename T, typename F>
		detail::WhenEitherControlFlow<const Self> when(F&& f) const;
		
		template <typename Visitor>
		void visit(Visitor& visitor);
		template <typename Visitor>
		void visit(Visitor& visitor) const;
		
		template <typename = typename std::enable_if<(IsMoveAssignable && IsMoveConstructible) || (IsCopyAssignable && IsCopyConstructible)>::type>
		void swap(Self& other);
		
		const TypeInfo& type_info() const {
			return *type_infos_[type_index_];
		}
		
		byte* memory() { return reinterpret_cast<byte*>(&memory_); }
		const byte* memory() const { return reinterpret_cast<const byte*>(&memory_); }

		uint8 type_index() const { return type_index_; }
		uint8 which() const { return type_index(); }
	private:
		// Unconstructed contents -- must be constructed with construct_move/construct_copy before destructor is called!
		Either() {}
	
		template <bool> friend struct WhenControlFlow;
		using Storage = typename AlignedUnion<Types...>::Type;
		Storage memory_;
		uint8 type_index_ = UINT8_MAX;
		static constexpr const TypeInfo* const type_infos_[sizeof...(Types)] = {&GetTypeInfo<Types>::Value...};
		
		void assign_move_opaque(Self&& other);
		void assign_copy_opaque(const Self& other);
		template <typename T>
		void assign_move(T&& other);
		template <typename T>
		void assign_copy(const T& other);
		void construct_move_opaque(Self&& other);
		void construct_copy_opaque(const Self& other);
		template <typename T>
		void construct_move(T&& other);
		template <typename T>
		void construct_copy(const T& other);
		void destruct();
	};
	
	template <typename... Types>
	constexpr const TypeInfo* const Either<Types...>::type_infos_[sizeof...(Types)];
	
	template <typename... Types>
	Either<Types...>::~Either() {
		destruct();
	}
	
	template <typename... Types>
	template <typename> // enable_if IsMoveAssignable || IsCopyAssignable
	void Either<Types...>::swap(Self& other) {
		Self tmp; // Uninitialized.
		if (is_same_type_as(other)) {
			if (IsMoveConstructible && IsMoveAssignable) {
				tmp.construct_move_opaque(move(other));
				other.assign_move_opaque(move(*this));
				assign_move_opaque(tmp);
			} else if (IsCopyConstructible && IsMoveConstructible) {
				tmp.construct_copy_opaque(other);
				other.assign_copy_opaque(*this);
				assign_copy_opaque(tmp);
			} else {
				UNREACHABLE();
			}
		} else {
			if (IsMoveConstructible) {
				tmp.construct_move_opaque(move(other));
				other.destruct();
				other.construct_move_opaque(move(*this));
				destruct();
				construct_move(move(tmp));
			} else if (IsCopyConstructible) {
				tmp.construct_copy_opaque(other);
				other.destruct();
				other.construct_copy_opaque(*this);
				destruct();
				construct_copy_opaque(tmp);
			} else {
				UNREACHABLE();
			}
		}
	}
	
	template <typename... Types>
	void Either<Types...>::destruct() {
		ASSERT(type_index_ < sizeof...(Types));
		type_info().destruct(memory());
		detail::poison_memory(memory(), memory() + sizeof(memory_), detail::FREED_MEMORY_PATTERN);
		type_index_ = UINT8_MAX;
	}
	
	template <typename... Types>
	void Either<Types...>::assign_move_opaque(Self&& other) {
		if (is_same_type_as(other)) {
			type_info().move_assign(memory(), other.memory());
		} else {
			destruct();
			construct_move_opaque(move(other));
		}
	}
	
	template <typename... Types>
	void Either<Types...>::assign_copy_opaque(const Self &other) {
		if (is_same_type_as(other)) {
			type_info().copy_assign(memory(), other.memory());
		} else {
			destruct();
			construct_copy_opaque(other);
		}
	}
	
	template <typename... Types>
	void Either<Types...>::construct_move_opaque(Self&& other) {
		ASSERT(type_index_ == UINT8_MAX); // Already constructed!
		type_index_ = other.type_index_;
		detail::poison_memory(memory(), memory() + sizeof(memory_), detail::UNINITIALIZED_MEMORY_PATTERN);
		type_info().move_construct(memory(), other.memory());
	}
	
	template <typename... Types>
	void Either<Types...>::construct_copy_opaque(const Self &other) {
		ASSERT(type_index_ == UINT8_MAX); // Already constructed!
		type_index_ = other.type_index_;
		detail::poison_memory(memory(), memory() + sizeof(memory_), detail::UNINITIALIZED_MEMORY_PATTERN);
		type_info().copy_construct(memory(), other.memory());
	}
	
	template <typename... Types>
	template <typename T>
	void Either<Types...>::assign_move(T&& value) {
		if (this->is_a<T>()) {
			*reinterpret_cast<T*>(memory()) = move(value);
		} else {
			destruct();
			construct_move(move(value));
		}
	}
	
	template <typename... Types>
	template <typename T>
	void Either<Types...>::assign_copy(const T& value) {
		if (is_a<T>()) {
			*reinterpret_cast<T*>(memory()) = value;
		} else {
			destruct();
			construct_copy(value);
		}
	}
	
	template <typename... Types>
	template <typename T>
	void Either<Types...>::construct_move(T&& value) {
		static_assert(IndexOfType<T, Types...>::Value < sizeof...(Types), "Invalid type for this Either.");
		ASSERT(type_index_ == UINT8_MAX); // Already constructed!
		type_index_ = IndexOfType<T, Types...>::Value;
		new(memory()) T(move(value));
	}
	
	template <typename... Types>
	template <typename T>
	void Either<Types...>::construct_copy(const T& value) {
		static_assert(IndexOfType<T, Types...>::Value < sizeof...(Types), "Invalid type for this Either.");
		ASSERT(type_index_ == UINT8_MAX); // Already constructed!
		type_index_ = IndexOfType<T, Types...>::Value;
		new(memory()) T(value);
	}

	namespace detail {
		template <typename EitherType>
		struct WhenEitherControlFlow {
			EitherType& either_;
			bool handled_ = false;
			explicit WhenEitherControlFlow(EitherType& either) : either_(either) {}
			WhenEitherControlFlow(WhenEitherControlFlow<EitherType>&& other) = default;
			WhenEitherControlFlow(const WhenEitherControlFlow<EitherType>& other) = default;

			template <typename T, typename F>
			WhenEitherControlFlow<EitherType>& when(F function) {
				using InnerType = typename std::conditional<std::is_const<EitherType>::value, const T, T>::type;
				if (either_.template is_a<T>()) {
					InnerType* memory = reinterpret_cast<InnerType*>(either_.memory());
					function(*memory);
					handled_ = true;
				}
				return *this;
			}

			template <typename F>
			void otherwise(F function) {
				if (!handled_) {
					function();
				}
			}
		};
	}

	template <typename... Types>
	template <typename T, typename F>
	detail::WhenEitherControlFlow<Either<Types...>> Either<Types...>::when(F&& function) {
		return detail::WhenEitherControlFlow<Self>(*this).template when<T>(std::forward<F>(function));
	}

	template <typename... Types>
	template <typename T, typename F>
	detail::WhenEitherControlFlow<const Either<Types...>> Either<Types...>::when(F&& function) const {
		return detail::WhenEitherControlFlow<const Self>(*this).template when<T>(std::forward<F>(function));
	}
	
	namespace detail {
		template <typename EitherClass, typename... RemainingTypes> struct VisitorCaller;
		template <typename EitherClass> struct VisitorCaller<EitherClass> {
			VisitorCaller(EitherClass& either) {}
			template <typename Visitor>
			void visit(Visitor&) {}
		};
		template <typename EitherClass, typename T, typename... Rest> struct VisitorCaller<EitherClass, T, Rest...> {
			EitherClass& either_;
			using ValueType = typename std::conditional<std::is_const<EitherClass>::value, const T, T>::type;
			VisitorCaller(EitherClass& either) : either_(either) {}
			
			template <typename Visitor>
			void visit(Visitor& visitor) {
				if (either_.template is_a<ValueType>()) {
					visitor(*reinterpret_cast<ValueType*>(either_.memory()));
				} else {
					VisitorCaller<EitherClass, Rest...>(either_).visit(visitor);
				}
			}
		};
	}
	
	template <typename... Types>
	template <typename Visitor>
	void Either<Types...>::visit(Visitor& visitor) {
		detail::VisitorCaller<Self, Types...>(*this).visit(visitor);
	}
	
	template <typename... Types>
	template <typename Visitor>
	void Either<Types...>::visit(Visitor& visitor) const {
		detail::VisitorCaller<Self, Types...>(*this).visit(visitor);
	}
}

#endif
