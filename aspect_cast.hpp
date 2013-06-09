//
//  aspect_cast.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_aspect_cast_hpp
#define grace_aspect_cast_hpp

#include "object/objectptr.hpp"
#include "object/composite_type.hpp"
#include "object/object_type_base.hpp"

namespace grace {
	template <typename To, typename From>
	typename std::enable_if<HasReflection<To>::Value && HasReflection<From>::Value, ObjectPtr<To>>::type
	aspect_cast(ObjectPtr<From> ptr) {
		return ObjectPtr<To>(aspect_cast<To>(ptr.get()));
	}
	
	template <typename To, typename From>
	typename std::enable_if<HasReflection<From>::Value && !HasReflection<To>::Value, To*>::type
	aspect_cast(ObjectPtr<From> ptr) {
		return aspect_cast<To>(ptr.get());
	}
	
	template <typename From>
	ObjectPtr<>
	aspect_cast(ObjectPtr<From> ptr, const DerivedType* type) {
		return ObjectPtr<>(aspect_cast(ptr.get(), type));
	}
	
	template <typename To, typename From>
	typename std::enable_if<HasReflection<To>::Value && HasReflection<From>::Value, To*>::type
	aspect_composite_cast(From* from) {
		if (from == nullptr) return nullptr;
		// At this point, we should've already checked that we can't directly up- or downcast,
		// so let's see if we're a composite.
		
		const StructuredType* from_type = from->object_type();
		const ObjectTypeBase* to_type = get_type<To>();
		
		const CompositeType* from_composite_type = dynamic_cast<const CompositeType*>(from_type);
		if (from_composite_type) {
			// from is a composite, so see if it has an aspect matching To.
			Object* found = from_composite_type->find_aspect_of_type(from, to_type);
			if (found != nullptr) {
				return dynamic_cast<To*>(found);
			}
		}
		
		Object* composite_parent = from->find_parent();
		const StructuredType* skip_aspect_search = from_type;
		while (composite_parent != nullptr) {
			// from is an aspect in a composite object.
			const CompositeType* composite_parent_type = dynamic_cast<const CompositeType*>(composite_parent->object_type());
			ASSERT(composite_parent_type != nullptr); // find_parent returned non-NULL, but parent is not a composite!
			Object* found = nullptr;
			
			// Check if the base type of the composite object is what we're looking for:
			To* f = dynamic_cast<To*>(composite_parent);
			if (f != nullptr) {
				return f;
			}
			
			// It wasn't, check if any of the siblings were what we're looking for:
			found = composite_parent_type->find_aspect_of_type(composite_parent, to_type, skip_aspect_search);
			if (found != nullptr) {
				return dynamic_cast<To*>(found);
			} else {
				// None of the siblings matched, so check siblings of parents:
				skip_aspect_search = composite_parent_type;
				composite_parent = composite_parent->find_parent();
			}
		}
		
		return nullptr;
	}
	
	template <typename To, typename From>
	typename std::enable_if<HasReflection<From>::Value && !HasReflection<To>::Value, To*>::type
	aspect_composite_cast(From* from) {
		if (from == nullptr) return nullptr;
		// At this point, we should've already checked that we can't directly up- or downcast,
		// so let's see if we're a composite.
		
		const StructuredType* from_type = from->object_type();
		const CompositeType* from_composite_type = dynamic_cast<const CompositeType*>(from_type);
		if (from_composite_type) {
			// Check if any aspect in the object can be cast to the interface
			for (size_t i = 0; i < from_composite_type->num_aspects(); ++i) {
				Object* aspect = from_composite_type->get_aspect_in_object(from, i);
				To* dyn_to = dynamic_cast<To*>(aspect);
				if (dyn_to != nullptr) {
					return dyn_to;
				}
			}
		}
		
		// We're not a composite ourselves, but we may part of another composite, so try upcasting
		Object* composite_parent = from->find_parent();
		Object* skip_aspect_search = from;
		while (composite_parent != nullptr) {
			// See if parent composite implements interface (upcast)
			To* dyn_to = dynamic_cast<To*>(composite_parent);
			if (dyn_to != nullptr) {
				return dyn_to;
			}
		
			// Check other aspects (sidecast)
			const CompositeType* composite_parent_type = dynamic_cast<const CompositeType*>(composite_parent->object_type());
			ASSERT(composite_parent_type != nullptr); // find_parent returned non-NULL, but parent is not a composite!
			for (size_t i = 0; i < composite_parent_type->num_aspects(); ++i) {
				Object* aspect = composite_parent_type->get_aspect_in_object(composite_parent, i);
				if (aspect == skip_aspect_search) continue;
				dyn_to = dynamic_cast<To*>(aspect);
				if (dyn_to != nullptr) {
					return dyn_to;
				}
			}
		}
		
		return nullptr;
	}

	
	// composite sidecast, potential upcast
	template <typename To, typename From>
	typename std::enable_if<
		!std::is_same<To, From>::value
		&& HasReflection<To>::Value && HasReflection<From>::Value
		&& !std::is_convertible<From*, To*>::value
		&& std::is_convertible<To*, From*>::value
		, To*>::type
	aspect_cast(From* ptr) {
		To* dyn_to = dynamic_cast<To*>(ptr);
		if (dyn_to != nullptr) return dyn_to;
		return aspect_composite_cast<To>(ptr);
	}
	
	// composite sidecast, no upcast
	template <typename To, typename From>
	typename std::enable_if<
		!std::is_same<To, From>::value
		&& HasReflection<To>::Value && HasReflection<From>::Value
		&& !std::is_convertible<From*, To*>::value
		&& !std::is_convertible<To*, From*>::value
		, To*>::type
	aspect_cast(From* ptr) {
		return aspect_composite_cast<To>(ptr);
	}
	
	// cast from reflected to nonreflected type
	template <typename To, typename From>
	typename std::enable_if<
		!std::is_same<To, From>::value
		&& !std::is_convertible<From*, To*>::value
		&& HasReflection<From>::Value && !HasReflection<To>::Value
		, To*>::type
	aspect_cast(From* ptr) {
		To* dyn_to = dynamic_cast<To*>(ptr);
		if (dyn_to != nullptr) return dyn_to;
		return aspect_composite_cast<To>(ptr);
	}
	
	// dynamic_cast for non-reflected types
	template <typename To, typename From>
	typename std::enable_if<
		!std::is_same<To, From>::value
		&& !HasReflection<To>::Value && !HasReflection<From>::Value
		&& !std::is_convertible<From*, To*>::value
		&& std::is_convertible<To*, From*>::value
	, To*>::type
	aspect_cast(From* ptr) {
		return dynamic_cast<To*>(ptr);
	}
	
	// static_cast
	template <typename To, typename From>
	typename std::enable_if<
		std::is_convertible<From*, To*>::value
	, To*>::type
	aspect_cast(From* ptr) {
		return static_cast<To*>(ptr);
	}
}

#endif
