#pragma once
#ifndef BAG_HPP_LRAVL9CJ
#define BAG_HPP_LRAVL9CJ

#include "base/array.hpp"
#include "memory/fixed_allocator.hpp"

namespace grace {

template <typename T, typename Container = Array<T*>>
class ContainedBag {
public:
	explicit ContainedBag(IAllocator& alloc = default_allocator()) : allocator_(alloc), elements_(alloc) {}
	ContainedBag(ContainedBag<T>&& other) = default;
	~ContainedBag() { clear(); }
	
	typedef typename Container::iterator iterator;
	typedef typename Container::const_iterator const_iterator;
	
	template <typename... Args>
	T* allocate(Args&&... args) {
		T* ptr = new(allocator_, alignof(T)) T(std::forward<Args>(args)...);
		elements_.push_back(ptr);
		return ptr;
	}
	
	void deallocate(iterator it) {
		destroy(*it, allocator_);
		elements_.erase(it);
	}
	
	iterator begin() { return elements_.begin(); }
	iterator end() { return elements_.end(); }
	const_iterator begin() const { return elements_.begin(); }
	const_iterator end() const { return elements_.end(); }
	size_t size() const { return elements_.size(); }
	
	void clear() {
		for (auto it: elements_) {
			destroy(it, allocator_);
		}
		elements_.clear();
	}
private:
	FixedAllocator<sizeof(T), alignof(T)> allocator_;
	Container elements_;
};

}

#endif /* end of include guard: BAG_HPP_LRAVL9CJ */
