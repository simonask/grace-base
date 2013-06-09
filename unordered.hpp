//
//  unordered.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/05/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_unordered_hpp
#define grace_unordered_hpp

#include "base/basic.hpp"

namespace grace {
	template <typename C>
	class Unordered {
	private:
		C data_;
	public:
		typedef typename C::value_type value_type;
		typedef typename C::iterator iterator;
		typedef typename C::const_iterator const_iterator;
		
		iterator begin() { return data_.begin(); }
		iterator end() { return data_.end(); }
		const_iterator begin() const { return data_.begin(); }
		const_iterator end() const { return data_.end(); }
		
		template <typename... Args>
		Unordered(Args&&... args) : data_(std::forward<Args>(args)...) {}
		Unordered(const Unordered<C>& other) : data_(other.data_) {}
		Unordered(Unordered<C>&& other) : data_(std::move(other.data_)) {}
		
		// The following fowards whatever interface the underlying container supports,
		// by using SFINAE and perfect forwarding.
		
		FORWARD_TO_MEMBER_CONST(size, data_, C)
		FORWARD_TO_MEMBER(push_back, data_, C)
		FORWARD_TO_MEMBER(push_front, data_, C)
		FORWARD_TO_MEMBER(reserve, data_, C)
		FORWARD_TO_MEMBER(resize, data_, C)
		FORWARD_TO_MEMBER(clear, data_, C)
		FORWARD_TO_MEMBER(emplace_back, data_, C)
		FORWARD_TO_MEMBER(insert, data_, C)
		
		/*template <typename T = C>
		auto size() const -> decltype(C().size()) {
			return data_.size();
		}
		
		template <typename T = C, typename... Args>
		auto push_back(Args&&... args) -> decltype(C().push_back(std::forward<Args>(args)...)) {
			return data_.push_back(std::forward<Args>(args)...);
		}
		
		template <typename T = C, typename... Args>
		auto push_front(Args&&... args) -> decltype(T().push_front(std::forward<Args>(args)...)) {
			return data_.push_front(std::forward<Args>(args)...);
		}
		
		template <typename T = C, typename... Args>
		auto reserve(Args&&... args) -> decltype(T().reserve(std::forward<Args>(args)...)) {
			return data_.reserve(std::forward<Args>(args)...);
		}
		
		template <typename T = C, typename... Args>
		auto resize(Args&&... args) -> decltype(T().resize(std::forward<Args>(args)...)) {
			return data_.resize(std::forward<Args>(args)...);
		}
		
		template <typename T = C, typename... Args>
		auto clear(Args&&... args) -> decltype(T().clear(std::forward<Args>(args)...)) {
			return data_.clear(std::forward<Args>(args)...);
		}
		
		template <typename T = C, typename... Args>
		auto emplace_back(Args&&... args) -> decltype(T().emplace_back(std::forward<Args>(args)...)) {
			return data_.emplace_back(std::forward<Args>(args)...);
		}
		
		template <typename T = C, typename... Args>
		auto insert(Args&&... args) -> decltype(T().insert(std::forward<Args>(args)...)) {
			return data_.insert(std::forward<Args>(args)...);
		}*/
		
		iterator erase(iterator it) {
			if (begin() != end()) {
				auto last = end()-1;
				if (it == last) {
					data_.erase(it);
					return end();
				} else {
					std::swap(*it, *last);
					data_.erase(last);
					return it;
				}
			} else {
				return end();
			}
		}
	};
}


#endif
