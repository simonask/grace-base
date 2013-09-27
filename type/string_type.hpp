#pragma once
#ifndef GRACE_STRING_TYPE_HPP_INCLUDED
#define GRACE_STRING_TYPE_HPP_INCLUDED

#include "type/type.hpp"

namespace grace {
	struct StringType : TypeFor<String> {
	static const StringType* get();
	
	void deserialize(String& place, const DocumentNode&, IUniverse&) const final;
	void serialize(const String& place, DocumentNode&, IUniverse&) const final;
	
	StringRef name() const final;
};
	
struct StringRefType : TypeFor<StringRef> {
	static const StringRefType* get();
	void deserialize(StringRef& place, const DocumentNode&, IUniverse&) const final;
	void serialize(const StringRef& place, DocumentNode&, IUniverse&) const final;
	StringRef name() const final;
};

template <> struct BuildTypeInfo<String> {
	static const StringType* build() { return StringType::get(); }
};
	
template <> struct BuildTypeInfo<StringRef> {
	static const StringRefType* build() { return StringRefType::get(); }
};

}

#endif
