//
//  color.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 01/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_color_hpp
#define falling_color_hpp

#include "base/vector.hpp"

namespace falling {
	struct Color {
	public:
		typedef vec4 ColorComponents;
		typedef ColorComponents::ComponentType ComponentType;
		
		Color() {}
		Color(ComponentType r, ComponentType g, ComponentType b, ComponentType a = 1.0) : components_(r, g, b, a) {}
		Color(const Color& other) = default;
		explicit Color(ColorComponents components) : components_(components) {}
		Color& operator=(const Color&) = default;
		ColorComponents components() const { return components_; }
		
		bool operator==(const Color& other) const { return components_ == approximately(other.components_, 0.1f); }
		bool operator!=(const Color& other) const { return !(*this == other); }
		
		ComponentType  red() const   { return components_[0]; }
		ComponentType& red()         { return components_[0]; }
		ComponentType  green() const { return components_[1]; }
		ComponentType& green()       { return components_[1]; }
		ComponentType  blue() const  { return components_[2]; }
		ComponentType& blue()        { return components_[2]; }
		ComponentType  alpha() const { return components_[3]; }
		ComponentType& alpha()       { return components_[3]; }
		
		static const Color White;
		static const Color Black;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Pink;
		static const Color Cyan;
		static const Color Yellow;
		static const Color Transparent;
	private:
		ColorComponents components_;
	};
	
	inline Color interpolate(Color a, Color b, Color::ComponentType weight = 0.5) {
		auto diff = b.components() - a.components();
		auto weighted_diff = diff * Color::ColorComponents::replicate(weight); 
		return Color(a.components() + weighted_diff);
	}
	
	struct CompactColor {
		typedef uint32 ColorComponents;
		typedef byte ComponentType;
		
		static ComponentType denormalize(Color::ComponentType component) { return component * 255; }
		static Color::ComponentType normalize(ComponentType component) { return float32(component) / 255.f; }
		static ColorComponents combine_components(byte r, byte g, byte b, byte a) {
			ColorComponents c;
			uint8* p = reinterpret_cast<uint8*>(&c); // since uint8 is a char type, this doesn't actually break aliasing rules.
			p[0] = r;
			p[1] = g;
			p[2] = b;
			p[3] = a;
			return c;
		}
		static ColorComponents denormalize_all(Color components) {
			byte r = denormalize(components.red());
			byte g = denormalize(components.green());
			byte b = denormalize(components.blue());
			byte a = denormalize(components.alpha());
			return combine_components(r, g, b, a);
		}
		
		CompactColor() {}
		CompactColor(const CompactColor& other) = default;
		CompactColor(Color c) : components_(denormalize_all(c)) {}
		CompactColor(byte r, byte g, byte b, byte a = 255) : r(r), g(g), b(b), a(a) {}
		CompactColor(uint32 hex) : components_(hex) {}
		
		operator Color() const {
			float32 fr = normalize(r);
			float32 fg = normalize(g);
			float32 fb = normalize(b);
			float32 fa = normalize(a);
			return Color(fr, fg, fb, fa);
		}
		
		bool operator==(CompactColor other) const { return components_ == other.components_; }
		bool operator!=(CompactColor other) const { return components_ != other.components_; }
		
		byte red() const { return r; }
		byte& red() { return r; }
		byte green() const { return g; }
		byte& green() { return g; }
		byte blue() const { return b; }
		byte& blue() { return b; }
		byte alpha() const { return a; }
		byte& alpha() { return a; }
	private:
		union {
			ColorComponents components_;
			struct {
				byte r, g, b, a;
			};
		};
	};
}

#endif
