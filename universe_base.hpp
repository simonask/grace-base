//
//  universe_base.h
//  falling
//
//  Created by Simon Ask Ulsnes on 08/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __falling__universe_base__
#define __falling__universe_base__

#include "object/universe.hpp"
#include "memory/unique_ptr.hpp"
#include "object/aspect_cast.hpp"

namespace falling {
	struct CompositeType;

	struct DeferredAttributeDeserialization {
		ObjectPtr<> object;
		const IAttribute* attribute;
		const ArchiveNode* node;
		
		void perform(IUniverse& universe) const;
	};

	struct UniverseBase : public IUniverse {
		virtual ~UniverseBase();
		
		// IUniverse interface (partial)
		bool instantiate(const ArchiveNode& scene_definition, String& out_error) final;
		void defer_attribute_deserialization(ObjectPtr<> obj, const IAttribute* attr, const ArchiveNode* serialized) final;
		IAllocator& allocator() const final { return allocator_; }
		
		// UniverseBase interface
		CompositeType* create_composite_type(const ObjectTypeBase* base, StringRef name = "");
		
		template <typename T>
		ObjectPtr<T> create(String id) {
			ObjectPtr<> o = this->create_object(get_type<T>(), std::move(id));
			ObjectPtr<T> ptr = aspect_cast<T>(o);
			ASSERT(ptr != nullptr); // create_object did not create an instance of T.
			return ptr;
		}
		
		template <typename T>
		ObjectPtr<T> create_root(String id) {
			ObjectPtr<> o = this->create_object_and_set_as_root(get_type<T>(), std::move(id));
			ObjectPtr<T> ptr = aspect_cast<T>(o);
			ASSERT(ptr != nullptr);
			return ptr;
		}
		
		template <typename T, size_t MemberOffset>
		AutoList<T, MemberOffset>& get_auto_list();
		
		template <typename AutoListType>
		AutoListType& get_auto_list() {
			return get_auto_list<typename AutoListType::ValueType, AutoListType::LinkOffset>();
		}
		
		void set_event_loop(IEventLoop* el) final { event_loop_ = el; }
		IEventLoop* event_loop() const final { return event_loop_; }
	protected:
		UniverseBase(IAllocator& alloc) : allocator_(alloc), auto_lists(alloc), deferred_(alloc), composite_types_(alloc) {}
		Array<DeferredAttributeDeserialization> deferred_;
		Array<CompositeType*> composite_types_;
	private:
		IAllocator& allocator_;
		Map<const StructuredType*, Map<size_t, VirtualAutoListBase*>> auto_lists;
		IEventLoop* event_loop_ = nullptr;
	};
	
	void error_category_already_initialized_with_different_type(StringRef name);
	void warn_attempt_to_get_objects_of_unindexed_type(const ObjectTypeBase*);
	
	template <typename T, size_t MemberOffset>
	AutoList<T, MemberOffset>& UniverseBase::get_auto_list() {
		VirtualAutoListBase* base_ptr = nullptr;
		const StructuredType* type = get_type<T>();
		auto it1 = auto_lists.find(type);
		if (it1 != auto_lists.end()) {
			auto& map2 = it1->second;
			auto it2 = map2.find(MemberOffset);
			if (it2 != map2.end()) {
				base_ptr = it2->second;
			} else {
				base_ptr = new VirtualAutoList<T, MemberOffset>;
				map2[MemberOffset] = base_ptr;
			}
		} else {
			base_ptr = new VirtualAutoList<T, MemberOffset>;
			Map<size_t, VirtualAutoListBase*> m = {{MemberOffset, base_ptr}};
			auto_lists[type] = std::move(m);
		}
		
		AutoList<T, MemberOffset>* ptr = nullptr;
#if DEBUG
		ptr = dynamic_cast<VirtualAutoList<T, MemberOffset>*>(base_ptr);
		ASSERT(ptr != nullptr);
#else
		ptr = static_cast<VirtualAutoList<T, MemberOffset>*>(base_ptr);
#endif
		return *ptr;
	}
	
	struct BasicUniverse : UniverseBase {
		// IUniverse interface:
		ObjectPtr<> create_object(const StructuredType* type, StringRef) final;
		ObjectPtr<> create_object_and_set_as_root(const StructuredType* type, StringRef) final;
		ObjectPtr<> get_object(StringRef object_id) const final {
			return find_or(object_map_, object_id, nullptr);
		}
		StringRef get_id(ObjectPtr<const Object> object) const final;
		bool rename_object(ObjectPtr<> object, StringRef) final;
		bool recreate_object_and_initialize(const ArchiveNode& node, StringRef object_id) final;
		ObjectPtr<> root() const final { return root_; }
		void set_root(ObjectPtr<> r) final {
			ASSERT(r != nullptr && this == r->universe());
			root_ = r;
		}
		bool serialize_scene(ArchiveNode& root, String& out_error) final;
		void run_initializers() final;
		void clear() final;
		
		BasicUniverse(IAllocator& alloc = default_allocator()) : UniverseBase(alloc), object_map_(alloc), reverse_object_map_(alloc), memory_map_(alloc) {}
		~BasicUniverse() { clear(); }
	private:
		Map<String, ObjectPtr<>> object_map_;
		Map<ObjectPtr<const Object>, String> reverse_object_map_;
		Array<Object*> memory_map_;
		ObjectPtr<> root_;
	};
	
	typedef BasicUniverse TestUniverse;

}

#endif /* defined(__falling__universe_base__) */
