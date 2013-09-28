#ifndef GRACE_STL_ALLOCATOR_HPP_INCLUDED
#define GRACE_STL_ALLOCATOR_HPP_INCLUDED

#include "memory/allocator.hpp"

namespace grace {
	template <typename T>
	struct STLAllocator {
		STLAllocator() : alloc(&default_allocator()) {}
		STLAllocator(IAllocator& alloc) : alloc(&alloc) {}
		STLAllocator(const STLAllocator<T>& other) = default;
		template <typename U>
		STLAllocator(const STLAllocator<U>& other) : alloc(&other.allocator()) {}
		STLAllocator<T>& operator=(STLAllocator<T>& other) = default;
		IAllocator& allocator() const { return *alloc; }

		// std::allocator class interface
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		template <typename U>
		struct rebind {
			using other = STLAllocator<U>;
		};

		// std::allocator instance interface
		pointer address(reference x) const {
			return &reinterpret_cast<byte&>(x);
		}

		const_pointer address(const_reference x) const {
			return &reinterpret_cast<const byte&>(x);
		}

		pointer allocate(size_type n, const_pointer hint_ignored = nullptr) {
			return (pointer)alloc->allocate(sizeof(T) * n, alignof(T));
		}

		void deallocate(pointer p, size_type n) {
			alloc->free(p, sizeof(T) * n);
		}

		size_type max_size() const {
			return SIZE_T_MAX;
		}

		template <typename U, typename... Args>
		void construct(U* p, Args&&... args) {
			new(p) U(std::forward<Args>(args)...);
		}

		template <typename U>
		void destroy(U* p) {
			p->~U();
		}
	private:
		IAllocator* alloc = nullptr;
	};
}

#endif
