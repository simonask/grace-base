//
//  geometry.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 20/10/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/geometry.hpp"
#include "type/vector_type.hpp"

namespace falling {
	namespace detail {
		struct RectType : TypeFor<Rect> {
			std::string name() const { return "Rect"; }
		
			virtual void deserialize(Rect& place, const ArchiveNode& node, IUniverse& universe) const {
				auto vt = get_type<vec2>();
				vt->deserialize(place.origin, node["origin"], universe);
				vt->deserialize(place.size, node["size"], universe);
			}
			
			virtual void serialize(const Rect& place, ArchiveNode&, IUniverse&) const {
				
			}
		};

		const Type* build_rect_type() {
			static const RectType* t = new RectType;
			return t;
		}
	}
}
