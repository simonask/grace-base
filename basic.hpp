#pragma once
#ifndef BASIC_HPP_S0NRU03V
#define BASIC_HPP_S0NRU03V

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include <type_traits>

namespace grace {

typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long int uint64;
typedef float float32;
typedef double float64;
typedef uint8 byte;

#define GRACE_CAPI extern "C"

#if !defined(SSIZE_MAX)
#define SSIZE_MAX INTPTR_MAX
#endif

static const struct NothingType { NothingType() {} } Nothing;
using NullPtr = std::nullptr_t;

using std::move;
using std::begin;
using std::end;

template <typename T, typename NameGetter>
struct HasMember {
	typedef char MatchedReturnType;
	typedef long UnmatchedReturnType;
	
	template <typename C>
	static MatchedReturnType f(typename NameGetter::template Check<C>*);
	
	template <typename C>
	static UnmatchedReturnType f(...);
public:
	static const bool Value = (sizeof(f<T>(0)) == sizeof(MatchedReturnType)); 
};

struct CheckHasBuildTypeInfo {
	template <typename T, const typename T::TypeInfoType*(*)() = T::build_type_info__>
	struct Check {};
};
	
template <typename T>
struct HasReflection : HasMember<T, CheckHasBuildTypeInfo> {};

#define FORWARD_TO_MEMBER(METHOD_NAME, MEMBER, MEMBER_TYPE) \
template <typename ForwardType_ = MEMBER_TYPE, typename... ForwardArgs_> \
auto METHOD_NAME(ForwardArgs_&&... args) -> decltype(ForwardType_().METHOD_NAME(std::forward<ForwardArgs_>(args)...)) { \
return MEMBER.METHOD_NAME(std::forward<ForwardArgs_>(args)...); \
}
	
#define FORWARD_TO_MEMBER_CONST(METHOD_NAME, MEMBER, MEMBER_TYPE) \
template <typename ForwardType_ = MEMBER_TYPE, typename... ForwardArgs_> \
auto METHOD_NAME(ForwardArgs_&&... args) const -> decltype(((const ForwardType_)ForwardType_()).METHOD_NAME(std::forward<ForwardArgs_>(args)...)) { \
return MEMBER.METHOD_NAME(std::forward<ForwardArgs_>(args)...); \
}
	
#if __has_attribute(always_inline)
#define ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define ALWAYS_INLINE inline
#endif

#define UNSAFE_OFFSET_OF(T, MEMBER) (size_t)(&((T*)nullptr)->MEMBER)

#if defined(__GNUC__)
#define ALIGNED(N) __attribute__((aligned(N)))
#else
#error Compiler unsupported.
#endif

template <typename T>
struct RemoveConstRef {
	typedef typename std::remove_const<typename std::remove_reference<T>::type>::type Type;
};

template <typename U>
struct IsCopyConstructibleNonRef {
	static const bool Value = std::is_copy_constructible<typename RemoveConstRef<U>::Type>::value;
};

template <typename U>
struct IsCopyAssignableNonRef {
	static const bool Value = std::is_copy_assignable<typename RemoveConstRef<U>::Type>::value;
};

template <typename U>
struct IsMoveConstructibleNonRef {
	static const bool Value = std::is_move_constructible<typename RemoveConstRef<U>::Type>::value;
};

template <typename U>
struct IsMoveAssignableNonRef {
	static const bool Value = std::is_move_assignable<typename RemoveConstRef<U>::Type>::value;
};
	
	template <typename T> struct IsFloatingPoint;
	template <> struct IsFloatingPoint<float32> { enum { Value = true }; };
	template <> struct IsFloatingPoint<float64> { enum { Value = true }; };
	template <typename T> struct IsFloatingPoint { enum { Value = false }; };
	

	template <typename T> struct IsSigned;
	template <> struct IsSigned<uint8> {  enum { Value = false }; };
	template <> struct IsSigned<uint16> { enum { Value = false }; };
	template <> struct IsSigned<uint32> { enum { Value = false }; };
	template <> struct IsSigned<uint64> { enum { Value = false }; };
	template <typename T> struct IsSigned { enum { Value = true }; };
	
	template <typename T, typename R, typename... Args>
	struct GetMemberFunctionPointerType;
	template <typename T, typename R, typename... Args>
	struct GetMemberFunctionPointerType<const T, R, Args...> {
		typedef R(T::*Type)(Args...) const;
	};
	template <typename T, typename R, typename... Args>
	struct GetMemberFunctionPointerType {
		typedef R(T::*Type)(Args...);
	};

template <typename T>
void destruct(byte* ptr) {
	reinterpret_cast<T*>(ptr)->~T();
}
	
	template <typename T, typename... Args>
	inline void construct_range(T* begin, T* end, Args&&... args) {
		for (T* p = begin; p != end; ++p) {
			new(p) T(std::forward<Args>(args)...);
		}
	}
	
	template <typename T>
	inline void destruct_range(T* begin, T* end) {
		for (T* p = begin; p != end; ++p) {
			p->~T();
		}
	}

template <typename Container, typename Key, typename DefaultValue>
auto find_or(Container& container, const Key& key, const DefaultValue& default_value)
-> typename std::common_type<typename Container::mapped_type, DefaultValue>::type {
	auto it = container.find(key);
	if (it != container.end()) return it->second;
	return default_value;
}

template <typename Container, typename ComparableValue>
auto linear_search(Container& container, const ComparableValue& value)
-> decltype(container.begin())
{
	for (auto it = container.begin(); it != container.end(); ++it) {
		if (*it == value) {
			return it;
		}
	}
	return container.end();
}


