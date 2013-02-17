//
//  property_animator.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "object/property_animator.hpp"
#include "object/reflect.hpp"
#include "base/anim_utils.hpp"

namespace falling {
	BEGIN_TYPE_INFO(PropertyAnimator)
	REFLECT_AUTO_LIST(PropertyAnimator, link);
	property(&PropertyAnimator::object, &PropertyAnimator::set_object, "object", "The object that owns the property to be animated.");
	property(&PropertyAnimator::property, &PropertyAnimator::set_property, "property", "The name of the property to be animated.");
	property(&PropertyAnimator::from_, "from", "Initial value (may be nothing).");
	property(&PropertyAnimator::to_, "to", "The target value.");
	property(&PropertyAnimator::duration_, "duration", "The duration of the animation.");
	//property(&PropertyAnimator::style_, "style", "The animation style.");
	//property(&PropertyAnimator::strategy_, "strategy", "The animation strategy.");
	
	slot(&PropertyAnimator::play, "play", "Play the animation.");
	slot(&PropertyAnimator::pause, "pause", "Pause the animation.");
	slot(&PropertyAnimator::stop, "stop", "Stop and reset the animation.");
	slot(&PropertyAnimator::reset, "reset", "Reset the animation without changing the playback state.");
	END_TYPE_INFO()
	
	void PropertyAnimator::update(GameTimeDelta delta) {
		if (state_ == PropertyAnimationState::Playing) {
			if (anim_ == nullptr) {
				anim_ = make_animator_for_current_property();
			}
			if (anim_ != nullptr) {
				anim_->update(delta);
			}
		}
	}
	
	void PropertyAnimator::play() {
		state_ = PropertyAnimationState::Playing;
	}
	
	void PropertyAnimator::pause() {
		state_ = PropertyAnimationState::Paused;
	}
	
	void PropertyAnimator::stop() {
		state_ = PropertyAnimationState::Stopped;
		reset();
	}
	
	void PropertyAnimator::reset() {
		anim_ = nullptr;
	}
	
	ObjectPtr<> PropertyAnimator::object() const {
		return object_;
	}
	
	void PropertyAnimator::set_object(ObjectPtr<> obj) {
		object_ = obj;
		reset();
	}
	
	StringRef PropertyAnimator::property() const {
		return property_name_;
	}
	
	void PropertyAnimator::set_property(String name) {
		property_name_ = move(name);
		reset();
	}
	
	PropertyAnimationState PropertyAnimator::TypedAnimatorImplBase::update(GameTimeDelta delta) {
		float64 progress_ratio = (float64)delta.microseconds() / (float64)remaining.microseconds();
		do_update(progress_ratio);
		if (delta > remaining) {
			return PropertyAnimationState::Stopped;
		} else {
			remaining -= delta;
			return PropertyAnimationState::Playing;
		}
	}
	
	template <typename T>
	struct PropertyAnimatorImpl : PropertyAnimator::TypedAnimatorImplBase {
		const AttributeOfType<T>* attr;
		T to;
		void do_update(float64 progress_ratio) override {
			T from;
			attr->get_polymorphic(object.get(), from);
			T value = anim_lerp(from, to, progress_ratio, progress_accum);
			attr->set_polymorphic(object.get(), value);
		}
	};
	
#define FOR_EACH_SUPPORTED_INTERPOLATABLE_PROPERTY_TYPE(F) \
	F(float32) \
	F(float64) \
	F(int32) \
	F(int64) \
	F(vec2) \
	F(vec3) \
	F(vec4) \
	F(matrix22) \
	F(matrix33) \
	F(matrix43) \
	F(matrix44) \
	F(Color)
	
	
	UniquePtr<PropertyAnimator::TypedAnimatorImplBase> PropertyAnimator::make_animator_for_current_property() {
		if (object_ == nullptr) return nullptr;
		auto type = dynamic_cast<const ObjectTypeBase*>(object_->object_type());
		if (type == nullptr) {
			Warning() << "Cannot animate properties on composite objects (yet).";
			return nullptr;
		}
		
		const IAttribute* attr = nullptr;
		for (auto a: type->attributes()) {
			if (a->name() == property_name_) {
				attr = a;
				break;
			}
		}
		if (attr == nullptr) {
			Warning() << "Property '" << property_name_ << "' not found on object of type " << type->name() << ".";
			return nullptr;
		}
		
		ObjectPtr<> real_object = object_;
		const IAttribute* real_attribute = attr;
		resolve_exposed_attribute(real_attribute, real_object);
		
		// My kingdom for unintrusive dynamic dispatch...
		UniquePtr<TypedAnimatorImplBase> result;
		const Type* attr_type = attr->type();
		if (false) {}
#define HANDLE_ATTRIBUTE_TYPE(T) \
		else if (attr_type == get_type<T>()) { \
			auto attr_t = dynamic_cast<const AttributeOfType<T>*>(real_attribute); \
			if (attr_t == nullptr) { \
				Warning() << "Cannot animate property representing type " << attr_type->name() << " but not derived from AttributeOfType<" << get_type<T>()->name() << ">."; \
				return nullptr; \
			} \
			auto u = make_unique<PropertyAnimatorImpl<T>>(default_allocator()); \
			u->attr = attr_t; \
		}
		FOR_EACH_SUPPORTED_INTERPOLATABLE_PROPERTY_TYPE(HANDLE_ATTRIBUTE_TYPE);
		
		if (result) {
			result->object = real_object;
			result->remaining = GameTime::microseconds(duration_);
			result->strategy = strategy_;
			result->style = style_;
		}
		return move(result);
	}
}
