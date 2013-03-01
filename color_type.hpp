//
//  color_type.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 01/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_color_type_hpp
#define falling_color_type_hpp

#include "base/color.hpp"
#include "type/type.hpp"

namespace falling {
	struct ArchiveNode;
	struct IUniverse;
	
	struct ColorType : TypeFor<Color> {
		ColorType() {}
		virtual void deserialize(Color& place, const ArchiveNode&, IUniverse&) const;
		virtual void serialize(const Color& place, ArchiveNode&, IUniverse&) const;
		
		StringRef name() const;
	};
	
	struct CompactColorType : TypeFor<CompactColor, ColorType> {
		CompactColorType() {}
		virtual void deserialize(CompactColor& place, const ArchiveNode&, IUniverse&) const;
		virtual void serialize(const CompactColor& place, ArchiveNode&, IUniverse&) const;
	};
	
	template <>
	struct BuildTypeInfo<Color> {
		static const ColorType* build();
	};
	
	template <>
	struct BuildTypeInfo<CompactColor> {
		static const ColorType* build();
	};
}

#endif
