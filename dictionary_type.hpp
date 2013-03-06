//
//  dictionary_type.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 06/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_dictionary_type_hpp
#define falling_dictionary_type_hpp

#include "type/map_type.hpp"
#include "base/dictionary.hpp"

namespace falling {
	String build_dictionary_type_name(IAllocator& alloc, const Type* value_type);

	template <typename Value, typename Cmp>
	struct DictionaryTypeImpl : public TypeFor<Dictionary<Value,Cmp>, MapTypeImpl<StringRef, Value, Cmp>> {
	public:
		DictionaryTypeImpl(IAllocator& alloc) : name_(build_dictionary_type_name(alloc, get_type<Value>())) {}
		
		void deserialize(Dictionary<Value,Cmp>& place, const ArchiveNode& node, IUniverse& universe) const;
		void serialize(const Dictionary<Value,Cmp>& place, ArchiveNode& node, IUniverse& universe) const;
		StringRef name() const { return name_; }
	private:
		String name_;
	};
	
	template <typename Value, typename Cmp>
	struct BuildTypeInfo<Dictionary<Value, Cmp>> {
		static const DictionaryTypeImpl<Value, Cmp>* build() {
			static const DictionaryTypeImpl<Value, Cmp>* p = new_static DictionaryTypeImpl<Value, Cmp>(static_allocator());
			return p;
		}
	};
	
	template <typename Value, typename Cmp>
	void DictionaryTypeImpl<Value,Cmp>::deserialize(Dictionary<Value,Cmp>& place, const ArchiveNode& node, IUniverse& universe) const {
		MapWriter<Dictionary<Value,Cmp>> w(place);
		this->deserialize_map(w, node, universe);
	}
	
	template <typename Value, typename Cmp>
	void DictionaryTypeImpl<Value,Cmp>::serialize(const Dictionary<Value,Cmp>& place, ArchiveNode& node, IUniverse& universe) const {
		MapReader<Dictionary<Value,Cmp>> r(place);
		this->serialize_map(r, node, universe);
	}
}

#endif
