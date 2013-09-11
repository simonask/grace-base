#pragma once
#ifndef GRACE_BASE_ERROR_HPP
#define GRACE_BASE_ERROR_HPP

#include "memory/unique_ptr.hpp"
#include "base/either.hpp"

namespace grace {
	template <typename> class ArrayRef;

	struct IError {
		virtual ~IError() {}
		virtual IError* duplicate(IAllocator& alloc) const = 0;
		virtual IError* move_to_heap(IAllocator& alloc) = 0;
		virtual StringRef type_name() const = 0;
		virtual StringRef description() const = 0;
		virtual ArrayRef<void*> backtrace() const = 0;
	};

	class FormattedStream;
	FormattedStream& operator<<(FormattedStream& os, const IError& error);

	using IErrorPtr = UniquePtr<const IError>;

	struct ErrorBaseImpl : IError {
		ErrorBaseImpl();
		ErrorBaseImpl(const ErrorBaseImpl& other);
		ErrorBaseImpl(ErrorBaseImpl&& other) : allocator_(other.allocator_) {
			std::swap(description_data_, other.description_data_);
			std::swap(description_length_, other.description_length_);
			std::swap(backtrace_, other.backtrace_);
			std::swap(backtrace_length_, other.backtrace_length_);
		}
		virtual ~ErrorBaseImpl();
		ArrayRef<void*> backtrace() const final;
		StringRef type_name() const;
		StringRef description() const;
		
		IAllocator& allocator() const { return allocator_; }

		// This is used to move text from the formatter into the error without copying, *AND* without
		// needing the String class, which would cause a circular dependency.
		void _take_description(char* description, size_t description_length, IAllocator& check_allocator);
		virtual const char* _internal_type_name() const = 0;
	private:
		void** backtrace_ = nullptr;
		size_t backtrace_length_ = 0;
		char* description_data_ = nullptr;
		size_t description_length_ = 0;
		IAllocator& allocator_;

		void build_stack_trace();
	};

	template <typename T>
	struct ErrorBase : ErrorBaseImpl {
		IError* duplicate(IAllocator& alloc) const final {
			return new(alloc) T(*dynamic_cast<const T*>(this));
		}
		IError* move_to_heap(IAllocator& alloc) final {
			return new(alloc) T(std::move(*dynamic_cast<T*>(this)));
		}
		const char* _internal_type_name() const final {
			return typeid(T).name();
		}
	};

	template <typename T = void> struct E;

	template <typename T>
	struct E {
		E(T x) : data_(std::move(x)) {}
		E(IErrorPtr error) : data_(std::move(error)) {}
		E(E<T>&& other) = default;
		E<T>& operator=(T x) { data_ = Container(std::move(x)); return *this; }
		E<T>& operator=(IErrorPtr x) { data_ = Container(std::move(x)); return *this; }

		bool is_error() const { return data_.template is_a<IErrorPtr>(); }

		const IError* error() const {
			const IErrorPtr* err = nullptr;
			data_.template when<IErrorPtr>([&](const IErrorPtr& ptr) {
				err = &ptr;
			});
			if (err) return err->get();
			return nullptr;
		}

		template <typename F>
		void map(F function) {
			data_.template when<T>(std::move(function));
		}
		template <typename F>
		void map(F function) const {
			data_.template when<T>(std::move(function));
		}
		template <typename F>
		void when_error(F function) const {
			data_.template when<IErrorPtr>([&](const IErrorPtr& ptr) {
				function(*ptr);
			});
		}

		IErrorPtr* error_ptr_ptr() {
			IErrorPtr* ptr = nullptr;
			data_.template when<IErrorPtr>([&](IErrorPtr& p) {
				ptr = &p;
			});
			return ptr;
		}
	private:
		using Container = Either<T, IErrorPtr>;
		Container data_;
	};

	template <>
	struct E<void> {
		using ResultType = NothingType;
		E() {}
		E(IErrorPtr error) : ptr_(std::move(error)) {}
		E(E<void>&& other) = default;
		E<void>& operator=(IErrorPtr x) { ptr_ = std::move(x); return *this; }
		E<void>& operator=(NothingType) { ptr_ = nullptr; return *this; }

		bool is_error() const { return ptr_ != nullptr; }

		const IError* error() const {
			return ptr_.get();
		}

		template <typename F>
		void when_error(F function) const {
			if (ptr_) {
				function(*ptr_);
			}
		}

		IErrorPtr* error_ptr_ptr() {
			if (ptr_) return &ptr_;
			return nullptr;
		}
	private:
		IErrorPtr ptr_;
	};

