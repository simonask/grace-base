//
//  slot_invoke.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 08/09/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_slot_invoke_hpp
#define grace_slot_invoke_hpp

#include "serialization/document_node.hpp"
#include "base/basic.hpp"
#include "base/any.hpp"

namespace grace {
	struct IUniverse;
	
	template <size_t SourceIdx, size_t TupleIdx, typename... TupleTypes>
	typename std::enable_if<(TupleIdx >= sizeof...(TupleTypes)), void>::type
	deserialize_list_into_tuple(const DocumentNode&, std::tuple<TupleTypes...>&, IUniverse&) {
		// Recursion base.
	}

	template <size_t SourceIdx, size_t TupleIdx, typename... TupleTypes>
	typename std::enable_if<(TupleIdx < sizeof...(TupleTypes)), void>::type
	deserialize_list_into_tuple(const DocumentNode& arg_list, std::tuple<TupleTypes...>& tuple, IUniverse& universe) {
		const DocumentNode& node = arg_list[SourceIdx];
		auto& target = std::get<TupleIdx>(tuple);
		const Type* t = get_type<typename RemoveConstRef<decltype(target)>::Type>();
		t->deserialize_raw((byte*)&target, node, universe);
		deserialize_list_into_tuple<SourceIdx+1, TupleIdx+1>(arg_list, tuple, universe);
	}
	
	template <size_t SourceIdx, size_t TupleIdx, typename... TupleTypes>
	typename std::enable_if<(TupleIdx >= sizeof...(TupleTypes)), bool>::type
	extract_anies(ArrayRef<Any> anies, std::tuple<TupleTypes...>& tuple) {
		// Recursion base
		return true;
	}
	
	template <size_t SourceIdx, size_t TupleIdx, typename... TupleTypes>
	typename std::enable_if<(TupleIdx < sizeof...(TupleTypes)), bool>::type
	extract_anies(ArrayRef<Any> anies, std::tuple<TupleTypes...>& tuple)
	{
		auto& target = std::get<TupleIdx>(tuple);
		using T = typename RemoveConstRef<decltype(target)>::Type;
		bool was_extracted = false;
		anies[SourceIdx].get<T>().map([&](const T& v) {
			target = v;
			was_extracted = true;
		});
		return was_extracted || extract_anies<SourceIdx+1, TupleIdx+1>(anies, tuple);
	}
}

#endif
