//
//  color_type.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 01/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "type/color_type.hpp"
#include "serialization/archive_node.hpp"

namespace falling {
	
	void ColorType::deserialize(Color& color, const ArchiveNode& node, UniverseBase&) const {
		if (node.is_map()) {
			node["r"].get(color.red());
			node["g"].get(color.green());
			node["b"].get(color.blue());
			if (!node["a"].get(color.alpha())) {
				color.alpha() = 1.f;
			}
			return;
		}
		color = Color::Pink;
	}
	
	void ColorType::serialize(const Color& color, ArchiveNode& node, UniverseBase&) const {
		auto& r = node["r"];
		auto& g = node["g"];
		auto& b = node["b"];
		auto& a = node["a"];
		r.set(color.red());
		g.set(color.green());
		b.set(color.blue());
		a.set(color.alpha());
	}
	
	std::string ColorType::name() const {
		return "Color";
	}
	
	const ColorType* BuildTypeInfo<Color>::build() {
		static const ColorType type;
		return &type;
	}
}