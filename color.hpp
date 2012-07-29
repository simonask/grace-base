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
#include <initializer_list>

namespace falling {
	struct Color {
	public:
		typedef vec4 ColorComponents;
		typedef ColorComponents::ComponentType ComponentType;
		
		Color() {}
		constexpr Color(ComponentType r, ComponentType g, ComponentType b, ComponentType a = 1.0) : components_(r, g, b, a) {}
		Color(const Color& other) = default;
		explicit Color(ColorComponents components) : components_(components) {}
		Color& operator=(const Color&) = default;
		ColorComponents components() const { return components_; }
		
		ComponentType  red() const   { return components_[0]; }
		ComponentType& red()         { return components_[0]; }
		ComponentType  green() const { return components_[1]; }
		ComponentType& green()       { return components_[1]; }
		ComponentType  blue() const  { return components_[2]; }
		ComponentType& blue()        { return components_[2]; }
		ComponentType  alpha() const { return components_[3]; }
		ComponentType& alpha()       { return components_[3]; }
		
		template <typename Function>
		auto pass_as_rgb(Function function) const
		-> decltype(function(ComponentType(), ComponentType(), ComponentType()))
		{
			return function(red(), green(), blue());
		}
		
		template <typename Function>
		auto pass_as_rgba(Function function) const
		-> decltype(function(ComponentType(), ComponentType(), ComponentType(), ComponentType()))
		{
			return function(red(), green(), blue(), alpha());
		}
		
		template <typename Function>
		auto pass_as_bgr(Function function) const
		-> decltype(function(ComponentType(), ComponentType(), ComponentType()))
		{
			return function(blue(), green(), red());
		}
		
		template <typename Function>
		auto pass_as_bgra(Function function) const
		-> decltype(function(ComponentType(), ComponentType(), ComponentType(), ComponentType()))
		{
			return function(blue(), green(), red(), alpha());
		}
		
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
}

#endif
