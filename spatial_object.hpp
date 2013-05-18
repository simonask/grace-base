//
//  spatial_object.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_spatial_object_hpp
#define falling_spatial_object_hpp

#include "object/object.hpp"
#include "base/vector.hpp"
#include "base/geometry.hpp"
#include "render/renderable.hpp"

namespace falling {
	class SpatialObject : public Renderable {
		REFLECT;
	public:
		virtual vec2 get_focus_point() const; // default: middle of bounds
		virtual Rect bounds() const;
		virtual vec2 position() const;
		virtual void set_position(vec2 p);
		virtual vec2 size() const;
		virtual void set_size(vec2);
		virtual vec2 scale() const;
		virtual void set_scale(vec2 v2);
		virtual float32 rotation() const;
		virtual void set_rotation(float32 rot);
		
		void debug_render(Renderer& render) override;
	private:
		vec2 position_ = vec2::zero();
		vec2 size_ = vec2::replicate(100.f);
		vec2 scale_ = vec2::one();
		float32 rotation_ = 0.f;
	};
}

#endif
