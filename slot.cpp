//
//  slot.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 04/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "object/slot.hpp"
#include "base/log.hpp"

namespace grace {
	void warn_signal_receiver_type_mismatch(ObjectPtr<> receiver, const Type* expected_type) {
		Warning() << "Polymorphic signal receiver is not of the expected type -- cannot invoke slot (expected " << expected_type->name() << ", got " << receiver->object_type()->name() << ").";
	}
	
	void warn_signal_receiver_argument_type_mismatch(ArrayRef<const Type*> signature) {
		Warning() << "Could not invoke slot, because there was an argument type mismatch.";
	}
}
