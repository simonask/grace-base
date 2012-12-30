//
//  any.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/any.hpp"

namespace falling {
	void AnyType::deserialize(Any &place, const ArchiveNode &n, UniverseBase &u) const {
		switch (n.type()) {
			case ArchiveNodeType::Empty: {
				place = Nothing; break;
			}
			case ArchiveNodeType::Float: {
				float64 v;
				n.get(v);
				place = v;
				break;
			}
			case ArchiveNodeType::Integer: {
				int64 v;
				n.get(v);
				place = v;
				break;
			}
			case ArchiveNodeType::String: {
				place = n.string_value;
				break;
			}
			case ArchiveNodeType::Array: {
				Array<Any> v;
				v.reserve(n.array_size());
				for (size_t i = 0; i < n.array_size(); ++i) {
					Any x;
					deserialize(x, n[i], u);
					v.push_back(move(x));
				}
				place = move(v);
				break;
			}
			case ArchiveNodeType::Map: {
				ASSERT(false); // MapType not implemented yet!
				UNREACHABLE();
				break;
				/*std::map<String, Any> v;
				for (auto pair: n.internal_map()) {
					Any x;
					deserialize(x, *pair.second, u);
					v[pair.first] = move(x);
				}
				place = move(v);
				break;*/
			}
		}
	}
	
	void AnyType::serialize(const Any& place, ArchiveNode& n, UniverseBase& u) const {
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
