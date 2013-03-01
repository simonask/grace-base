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
			node["r"] >> color.red();
			node["g"] >> color.green();
			node["b"] >> color.blue();
			if (!(node["a"] >> color.alpha())) {
				color.alpha() = 1.f;
			}
			return;
		}
		color = Color::Pink;
	}
	
	void ColorType::serialize(const Color& color, ArchiveNode& node, IUniverse&) const {
		auto& r = node["r"];
		auto& g = node["g"];
		auto& b = node["b"];
		auto& a = node["a"];
		r << color.red();
		g << color.green();
		b << color.blue();
		a << color.alpha();
	}
	
	StringRef ColorType::name() const {
		return "Color";
	}
	
	void CompactColorType::deserialize(CompactColor &place, const ArchiveNode & node, IUniverse & universe) const {
		Color c;
		const ColorType* t = this;
		t->deserialize(c, node, universe);
		place = c;
	}
	
	void CompactColorType::serialize(const CompactColor& place, ArchiveNode& node, IUniverse& universe) const {
		Color c = place;
		const ColorType* t = this;
		t->serialize(c, node, universe);
	}
	
	const ColorType* BuildTypeInfo<Color>::build() {
		static const ColorType type;
		return &type;
	}
	const ColorType* BuildTypeInfo<CompactColor>::build() {
		static const CompactColorType type;
		return &type;
	}
}