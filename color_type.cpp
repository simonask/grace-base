//
//  color_type.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 01/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "type/color_type.hpp"
#include "serialization/document_node.hpp"

namespace grace {
	
	void ColorType::deserialize(Color& color, const DocumentNode& node, IUniverse&) const {
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
	
	void ColorType::serialize(const Color& color, DocumentNode& node, IUniverse&) const {
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
	
	StringRef CompactColorType::name() const {
		return "CompactColor";
	}
	
	void CompactColorType::deserialize(CompactColor &place, const DocumentNode & node, IUniverse & universe) const {
		Color c;
		get_type<Color>()->deserialize(c, node, universe);
		place = c;
	}
	
	void CompactColorType::serialize(const CompactColor& place, DocumentNode& node, IUniverse& universe) const {
		Color c = place;
		get_type<Color>()->serialize(c, node, universe);
	}
	
	const ColorType* BuildTypeInfo<Color>::build() {
		static const ColorType type;
		return &type;
	}
	const CompactColorType* BuildTypeInfo<CompactColor>::build() {
		static const CompactColorType type;
		return &type;
	}
}