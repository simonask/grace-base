//
//  enum_type.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 24/02/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_enum_type_hpp
#define grace_enum_type_hpp

#include "type/type.hpp"

namespace grace {
	template <typename T>
	struct EnumTypeImpl : TypeFor<T, EnumType> {
		EnumTypeImpl(IAllocator& alloc, StringRef name, size_t width) : TypeFor<T, EnumType>(alloc, name, width) {}
	
		void deserialize(T& place, const DocumentNode& node, IUniverse&) const {
			StringRef name;
			ssize_t value;
			if (node >> name) {
				if (this->value_for_name(name, value)) {
					place = (T)value;
				} else {
					Warning() << "Enum type '" << this->name() << "' does not contain a value named '" << name << "'.";
				}
			} else if (node >> value) {
				if (this->contains(value) || this->allows_arbitrary_value()) {
					place = (T)value;
				} else {
					Warning() << "Integer " << value << " is invalid for enum type '" << this->name() << "'.";
				}
			}
		}
		
		void serialize(const T& place, DocumentNode& node, IUniverse&) const {
			StringRef name;
			ssize_t value = (ssize_t)place;
			if (this->name_for_value(value, name)) {
				node << name;
			} else {
				if (this->allows_arbitrary_value()) {
					node << value;
				} else {
					Error() << "The integer value " << value << " is invalid for enum type '" << this->name() << "'.";
				}
			}
		}
	};

	template <typename T>
	struct EnumTypeBuilder {
		virtual void define__() = 0;
		
		const EnumTypeImpl<T>* build__() {
			define__();
			EnumTypeImpl<T>* t = new_static EnumTypeImpl<T>(static_allocator(), name_, width_);
			t->entries_ = move(entries_);
			t->allow_arbitrary_value_ = allow_arbitrary_integers_;
			t->min_ = min_;
			t->max_ = max_;
			return t;
		}
		
		EnumTypeBuilder<T>& name(StringRef name) { name_ = name; return *this; }
		EnumTypeBuilder<T>& width(size_t bytes) { width_ = bytes; return *this; }
		EnumTypeBuilder<T>& allow_arbitrary_integers(bool b) { allow_arbitrary_integers_ = b; return *this; }
		
		EnumTypeBuilder& value(T val, StringRef name, StringRef description = "") {
			entries_.push_back(EnumType::Entry{name, description, (ssize_t)val});
			return *this;
		}
		EnumTypeBuilder& value(ssize_t val, StringRef name, StringRef description = "") {
			ASSERT(allow_arbitrary_integers_);
			ASSERT(val >= SSIZE_MIN);
			ASSERT(val <= SSIZE_MAX);
			entries_.push_back(EnumType::Entry{name, description, val});
			return *this;
		}
		
		EnumTypeBuilder& min(ssize_t m) {
			min_ = m;
			return *this;
		}
		EnumTypeBuilder& max(ssize_t m) {
			max_ = m;
			return *this;
		}
		
		Array<EnumType::Entry> entries_;
		bool allow_arbitrary_integers_ = false;
		StringRef name_;
		size_t width_;
		ssize_t min_ = SSIZE_MIN;
		ssize_t max_ = SSIZE_MAX;
	};
}

#define BEGIN_ENUM_INFO(T) \
	struct EnumTypeBuilder_##T : grace::EnumTypeBuilder<T> { \
		static const EnumType* build(); \
		void define__() final; \
	}; \
	const grace::EnumType* grace::BuildTypeInfo<T>::build() { \
		static const grace::EnumType* p = EnumTypeBuilder_##T::build(); \
		return p; \
	} \
	inline const grace::EnumType* EnumTypeBuilder_##T::build() { \
		EnumTypeBuilder_##T builder; \
		builder.name(#T); \
		builder.width(sizeof(T)); \
		return builder.build__(); \
	} \
	void EnumTypeBuilder_##T::define__() {
#define END_ENUM_INFO()	\
	}

#endif
