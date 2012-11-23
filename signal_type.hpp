//
//  signal_type.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 04/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_signal_type_hpp
#define falling_signal_type_hpp

#include "type/type.hpp"
#include "object/signal.hpp"
#include "base/log.hpp"
#include "object/objectptr.hpp"

namespace falling {
	struct SignalTypeBase : Type {
	public:
		ArrayRef<const Type*> signature() const { return signature_; }
		std::string name() const { return name_; }
	protected:
		std::string name_;
		Array<const Type*> signature_;
		static std::string build_signal_name(ArrayRef<const Type*> signature);
	};
	
	template <typename... Args>
	struct SignalType : TypeFor<Signal<Args...>, SignalTypeBase> {
		void deserialize(Signal<Args...>& place, const ArchiveNode&, UniverseBase&) const;
		void serialize(const Signal<Args...>& place, ArchiveNode&, UniverseBase&) const;
		
		SignalType() {
			build_signature<Args...>(this->signature_);
			this->name_ = SignalTypeBase::build_signal_name(this->signature_);
		}
	};
	
	template <typename... Args>
	struct BuildTypeInfo<Signal<Args...>> {
		static const SignalType<Args...>* build() {
			static const SignalType<Args...>* p = new SignalType<Args...>();
			return p;
		}
	};
	
	template <typename... Args>
	void SignalType<Args...>::deserialize(Signal<Args...>& signal, const ArchiveNode& node, UniverseBase&) const {
		if (node.is_array()) {
			for (size_t i = 0; i < node.array_size(); ++i) {
				const ArchiveNode& connection = node[i];
				if (connection.is_map()) {
					auto& receiver_node = connection["receiver"];
					auto& slot_node = connection["slot"];
					std::string receiver;
					std::string slot;
					if (receiver_node.get(receiver) && slot_node.get(slot)) {
						node.register_signal_for_deserialization(&signal, receiver, slot);
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
	void SignalType<Args...>::serialize(const Signal<Args...>& signal, ArchiveNode& node, UniverseBase& universe) const {
		for (size_t i = 0; i < signal.num_connections(); ++i) {
			const SignalInvoker<Args...>* invoker = signal.connection_at(i);
			ObjectPtr<Object> receiver = invoker->receiver();
			const SlotBase* slot = invoker->slot();
			if (receiver != nullptr && slot != nullptr) {
				ArchiveNode& signal_connection = node.array_push();
				ArchiveNode& receiver_node = signal_connection["receiver"];
				get_type(receiver)->serialize_raw(reinterpret_cast<byte*>(&receiver), receiver_node, universe);
				signal_connection["slot"] = slot->name();
			}
		}
	}
}


#endif
