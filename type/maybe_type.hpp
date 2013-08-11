#pragma once
#ifndef _grace_maybe_hpp_included
#define _grace_maybe_hpp_included

#include "type/type.hpp"
#include "base/maybe.hpp"
#include "serialization/document_node.hpp"

namespace grace {

String build_maybe_type_name(const IType* inner_type);

template <typename T>
struct MaybeType : TypeFor<Maybe<T>> {
	MaybeType() : name_(build_maybe_type_name(get_type<T>())) {}
	
	void deserialize(Maybe<T>& place, const DocumentNode&, IUniverse&) const;
	void serialize(const Maybe<T>& place, DocumentNode&, IUniverse&) const;
	
	StringRef name() const { return name_; }
	
	const IType* inner_type() const { return get_type<T>(); }
private:
	String name_;
};

template <typename T>
void MaybeType<T>::deserialize(Maybe<T>& m, const DocumentNode& node, IUniverse& universe) const {
	if (!node.is_empty()) {
		T value;
		inner_type()->deserialize_raw(reinterpret_cast<byte*>(&value), node, universe);
		m = std::move(value);
	}
}

template <typename T>
void MaybeType<T>::serialize(const Maybe<T>& m, DocumentNode& node, IUniverse& universe) const {
	/*struct Serialize {
		const IType* inner_type;
		DocumentNode& node;
		IUniverse& universe;
		Serialize(const IType* inner_type, DocumentNode& node, IUniverse& universe) : inner_type(inner_type), node(node), universe(universe) {}
		void operator()(const T& it) {
			inner_type->serialize(reinterpret_cast<const byte*>(&it), node, universe);
		};
	};
	
	maybe_if(m, Serialize(inner_type(), node, universe));*/
	
	m.map([&](const T& it) {
		inner_type()->serialize_raw(reinterpret_cast<const byte*>(&it), node, universe);
	});
}

template <typename T>
struct BuildTypeInfo<Maybe<T>> {
	static const MaybeType<T>* build() {
		static const MaybeType<T>* type = new_static MaybeType<T>;
		return type;
	}
};

}

#endif