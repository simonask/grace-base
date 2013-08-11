//
//  spatial_object.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "object/spatial_object.hpp"
#include "type/vector_type.hpp"
#include "object/reflect.hpp"
#include "geometry/2d.hpp"
#include "render/renderer.hpp"
#include "render/path.hpp"
#include "render/vertex.hpp"

namespace grace {
	BEGIN_TYPE_INFO(SpatialObject)
		super<Renderable>();
		property(&SpatialObject::position, &SpatialObject::set_position, "position", "World Position");
		property(&SpatialObject::size, &SpatialObject::set_size, "size");
		property(&SpatialObject::scale, &SpatialObject::set_scale, "scale");
		property(&SpatialObject::rotation, &SpatialObject::set_rotation, "rotation");
		property(&SpatialObject::rotation_origin, &SpatialObject::set_rotation_origin, "rotation_origin");
		property(&SpatialObject::transform_parent, &SpatialObject::set_transform_parent, "transform_parent");
		property(&SpatialObject::transform, nullptr, "transform");
		property(&SpatialObject::local_transform, nullptr, "local_transform");
	END_TYPE_INFO()
	
	void SpatialObject::initialize() {
		Renderable::initialize();
		if (transform_parent_ == nullptr) {
			reset_transform_parent();
		}
	}
	
	vec2 SpatialObject::get_focus_point() const {
		Rect b = bounds();
		return b.origin + b.size / vec2::two();
	}
		
	Rect SpatialObject::bounds() const {
		return bounds_of_rotated_rectangle(Rect{position(), size()}, rotation());
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
	
	Degrees SpatialObject::rotation() const {
		return rotation_;
	}
	
	void SpatialObject::set_rotation(Degrees rot) {
		rotation_ = rot;
	}
	
	vec2 SpatialObject::rotation_origin() const {
		return rotation_origin_;
	}
	
	void SpatialObject::set_rotation_origin(vec2 offset) {
		rotation_origin_ = offset;
	}
	
	ObjectPtr<SpatialObject> SpatialObject::transform_parent() const {
		return transform_parent_;
	}
	
	void SpatialObject::set_transform_parent(ObjectPtr<SpatialObject> parent) {
		ObjectPtr<SpatialObject> p = parent;
		while (p != nullptr) {
			if (p.get() == this) {
				Warning() << "Spatial transform parent of '" << object_id() << "' is cyclic -- setting to null.";
				transform_parent_ = nullptr;
				return;
			}
			p = p->transform_parent();
		}
		
		transform_parent_ = parent;
	}
	
	void SpatialObject::reset_transform_parent() {
		transform_parent_ = nullptr;
		if (is_aspect_in_composite()) {
			Object* p = find_parent();
			while (p != nullptr) {
				SpatialObject* ps = dynamic_cast<SpatialObject*>(p);
				if (ps != nullptr) {
					transform_parent_ = ObjectPtr<SpatialObject>(ps);
					return;
				}
				p = p->find_parent();
			}
		}
	}
	
	matrix33 SpatialObject::transform() const {
		if (transform_parent_) {
			return local_transform() * transform_parent_->transform();
		} else {
			return local_transform();
		}
	}
	
	matrix33 SpatialObject::local_transform() const {
		return make_2d_transform_matrix(position(), scale(), rotation());
	}
}
