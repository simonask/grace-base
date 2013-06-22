//
//  spatial_object.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_spatial_object_hpp
#define grace_spatial_object_hpp

#include "object/object.hpp"
#include "base/vector.hpp"
#include "base/geometry.hpp"
#include "render/renderable.hpp"
#include "object/objectptr.hpp"

namespace grace {
	class SpatialObject : public Renderable {
		REFLECT;
	public:
		void initialize() override;
	
		virtual vec2 get_focus_point() const; // default: middle of bounds
		virtual Rect bounds() const;
		virtual vec2 position() const;
		virtual void set_position(vec2 p);
		virtual vec2 size() const;
		virtual void set_size(vec2);
		virtual vec2 scale() const;
		virtual void set_scale(vec2 v2);
		virtual Degrees rotation() const;
		virtual void set_rotation(Degrees rot);
		virtual vec2 rotation_origin() const;
		virtual void set_rotation_origin(vec2 offset);
		
		/*
		 Transform parent can be:
		 1) Manually set as a property.
		 2) If not manually set, it is initialized to the closest SpatialObject
		    in the containing composites.
		 3) If it isn't contained in a spatial object, it is a toplevel object,
		    and transform_parent is NULL.
		*/
		ObjectPtr<SpatialObject> transform_parent() const;
		void set_transform_parent(ObjectPtr<SpatialObject>);
		void reset_transform_parent(); // like set_transform_parent(nullptr), but tries to find a parent in the composite.
		
		matrix33 transform() const; // global transform
		matrix33 local_transform() const; // transform without parents
	private:
		vec2 position_ = vec2::zero();
		vec2 size_ = vec2::replicate(100.f);
		vec2 scale_ = vec2::one();
		Degrees rotation_ = 0_deg;
		vec2 rotation_origin_ = vec2::zero();
		
		ObjectPtr<SpatialObject> transform_parent_;
	};
}

#endif
