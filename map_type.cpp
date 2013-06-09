//
//  map_type.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 10/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "type/map_type.hpp"
#include "io/string_stream.hpp"
#include "serialization/archive_node.hpp"

namespace grace {
	void MapType::build_map_type_name() {
		ScratchAllocator scratch;
		StringStream ss(scratch);
		ss << "Map<" << key_type()->name() << ", " << value_type()->name() << '>';
		name_ = ss.string(static_allocator());
	}
	
	void MapType::deserialize_map(IMapWriter& w, const ArchiveNode& dict, IUniverse& universe) const {
		auto kt = key_type();
		auto vt = value_type();
		size_t value_sz = vt->size();
		byte value_data_storage[value_sz];
		byte* value_data = value_data_storage;
		
		vt->construct(value_data, universe);
		dict.when<ArchiveNode::MapType>([&](const ArchiveNode::MapType& map) {
			for (auto pair: map) {
				vt->deserialize_raw(value_data, *pair.second, universe);
				if (kt == get_type<String>()) {
					String k(pair.first);
					w.set_and_move((byte*)&k, value_data);
				} else if (kt == get_type<StringRef>()) {
					StringRef k(pair.first);
					w.set_and_move((byte*)&k, value_data);
				} else {
					ASSERT(false); // Unsupported key type in non-array deserialization.
				}
			}
		});
		vt->destruct(value_data, universe);
	}
	
	void MapType::serialize_map(IMapReader& r, ArchiveNode& node, IUniverse& universe) const {
		auto kt = key_type();
		auto vt = value_type();
		
		while (r.next()) {
			void* keyp = r.current_key();
			void* valuep = r.current_value();
			
			StringRef key;
			if (kt == get_type<String>()) {
				key = *reinterpret_cast<String*>(keyp);
			} else if (kt == get_type<StringRef>()) {
				key = *reinterpret_cast<StringRef*>(keyp);
			} else {
				ASSERT(false); // Unsupported key type in non-array serialization.
			}
			
			ArchiveNode& value = node[key];
			vt->serialize_raw((const byte*)valuep, value, universe);
		}
	}
	
	void MapType::deserialize_map_from_array(IMapWriter& w, const ArchiveNode& node, IUniverse& universe) const {
		auto kt = key_type();
		auto vt = value_type();
		size_t key_sz = kt->size();
		size_t value_sz = vt->size();
		byte key_data_storage[key_sz];
		byte value_data_storage[value_sz];
		byte* key_data = key_data_storage;
		byte* value_data = value_data_storage;
		
		kt->construct(key_data, universe);
		vt->construct(value_data, universe);
		node.when<ArchiveNode::ArrayType>([&](const ArchiveNode::ArrayType& array) {
			for (auto child: array) {
				child->when<ArchiveNode::ArrayType>([&](const ArchiveNode::ArrayType& pair) {
					if (pair.size() < 2) return;
					const ArchiveNode* key_node = pair[0];
					const ArchiveNode* value_node = pair[1];
					kt->deserialize_raw(key_data, *key_node, universe);
					vt->deserialize_raw(value_data, *value_node, universe);
					w.set_and_move(key_data, value_data);
				});
			}
		});
		kt->destruct(key_data, universe);
		vt->destruct(value_data, universe);
	}
	
	void MapType::serialize_map_to_array(IMapReader& r, ArchiveNode& node, IUniverse& universe) const {
		auto kt = key_type();
		auto vt = value_type();
		
		while (r.next()) {
			void* keyp = r.current_key();
			void* valuep = r.current_value();
			
			ArchiveNode& pair = node.array_push();
			ArchiveNode& key_node = pair.array_push();
			ArchiveNode& value_node = pair.array_push();
			kt->serialize_raw((const byte*)keyp, key_node, universe);
			vt->serialize_raw((const byte*)valuep, value_node, universe);
		}
	}
}
