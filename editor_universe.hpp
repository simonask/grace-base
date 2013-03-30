//
//  editor_universe.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 21/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __falling__editor_universe__
#define __falling__editor_universe__

#include "object/universe_base.hpp"
#include "object/object_ptr_root.hpp"

namespace falling {
	class EditorUniverse : public UniverseBase {
	public:
		// EditorUniverse API
		EditorUniverse();
		~EditorUniverse();
		void register_object_root(ObjectPtrRootBase* root);
		void unregister_object_root(void* root);
	
		// Runtime API
		ObjectPtr<> create_object(const StructuredType* type, StringRef obj_id) final;
		ObjectPtr<> create_object_and_set_as_root(const StructuredType* type, StringRef obj_id) final;
		ObjectPtr<> get_object(StringRef obj_id) const final;
		StringRef get_id(ObjectPtr<const Object> object) const final;
		ObjectPtr<> root() const final;
		void set_root(const ObjectPtr<> root) final;
		void clear() final;
		
		// Editor/dev-time API
		bool rename_object(ObjectPtr<>, StringRef new_id) final;
		bool recreate_object_and_initialize(const ArchiveNode& object_definition, StringRef object_id) final;
		
		// Serialization API
		void run_initializers() final;
		bool serialize_scene(ArchiveNode& out_scene, String& out_error) final;
	private:
		struct Impl;
		Impl* impl_ = nullptr;
	};
}

#endif /* defined(__falling__editor_universe__) */
