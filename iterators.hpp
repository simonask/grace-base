//
//  iterators.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 25/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_iterators_hpp
#define falling_iterators_hpp

namespace falling {
	template <typename T>
	struct GetNodeValueType {
		using Type = T;
	};
	
	template <typename T>
	struct GetValueForNode {
		static T* get(T* x) { return x; }
	};
	
	template <typename T>
	struct GetNextNode {
		static T* get(T* x) { return x->next; }
	};
	template <typename T>
	struct GetPreviousNode {
		static T* get(T* x) { return x->previous; }
	};

	template <class Owner, typename Node, bool IsConst>
	struct ForwardLinkListIterator {
	public:
		using Self = ForwardLinkListIterator<Owner, Node, IsConst>;
		using ValueTypeRaw = typename GetNodeValueType<Node>::Type;
		using ValueType = typename std::conditional<IsConst, const ValueTypeRaw, ValueTypeRaw>::type;
		
		ForwardLinkListIterator() : current_(nullptr) {}
		ForwardLinkListIterator(const ForwardLinkListIterator<Owner, Node, false>& other) : current_(other.current_) {}
		template <bool IsConst_ = IsConst>
		ForwardLinkListIterator(const typename std::enable_if<IsConst_, Self>::type& other) : current_(other.current_) {}
		Self& operator=(const ForwardLinkListIterator<Owner, Node, false>& other) {
			current_ = other.current_;
			return *this;
		}
		template <bool IsConst_ = IsConst>
		Self& operator=(const typename std::enable_if<IsConst_, Self>::type& other) {
			current_ = other.current_;
			return *this;
		}
		
		ValueType* get() const { return GetValueForNode<Node>::get(current_); }
		
		ValueType* operator->() const { return get(); }
		ValueType& operator*() const { return *get(); }
		
		Self& operator++() {
			current_ = GetNextNode<Node>::get(current_);
			return *this;
		}
		
		Self& operator--() {
			current_ = GetPreviousNode<Node>::get(current_);
			return *this;
		}
		
		Self operator++(int) {
			Self s = *this;
			++(*this);
			return s;
		}
		
		Self operator--(int) {
			Self s = *this;
			--(*this);
			return s;
		}
		
		Self operator+(int n) const {
			Self s = *this;
			s += n;
			return s;
		}
		
		Self& operator+=(int n) {
			if (n > 0) {
				for (int i = 0; i < n; ++i) {
					++(*this);
				}
			} else if (n < 0) {
				for (int i = 0; i < -n; ++i) {
					--(*this);
				}
			}
			return *this;
		}
		
		Self operator-(int n) const {
			return (*this) + (-n);
		}
		
		Self& operator-=(int n) {
			return (*this) += -n;
		}
		
		template <bool B>
		bool operator==(const ForwardLinkListIterator<Owner, Node, B>& other) const {
			return current_ == other.current_;
		}
		template <bool B>
		bool operator!=(const ForwardLinkListIterator<Owner, Node, B>& other) const {
			return current_ != other.current_;
		}
	private:
		friend struct ForwardLinkListIterator<Owner, Node, !IsConst>;
		friend Owner;
		
		ForwardLinkListIterator(Node* n) : current_(n) {}
		
		Node* current_;
	};
	
	
	template <class Owner, typename T, bool IsConst>
	struct LinearMemoryIterator {
	public:
		using Self = LinearMemoryIterator<Owner, T, IsConst>;
		using ValueType = typename std::conditional<IsConst, const T, T>::type;
		
		LinearMemoryIterator() {}
		LinearMemoryIterator(const LinearMemoryIterator<Owner, T, false>& other) : current_(other.current_) {}
		template <bool IsConst_ = IsConst>
		LinearMemoryIterator(const typename std::enable_if<IsConst_, Self>::type& other) : current_(other.current_) {}
		
		Self& operator=(const LinearMemoryIterator<Owner, T, false>& other) {
			current_ = other.current_;
			return *this;
		}
		template <bool IsConst_ = IsConst>
		Self& operator=(const typename std::enable_if<IsConst_, Self>::type& other) {
			current_ = other.current_;
			return *this;
		}
		
		ValueType* get() const { return current_; }
		ValueType* operator->() const { return current_; }
		ValueType& operator*() const { return *current_; }
		
		Self& operator++() {
			++current_;
			return *this;
		}
		Self operator++(int) {
			Self s = *this;
			++(*this);
			return s;
		}
		
		Self& operator--() {
			--current_;
			return *this;
		}
		Self operator--(int) {
			Self s = *this;
			++(*this);
			return s;
		}
		
		Self& operator+=(ptrdiff_t d) {
			current_ += d;
			return *this;
		}
		
		Self& operator-=(ptrdiff_t d) {
			current_ -= d;
			return *this;
		}
		
		Self operator+(ptrdiff_t d) {
			Self s = *this;
			s += d;
			return s;
		}
		
		Self operator-(ptrdiff_t d) {
			Self s = *this;
			s -= d;
			return s;
		}
		
		template <bool B>
		ptrdiff_t operator-(const LinearMemoryIterator<Owner, T, B>& other) {
			return current_ - other.current_;
		}
		
		template <bool B>
		bool operator==(const LinearMemoryIterator<Owner, T, B>& other) {
			return current_ == other.current_;
		}
		template <bool B>
		bool operator!=(const LinearMemoryIterator<Owner, T, B>& other) {
			return current_ != other.current_;
		}
		template <bool B>
		bool operator>=(const LinearMemoryIterator<Owner, T, B>& other) {
			return current_ >= other.current_;
		}
		template <bool B>
		bool operator>(const LinearMemoryIterator<Owner, T, B>& other) {
			return current_ > other.current_;
		}
		template <bool B>
		bool operator<=(const LinearMemoryIterator<Owner, T, B>& other) {
			return current_ <= other.current_;
		}
		template <bool B>
		bool operator<(const LinearMemoryIterator<Owner, T, B>& other) {
			return current_ < other.current_;
		}
		
	private:
		friend Owner;
		friend struct LinearMemoryIterator<Owner, T, !IsConst>;
		ValueType* current_ = nullptr;
		
		LinearMemoryIterator(ValueType* x) : current_(x) {}
	};
}

#endif
