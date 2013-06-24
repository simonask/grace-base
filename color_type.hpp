//
//  color_type.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 01/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_color_type_hpp
#define grace_color_type_hpp

#include "base/color.hpp"
#include "type/type.hpp"

namespace grace {
	struct DocumentNode;
	struct IUniverse;
	
	struct ColorType : TypeFor<Color> {
		ColorType() {}
		virtual void deserialize(Color& place, const DocumentNode&, IUniverse&) const;
		virtual void serialize(const Color& place, DocumentNode&, IUniverse&) const;
		
		StringRef name() const;
	};
	
	struct CompactColorType : TypeFor<CompactColor> {
		CompactColorType() {}
		virtual void deserialize(CompactColor& place, const DocumentNode&, IUniverse&) const;
		virtual void serialize(const CompactColor& place, DocumentNode&, IUniverse&) const;
		StringRef name() const;
	};
	
	template <>
	struct BuildTypeInfo<Color> {
		static const ColorType* build();
	};
	
	template <>
	struct BuildTypeInfo<CompactColor> {
		static const CompactColorType* build();
	};
}

#endif
