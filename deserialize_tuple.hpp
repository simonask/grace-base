//
//  slot_invoke.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 08/09/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_slot_invoke_hpp
#define falling_slot_invoke_hpp

#include "object/slot.hpp"
#include "serialization/archive_node.hpp"
#include "base/basic.hpp"

namespace falling {
	struct IUniverse;
	
	template <size_t SourceIdx, size_t TupleIdx, typename... TupleTypes>
	typename std::enable_if<(TupleIdx >= sizeof...(TupleTypes)), void>::type
	deserialize_list_into_tuple(const ArchiveNode&, std::tuple<TupleTypes...>&, IUniverse&) {
		// Recursion base.
	}

	template <size_t SourceIdx, size_t TupleIdx, typename... TupleTypes>
	typename std::enable_if<(TupleIdx < sizeof...(TupleTypes)), void>::type
	deserialize_list_into_tuple(const ArchiveNode& arg_list, std::tuple<TupleTypes...>& tuple, IUniverse& universe) {
		const ArchiveNode& node = arg_list[SourceIdx];
		auto& target = std::get<TupleIdx>(tuple);
		const Type* t = get_type<typename RemoveConstRef<decltype(target)>::Type>();
		t->deserialize((byte*)&target, node, universe);
		deserialize_list_into_tuple<SourceIdx+1, TupleIdx+1>(arg_list, tuple, universe);
	}
}

#endif