	template <typename R> struct CatchingErrors;
	template <> struct CatchingErrors<void> {
		template <typename F, typename... Args>
		static E<void> call(F function, Args&&... args) {
			try {
				function(std::forward<Args>(args)...);
				return E<void>();
			}
			catch (IError& error) {
				IErrorPtr err;
				err.reset(default_allocator(), error.move_to_heap(default_allocator()));
				return std::move(err);
			}
			catch (const IError& error) {
				IErrorPtr err;
				err.reset(default_allocator(), error.duplicate(default_allocator()));
				return std::move(err);
			}
		}
	};
	template <typename R> struct CatchingErrors {
		template <typename F, typename... Args>
		static E<R> call(F function, Args&&... args) {
			try {
				R result = function(std::forward<Args>(args)...);
				return std::move(result);
			}
			catch (IError& error) {
				IErrorPtr err;
				err.reset(default_allocator(), error.move_to_heap(default_allocator()));
				return std::move(err);
			}
			catch (const IError& error) {
				IErrorPtr err;
				err.reset(default_allocator(), error.duplicate(default_allocator()));
				return std::move(err);
			}
		}
	};

	template <typename F, typename... Args>
	auto catching_errors(F function, Args&&... args)
	-> E<decltype(function(std::forward<Args>(args)...))> {
		using ReturnType = decltype(function(std::forward<Args>(args)...));

		return CatchingErrors<ReturnType>::call(function, std::forward<Args>(args)...);
	}

	template <typename T> struct RemoveE;
	template <typename T> struct RemoveE<E<T>> { using Type = T; };
	template <typename T> struct RemoveE       { using Type = T; };
	template <typename T> struct AddE;
	template <typename T> struct AddE<E<T>> { using Type = E<T>; };
	template <typename T> struct AddE       { using Type = E<T>; };

	// RECURSION BASE: unwrap_args_or_return_first_error
	template <size_t I, typename Tuple>
	IErrorPtr unwrap_args_or_return_first_error(Tuple& tuple) {
		return nullptr;
	}

	// UNWRAP MOVE-FORWARDED ERROR
	template <size_t I, typename Tuple, typename Head, typename... Tail>
	IErrorPtr unwrap_args_or_return_first_error(Tuple& tuple, E<Head>& x, Tail&&... tail) {
		IErrorPtr* eptr = x.error_ptr_ptr();
		if (eptr) { return std::move(*eptr); }
		x.map([&](Head& value) {
			std::get<I>(tuple) = std::move(value);
		});
		return unwrap_args_or_return_first_error<I+1>(tuple, std::forward<Tail>(tail)...);
	}

	// UNWRAP CONST-FORWARDED ERROR
	template <size_t I, typename Tuple, typename Head, typename... Tail>
	IErrorPtr unwrap_args_or_return_first_error(Tuple& tuple, const E<Head>& x, Tail&&... tail) {
		IErrorPtr err_ptr;
		x.when_error([&](const IError& err) {
			err_ptr.reset(default_allocator(), err.duplicate(default_allocator()));
		});
		if (err_ptr) {
			return std::move(err_ptr);
		} else {
			x.map([&](const Head& value) {
				std::get<I>(tuple) = value;
			});	
			return unwrap_args_or_return_first_error<I+1>(tuple, std::forward<Tail>(tail)...);
		}
	}

	// FORWARD MOVED VALUE
	template <size_t I, typename Tuple, typename Head, typename... Tail>
	IErrorPtr unwrap_args_or_return_first_error(Tuple& tuple, Head& x, Tail&&... tail) {
		std::get<I>(tuple) = std::move(x);
		return unwrap_args_or_return_first_error<I+1>(tuple, std::forward<Tail>(tail)...);
	}

	// FORWARD CONST VALUE
	template <size_t I, typename Tuple, typename Head, typename... Tail>
	IErrorPtr unwrap_args_or_return_first_error(Tuple& tuple, const Head& x, Tail&&... tail) {
		std::get<I>(tuple) = x;
		return unwrap_args_or_return_first_error<I+1>(tuple, std::forward<Tail>(tail)...);
	}

	template <typename F, typename... Args>
	auto bind(F functor, Args&&... args)
	-> typename AddE<typename std::result_of<F(typename RemoveE<typename RemoveConstRef<Args>::Type>::Type...)>::type>::Type
	{
		using Tuple = std::tuple<typename RemoveE<typename RemoveConstRef<Args>::Type>::Type...>;
		Tuple unwrapped_args;
		IErrorPtr err = unwrap_args_or_return_first_error<0>(unwrapped_args, std::forward<Args>(args)...);
		if (err) {
			return std::move(err);
		}
		return apply_tuple_to_function(functor, std::move(unwrapped_args));
	}
}

#endif
