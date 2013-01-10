//
//  map_type.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 10/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_map_type_hpp
#define falling_map_type_hpp

#include "type/type.hpp"
#include "base/map.hpp"
#include "serialization/archive_node.hpp"

namespace falling {
	struct MapType : public Type {
		virtual const Type* key_type() const = 0;
		virtual const Type* value_type() const = 0;
	protected:
		String build_map_type_name();
	};
	
	template <typename K, typename V, typename Cmp>
	struct MapTypeImpl : public TypeFor<Map<K,V,Cmp>, MapType> {
		// TypeFor interface
		void deserialize(Map<K,V,Cmp>& place, const ArchiveNode& node, IUniverse& universe) const;
		void serialize(const Map<K,V,Cmp>& place, ArchiveNode& node, IUniverse& universe) const;
		
		// MapType interface
		const Type* key_type() const { return get_type<K>(); }
		const Type* value_type() const { return get_type<V>(); }
		
		// Type interface
		String name() const {
			return this->build_map_type_name();
		}
	};
	
	template <typename K, typename V, typename Cmp>
	struct BuildTypeInfo<Map<K,V,Cmp>> {
		static const MapTypeImpl<K,V,Cmp>* build() {
			static const MapTypeImpl<K,V,Cmp>* t = new_static MapTypeImpl<K,V,Cmp>;
			return t;
		}
	};
	
	template <typename K, typename V, typename Cmp>
	void MapTypeImpl<K,V,Cmp>::deserialize(Map<K, V, Cmp> &place, const ArchiveNode &node, IUniverse &universe) const {
		if (key_type() == get_type<String>()) {
			// Use the built-in support for string keys in the archive backend.
			if (node.is_map()) {
				for (auto pair: node.internal_map()) {
					V v;
					value_type()->deserialize_raw(reinterpret_cast<byte*>(&v), *pair.second, universe);
					place[pair.first] = move(v);
				}
				return;
			}
		}
		
		if (node.is_array()) {
			for (size_t i = 0; i < node.array_size(); ++i) {
				const ArchiveNode& pair = node[i];
				K k;
				V v;
				key_type()->deserialize_raw(reinterpret_cast<byte*>(&k), pair[0], universe);
				value_type()->deserialize_raw(reinterpret_cast<byte*>(&v), pair[1], universe);
				place.set(move(k), move(v));
			}
		}
	}
	
	template <typename K, typename V, typename Cmp>
	void MapTypeImpl<K,V,Cmp>::serialize(const Map<K, V, Cmp> &place, ArchiveNode &node, IUniverse &universe) const {
		if (key_type() == get_type<String>()) {
			// Use the built-in support for string keys in the archive backend.
			for (auto pair: place) {
				ArchiveNode& v = node[pair.first];
				value_type()->serialize_raw(reinterpret_cast<const byte*>(&pair.second), v, universe);
			}
			return;
		}
		
		for (auto pair: place) {
			ArchiveNode& out_pair = node.array_push();
			ArchiveNode& k = out_pair.array_push();
			ArchiveNode& v = out_pair.array_push();
			key_type()->serialize_raw(reinterpret_cast<const byte*>(&pair.first), k, universe);
			value_type()->serialize_raw(reinterpret_cast<const byte*>(&pair.second), k, universe);
		}
	}
}

#endif
