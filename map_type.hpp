//
//  map_type.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 10/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_map_type_hpp
#define grace_map_type_hpp

#include "type/type.hpp"
#include "base/map.hpp"

namespace grace {
	struct IMapReader;
	struct IMapWriter;

	struct MapType : public Type {
		virtual const Type* key_type() const = 0;
		virtual const Type* value_type() const = 0;
		StringRef name() const override { return name_; }
	protected:
		MapType(const TypeInfo& ti, IAllocator& alloc) : Type(ti), name_(alloc) {}
		void build_map_type_name();
		String name_;
		
		void deserialize_map_from_array(IMapWriter&, const ArchiveNode&, IUniverse&) const;
		void serialize_map_to_array(IMapReader&, ArchiveNode&, IUniverse&) const;
		void deserialize_map(IMapWriter&, const ArchiveNode&, IUniverse&) const;
		void serialize_map(IMapReader&, ArchiveNode&, IUniverse&) const;
	};
	
	template <typename K, typename V>
	struct MapTypeWithKeyValueType : public MapType {
		MapTypeWithKeyValueType(const TypeInfo& ti, IAllocator& alloc) : MapType(ti, alloc) { build_map_type_name(); }
		const Type* key_type() const final { return get_type<K>(); }
		const Type* value_type() const final { return get_type<V>(); }
	};
	
	template <typename K, typename V, typename Cmp>
	struct MapTypeImpl;
	
	template <typename V, typename Cmp>
	struct MapTypeImpl<String, V, Cmp> : TypeFor<Map<String, V, Cmp>, MapTypeWithKeyValueType<String, V>> {
		MapTypeImpl(IAllocator& alloc) : TypeFor<Map<String, V, Cmp>, MapTypeWithKeyValueType<String, V>>(alloc) {}
		void deserialize(Map<String,V,Cmp>& place, const ArchiveNode& node, IUniverse& universe) const;
		void serialize(const Map<String,V,Cmp>& place, ArchiveNode& node, IUniverse& universe) const;
	};
	
	template <typename V, typename Cmp>
	struct MapTypeImpl<StringRef, V, Cmp> : TypeFor<Map<StringRef, V, Cmp>, MapTypeWithKeyValueType<StringRef, V>> {
		MapTypeImpl(const TypeInfo& ti, IAllocator& alloc) : TypeFor<Map<StringRef, V, Cmp>, MapTypeWithKeyValueType<StringRef, V>>(alloc) {}
		void deserialize(Map<StringRef,V,Cmp>& place, const ArchiveNode& node, IUniverse& universe) const;
		void serialize(const Map<StringRef,V,Cmp>& place, ArchiveNode& node, IUniverse& universe) const;
	};
	
	template <typename K, typename V, typename Cmp>
	struct MapTypeImpl : TypeFor<Map<K, V, Cmp>, MapTypeWithKeyValueType<K, V>> {
		MapTypeImpl(IAllocator& alloc) : TypeFor<Map<K,V,Cmp>, MapTypeWithKeyValueType<K,V>>(alloc) {}
		void deserialize(Map<K,V,Cmp>& place, const ArchiveNode& node, IUniverse& universe) const;
		void serialize(const Map<K,V,Cmp>& place, ArchiveNode& node, IUniverse& universe) const;
	};
	
	template <typename K, typename V, typename Cmp>
	struct BuildTypeInfo<Map<K,V,Cmp>> {
		static const MapTypeImpl<K,V,Cmp>* build() {
			static const MapTypeImpl<K,V,Cmp>* t = new_static MapTypeImpl<K,V,Cmp>(static_allocator());
			return t;
		}
	};
	
	struct IMapReader {
		virtual void* current_key() = 0;
		virtual void* current_value() = 0;
		virtual bool next() = 0;
	};
	
	struct IMapWriter {
		virtual void set_and_move(byte* key, byte* value) = 0;
	};
	
	template <typename MapType>
	struct MapReader : IMapReader {
		MapReader(const MapType& m) : map_(m) {}
		const MapType& map_;
		typename MapType::const_iterator p_;
		bool init_ = false;
		
		bool next() final {
			if (!init_) {
				p_ = map_.begin();
				init_ = true;
			} else {
				++p_;
			}
			return p_ != map_.end();
		}
		
		void* current_key() final {
			ASSERT(init_);
			return (void*)&p_->first;
		}
		void* current_value() final {
			ASSERT(init_);
			return (void*)&p_->second;
		}
	};
	
	template <typename MapType>
	struct MapWriter : IMapWriter {
		MapWriter(MapType& m) : map_(m) {}
		MapType& map_;
		
		void set_and_move(byte* key, byte* value) final {
			using K = typename MapType::key_type;
			using V = typename MapType::mapped_type;
			K* k = reinterpret_cast<K*>(key);
			V* v = reinterpret_cast<V*>(value);
			map_[move(*k)] = move(*v);
		}
	};
	
	template <typename K, typename V, typename Cmp>
	void MapTypeImpl<K,V,Cmp>::deserialize(Map<K, V, Cmp> &place, const ArchiveNode &node, IUniverse &universe) const {
		MapWriter<Map<K,V,Cmp>> w(place);
		deserialize_map_from_array(w, node, universe);
	}
	
	template <typename K, typename V, typename Cmp>
	void MapTypeImpl<K,V,Cmp>::serialize(const Map<K, V, Cmp> &place, ArchiveNode &node, IUniverse &universe) const {
		MapReader<Map<K,V,Cmp>> e(place);
		serialize_map_as_array(e, node, universe);
	}
	
	template <typename V, typename Cmp>
	void MapTypeImpl<String,V,Cmp>::deserialize(Map<String, V, Cmp> &place, const ArchiveNode &node, IUniverse &universe) const {
		MapWriter<Map<String,V,Cmp>> w(place);
		this->deserialize_map(w, node, universe);
	}
	
	template <typename V, typename Cmp>
	void MapTypeImpl<String,V,Cmp>::serialize(const Map<String, V, Cmp> &place, ArchiveNode &node, IUniverse &universe) const {
		MapReader<Map<String,V,Cmp>> e(place);
		this->serialize_map(e, node, universe);
	}
	
	template <typename V, typename Cmp>
	void MapTypeImpl<StringRef,V,Cmp>::deserialize(Map<StringRef, V, Cmp> &place, const ArchiveNode &node, IUniverse &universe) const {
		ASSERT(false); // Cannot deserialize StringRef!
	}
	
	template <typename V, typename Cmp>
	void MapTypeImpl<StringRef,V,Cmp>::serialize(const Map<StringRef, V, Cmp> &place, ArchiveNode &node, IUniverse &universe) const {
		MapReader<Map<StringRef,V,Cmp>> e(place);
		this->serialize_map(e, node, universe);
	}
}

#endif
