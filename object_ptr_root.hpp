//
//  object_ptr_root.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_object_ptr_root_hpp
#define falling_object_ptr_root_hpp

#include "object/objectptr.hpp"
#include "object/aspect_cast.hpp"

namespace falling {
	struct ObjectPtrRootBase {
		virtual void set(ObjectPtr<> root) const = 0;
		virtual ObjectPtr<> get() const = 0;
		virtual void* ptr() const = 0;
	};
	template <typename T>
	struct ObjectPtrRoot : ObjectPtrRootBase {
		ObjectPtr<T>* root;
		ObjectPtrRoot(ObjectPtr<T>* root) : root(root) {}
		void set(ObjectPtr<> r) const final {
			*root = aspect_cast<T>(r);
		}
		ObjectPtr<> get() const final {
			return *root;
		}
		void* ptr() const final {
			return root;
		}
	};
	
	
	void register_object_root_impl(ObjectPtrRootBase* root_descriptor, IUniverse& universe);
	void unregister_object_root_impl(void* root, IUniverse& universe);
	
	template <typename T>
	void register_object_root(ObjectPtr<T>* root, IUniverse& universe) {
#if !defined(DISABLE_EDITOR_FEATURES)
		register_object_root_impl(new(default_allocator()) ObjectPtrRoot<T>(root), universe);
#endif
	}
	template <typename T>
	void unregister_object_root(ObjectPtr<T>* root, IUniverse& universe) {
#if !defined(DISABLE_EDITOR_FEATURES)
		unregister_object_root_impl(root, universe);
#endif
	}

}

#endif
