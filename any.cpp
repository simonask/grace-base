//
//  any.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/any.hpp"
#include "serialization/archive_node.hpp"
#include "type/array_type.hpp"
#include "type/map_type.hpp"

namespace falling {
	void AnyType::deserialize(Any &place, const ArchiveNode &n, IUniverse &u) const {
		ArchiveNode::IntegerType inum;
		ArchiveNode::FloatType fnum;
		if (n.is_empty()) {
			place = Nothing;
		} else if (n >> inum) {
			place = inum;
		} else if (n >> fnum) {
			place = fnum;
		} else if (n.is_array()) {
			Array<Any> v;
			v.reserve(n.array_size());
			for (size_t i = 0; i < n.array_size(); ++i) {
				Any x;
				deserialize(x, n[i], u);
				v.push_back(move(x));
			}
			place = move(v);
		} else if (n.is_map()) {
			Map<String, Any> v;
			n.map_each_pair([&](StringRef key, const ArchiveNode* value) {
				Any x;
				deserialize(x, *value, u);
				v[key] = move(x);
			});
			place = move(v);
		} else {
			ASSERT(false);
		}
	}
	
	void AnyType::serialize(const Any& place, ArchiveNode& n, IUniverse& u) const {
		if (!place.is_empty()) {
			place.type()->serialize_raw(place.ptr(), n, u);
		} else {
			n.clear();
		}
	}
	
	const AnyType* BuildTypeInfo<Any>::build() {
		static const AnyType* t = new_static AnyType;
		return t;
	}
}
