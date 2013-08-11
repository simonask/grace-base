//
//  rect.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/08/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#include "geometry/rect.hpp"
#include "type/type.hpp"
#include "type/vector_type.hpp"

namespace grace {
	namespace detail {
		struct RectType : TypeFor<Rect> {
			StringRef name() const { return "Rect"; }
			
			virtual void deserialize(Rect& place, const DocumentNode& node, IUniverse& universe) const {
				auto vt = get_type<vec2>();
				vt->deserialize(place.origin, node["origin"], universe);
				vt->deserialize(place.size, node["size"], universe);
			}
			
			virtual void serialize(const Rect& place, DocumentNode&, IUniverse&) const {
				
			}
		};
		
		const IType* build_rect_type() {
			static const RectType* t = new RectType;
			return t;
		}
	}
}