//
//  pair.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 19/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_pair_hpp
#define grace_pair_hpp

namespace grace {
	template <typename Key, typename Value>
	struct Pair {
		Key first;
		Value second;
		
		// This is necessary for operator-> on map iterators.
		// XXX: Is there a nicer way to do this?
		Pair<Key, Value>* operator->() {
			return this;
		}
	};
}

#endif
