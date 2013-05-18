//
//  spatial_object.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "object/spatial_object.hpp"
#include "type/vector_type.hpp"
#include "object/reflect.hpp"
#include "base/geometry.hpp"
#include "render/renderer.hpp"
#include "render/path.hpp"
#include "render/vertex.hpp"

namespace falling {
	BEGIN_TYPE_INFO(SpatialObject)
		super<Renderable>();
		property(&SpatialObject::position, &SpatialObject::set_position, "position", "World Position");
		property(&SpatialObject::size, &SpatialObject::set_size, "size");
		property(&SpatialObject::scale, &SpatialObject::set_scale, "scale");
		property(&SpatialObject::rotation, &SpatialObject::set_rotation, "rotation");
	END_TYPE_INFO()
	
	vec2 SpatialObject::get_focus_point() const {
		Rect b = bounds();
		return b.origin + b.size / vec2::two();
	}
	
	Rect SpatialObject::bounds() const {
		return Rect(position(), size() * scale());
	}
	
	vec2 SpatialObject::position() const {
		return position_;
	}
	
	void SpatialObject::set_position(vec2 p) {
		position_ = p;
	}
	
	vec2 SpatialObject::size() const {
		return size_;
	}
	
	void SpatialObject::set_size(vec2 s) {
		size_ = s;
	}
	
	vec2 SpatialObject::scale() const {
		return scale_;
	}
	
	void SpatialObject::set_scale(vec2 v2) {
		scale_ = v2;
	}
	
	float32 SpatialObject::rotation() const {
		return rotation_;
	}
	
	void SpatialObject::set_rotation(float32 rot) {
		rotation_ = rot;
	}
	
	void SpatialObject::debug_render(Renderer& r) {
		auto rect = bounds();
		Path<SolidVertex2> path = make_solid_rect(default_allocator(), vec2::zero(), rect.size, Color(1, 1, 1, 0.1));
		r.move_to(rect.origin);
		r.fill_path(path);
	}
}
