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
	
	void ColorType::deserialize(Color& color, const ArchiveNode& node, IUniverse&) const {
		if (node.is_map()) {
			auto& r = node["r"];
			auto& g = node["g"];
			auto& b = node["b"];
			if (r.get(color.red()) && g.get(color.green()) && b.get(color.blue())) {
				auto& a = node["a"];
				a.get(color.alpha());
				// Success!
				return;
			}
		}
		color = Color::Pink;
	}
	
	void ColorType::serialize(const Color& color, ArchiveNode& node, IUniverse&) const {
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