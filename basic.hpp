#pragma once
#ifndef BASIC_HPP_S0NRU03V
#define BASIC_HPP_S0NRU03V

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include <type_traits>

namespace falling {

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

template <typename T>
void destruct(T* ptr) {
	ptr->~T();
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

#define ASSERT(X) do{ if (!(X)) { fprintf(stderr, "TRAP AT %s:%d (function '%s', expression '%s')\n", __FILE__, __LINE__, __func__, #X); __asm__ __volatile__("int3\n"); } } while(0)


#if defined(__has_feature) && __has_feature(cxx_lambdas)
#define HAS_LAMBDAS 1
#else
#define HAS_LAMBDAS 0
#endif
	
}

#endif /* end of include guard: BASIC_HPP_S0NRU03V */
