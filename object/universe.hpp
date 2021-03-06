#pragma once
#ifndef UNIVERSE_HPP_VHU9428R
#define UNIVERSE_HPP_VHU9428R

#include "base/string.hpp"
#include "object/objectptr.hpp"

namespace grace {
	class Object;
	struct StructuredType;
	struct DeserializeReferenceBase;
	struct SerializeReferenceBase;
	struct DeserializeSignalBase;
	struct IAttribute;
	struct IEventLoop;

	struct IUniverse {
		// Runtime API
		virtual ObjectPtr<> create_object(const StructuredType* type, StringRef id) = 0;
		virtual ObjectPtr<> create_object_and_set_as_root(const StructuredType* type, StringRef id) = 0;
		virtual ObjectPtr<> get_object(StringRef object_id) const = 0;
		virtual StringRef get_id(ObjectPtr<const Object> object) const = 0;
		virtual ObjectPtr<> root() const = 0;
		virtual void set_root(const ObjectPtr<> root) = 0;
		virtual void clear() = 0;
		virtual IAllocator& allocator() const = 0;
		virtual void update(GameTimeDelta delta) = 0; // Update all objects in universe
		virtual void register_object_for_update(ObjectPtr<> obj) = 0;
		virtual void unregister_object_for_update(ObjectPtr<> obj) = 0;
		
		// Editor/dev-time API
		virtual bool rename_object(ObjectPtr<>, StringRef new_id) = 0;
		virtual bool recreate_object_and_initialize(const DocumentNode& node, StringRef object_id) = 0;
		
		// Serialization API
		virtual void run_initializers() = 0;
		virtual bool instantiate(const DocumentNode& scene_root, String& out_error) = 0;
		virtual bool serialize_scene(DocumentNode& scene_definition, String& out_error) = 0;
		virtual void defer_attribute_deserialization(ObjectPtr<> object, const IAttribute* attr, const DocumentNode* serialized) = 0;
		
		// Event loop integration API
		virtual void set_event_loop(IEventLoop* event_loop) = 0;
		virtual IEventLoop* event_loop() const = 0;
	};
}

#endif /* end of include guard: UNIVERSE_HPP_VHU9428R */