	template <size_t...> struct Indices {};
	template <size_t N, size_t... S> struct MakeIndices : MakeIndices<N-1, N-1, S...> {};
	template <size_t... S> struct MakeIndices<0, S...> {
		typedef Indices<S...> Type;
	};
		
	template <typename T, typename FunctionType, typename... Args, size_t... I>
	auto apply_tuple_to_member_impl(T* object, FunctionType function, std::tuple<Args...> args, Indices<I...> i)
	-> decltype((object->*function)(std::get<I>(args)...))
	{
		return (object->*function)(std::get<I>(args)...);
	}
	
	template <typename T, typename FunctionType, typename... Args>
	auto apply_tuple_to_member(T* object, FunctionType function, std::tuple<Args...> args)
	-> decltype(apply_tuple_to_member_impl(object, function, std::move(args), typename MakeIndices<sizeof...(Args)>::Type()))
	{
		typedef typename MakeIndices<sizeof...(Args)>::Type Indices;
		return apply_tuple_to_member_impl(object, function, std::move(args), Indices());
	}
	
	
	template <typename T>
	constexpr T round_up_impl(T val, T boundary, T rest) {
		return rest != 0 ? val + (boundary - rest) : val;
	}
	
	template <typename T>
	constexpr T round_up(T val, T boundary) {
		return round_up_impl(val, boundary, val % boundary);
	}
	
	template <typename T, bool Move>
	struct MoveOrCopyImpl;
	template <typename T> struct MoveOrCopyImpl<T, true> {
		static void move_or_copy(T& dst, T& src) {
			dst = std::move(src);
		}
	};
	template <typename T> struct MoveOrCopyImpl<T, false> {
		static void move_or_copy(T& dst, const T& src) {
			dst = src;
		}
	};
	template <bool Move, typename T>
	void move_or_copy(T& dst, T& src) {
		MoveOrCopyImpl<T, Move>::move_or_copy(dst, src);
	}
	template <bool Move, typename T>
	void move_or_copy(T& dst, const T& src) {
		MoveOrCopyImpl<T, false>::move_or_copy(dst, src);
	}
	template <typename T, bool Move>
	struct MoveOrCopyConstructImpl;
	template <typename T> struct MoveOrCopyConstructImpl<T, true> {
		static void move_or_copy(void* dst, T& src) {
			new(dst) T(std::move(src));
		}
	};
	template <typename T> struct MoveOrCopyConstructImpl<T, false> {
		static void move_or_copy(void* dst, const T& src) {
			new(dst) T(src);
		}
	};
	template <bool Move, typename T>
	void move_or_copy_construct(void* dst, T& src) {
		MoveOrCopyConstructImpl<T, Move>::move_or_copy(dst, src);
	}
	template <bool Move, typename T>
	void move_or_copy_construct(void* dst, const T& src) {
		MoveOrCopyConstructImpl<T, false>::move_or_copy(dst, src);
	}
	
	struct Less {
		template <typename A, typename B>
		bool operator()(const A& a, const B& b) const {
			return a < b;
		}
	};
	
#define ENUM_IS_FLAGS(T) \
	inline typename std::underlying_type<T>::type \
	operator|(T a, T b) { return static_cast<typename std::underlying_type<T>::type>(+a | +b); } \
	inline typename std::underlying_type<T>::type \
	operator&(T a, T b) { return static_cast<typename std::underlying_type<T>::type>(+a & +b); } \
	inline typename std::underlying_type<T>::type \
	operator|(typename std::underlying_type<T>::type a, T b) \
	{ return a | +b; } \
	inline typename std::underlying_type<T>::type \
	operator|(T a, typename std::underlying_type<T>::type b) \
	{ return b | +a; }

#if defined(__i386__) || defined(__x86_64__)
#define TRAP() do{ __asm__ __volatile__("int3\n"); }while(0)
#elif defined(__arm__)
#define TRAP() do{ __asm__ __volatile__("bkpt 0\n"); }while(0)
#endif

#define UNREACHABLE() __builtin_unreachable()

//#if defined(DEBUG)
#define ASSERT(X) do{ if (!(X)) { fprintf(stderr, "TRAP AT %s:%d (function '%s', expression '%s')\n", __FILE__, __LINE__, __func__, #X); TRAP(); UNREACHABLE(); } } while(0)
//#else
//#define ASSERT(X) do{ if (!(X)) { TRAP(); UNREACHABLE(); } } while(0)
//#endif

#if defined(__GNUC__)
#  if defined(SHARED_LIBRARY)
#    define DLL_PUBLIC __attribute__((visibility("default")))
#    define DLL_LOCAL  __attribute__((visibility("hidden")))
#  else
#    define DLL_PUBLIC 
#    define DLL_LOCAL  __attribute__((visibility("default")))
#  endif
#elif defined(_MSC_VER)
#  if defined(SHARED_LIBRARY)
#    define DLL_PUBLIC __declspec((dllexport))
#    define DLL_LOCAL
#  else
#    define DLL_PUBLIC __declspec((dllimport))
#    define DLL_LOCAL
#  endif
#endif


#if defined(__has_feature) && __has_feature(cxx_lambdas)
#define HAS_LAMBDAS 1
#else
#define HAS_LAMBDAS 0
#endif

#define DECLARE_CONFIG_SCOPE(SCOPE) void define_config_scope_ ## SCOPE ##__()
#define REGISTER_CONFIG_SCOPE(SCOPE) define_config_scope_ ## SCOPE ##__()
	
}

#endif /* end of include guard: BASIC_HPP_S0NRU03V */
