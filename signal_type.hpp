//
//  signal_type.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 04/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_signal_type_hpp
#define grace_signal_type_hpp

#include "type/type.hpp"
#include "object/signal.hpp"
#include "base/log.hpp"
#include "object/objectptr.hpp"

namespace grace {
	struct SignalTypeBase : Type {
	public:
		ArrayRef<const Type*> signature() const { return signature_; }
		StringRef name() const { return name_; }
		bool deferred_instantiation() const final { return true; }
	protected:
		String name_;
		Array<const Type*> signature_;
		static String build_signal_name(ArrayRef<const Type*> signature);
	};
	
	template <typename... Args>
	struct SignalType : TypeFor<Signal<Args...>, SignalTypeBase> {
		void deserialize(Signal<Args...>& place, const DocumentNode&, IUniverse&) const;
		void serialize(const Signal<Args...>& place, DocumentNode&, IUniverse&) const;
		
		SignalType() {
			build_signature<Args...>(this->signature_);
			this->name_ = SignalTypeBase::build_signal_name(this->signature_);
		}
	};
	
	template <typename... Args>
	struct BuildTypeInfo<Signal<Args...>> {
		static const SignalType<Args...>* build() {
			static const SignalType<Args...>* p = new_static SignalType<Args...>();
			return p;
		}
	};
	
	template <typename... Args>
	void SignalType<Args...>::deserialize(Signal<Args...>& signal, const DocumentNode& node, IUniverse& universe) const {
		if (node.is_array()) {
			for (size_t i = 0; i < node.array_size(); ++i) {
				const DocumentNode& connection = node[i];
				if (connection.is_map()) {
					auto& receiver_node = connection["receiver"];
					auto& slot_node = connection["slot"];
					StringRef receiver_id;
					StringRef slot_name;
					if ((receiver_node >> receiver_id) && (slot_node >> slot_name)) {
						ObjectPtr<> receiver = universe.get_object(receiver_id);
						if (receiver) {
							signal.connect(receiver, slot_name);
						} else {
							Warning() << "Invalid signal connection: No object named '" << receiver_id << "' in scene.";
						}
					} else {
						Warning() << "Invalid signal connection.";
					}
				} else {
					Warning() << "Signal connection isn't a map.";
				}
			}
		}
	}
	
	template <typename... Args>
	void SignalType<Args...>::serialize(const Signal<Args...>& signal, DocumentNode& node, IUniverse& universe) const {
		for (size_t i = 0; i < signal.num_connections(); ++i) {
			const SignalInvoker<Args...>* invoker = signal.connection_at(i);
			ObjectPtr<Object> receiver = invoker->receiver();
			const ISlot* slot = invoker->slot();
			if (receiver != nullptr && slot != nullptr) {
				DocumentNode& signal_connection = node.array_push();
				DocumentNode& receiver_node = signal_connection["receiver"];
				get_type(receiver)->serialize_raw(reinterpret_cast<byte*>(&receiver), receiver_node, universe);
				signal_connection["slot"] << slot->name();
			}
		}
	}
}


#endif
