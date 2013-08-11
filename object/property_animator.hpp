//
//  property_animator.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 29/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_property_animator_hpp
#define grace_property_animator_hpp

#include "object/object.hpp"
#include "object/objectptr.hpp"
#include "base/any.hpp"
#include "base/auto_link_queue.hpp"
#include "memory/unique_ptr.hpp"

namespace grace {
	enum PropertyAnimationStyle : uint8 {
		LinearInterpolation,
		// TODO: Add more
	};
	
	enum PropertyAnimationStrategy : uint8 {
		Once,
		Loop,
		PingPong, // TODO: Better name?
	};
	
	enum PropertyAnimationState : uint8 {
		Stopped,
		Paused,
		Playing,
	};
	
	class PropertyAnimator : public Object {
		REFLECT;
	public:
		void update(GameTimeDelta delta);
		
		void play();
		void pause();
		void stop();
		void reset();
		
		ObjectPtr<> object() const;
		void set_object(ObjectPtr<> obj);
		StringRef property() const;
		void set_property(String name);
		
		
		struct TypedAnimatorImplBase {
			ObjectPtr<> object;
			GameTimeDelta remaining;
			PropertyAnimationStrategy strategy;
			PropertyAnimationStyle style;
			PropertyAnimationState update(GameTimeDelta);
			float64 progress_accum = 0;
		protected:
			virtual void do_update(float64 progress_ratio) = 0;
		};
	private:
		AutoListLink<PropertyAnimator> link;
		
		ObjectPtr<> object_;
		String property_name_;
		Any from_; // may be nothing, in which case the initial value is used.
		Any to_;
		int64 duration_; // in microsecs
		PropertyAnimationStyle style_ = PropertyAnimationStyle::LinearInterpolation;
		PropertyAnimationStrategy strategy_ = PropertyAnimationStrategy::Once;
		PropertyAnimationState state_ = PropertyAnimationState::Stopped;

		UniquePtr<TypedAnimatorImplBase> anim_;
		
		UniquePtr<TypedAnimatorImplBase> make_animator_for_current_property();
	};
}

#endif
