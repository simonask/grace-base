//
//  slot.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 03/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_slot_hpp
#define falling_slot_hpp

#include "base/basic.hpp"
#include "type/type.hpp"
#include "object/object.hpp"
#include "object/objectptr.hpp"
#include "serialization/deserialize_tuple.hpp"

namespace falling {
	struct ArchiveNode;
	struct UniverseBase;

	void warn_signal_receiver_type_mismatch(ObjectPtr<> receiver, const Type* expected_type);
	
	struct SlotBase {
		virtual ~SlotBase() {}
		SlotBase(String name, String description) : name_(std::move(name)), description_(std::move(description)) {}
		const String& name() const { return name_; }
		const String& description() const { return description_; }
		virtual String signature_description() const = 0;
		virtual void invoke_with_serialized_arguments(ObjectPtr<> object, const ArchiveNode& arg_list, UniverseBase& universe) const = 0;
	private:
		String name_;
		String description_;
	};
	
	template <typename... Args>
	struct Slot {
		virtual void invoke_polymorphic(ObjectPtr<> receiver, Args... args) const = 0;
	};
	
	template <typename T>
	struct SlotForType : SlotBase {
		SlotForType(String name, String description) : SlotBase(std::move(name), std::move(description)) {}
		T* get_object_polymorphic(ObjectPtr<> receiver) const {
			return dynamic_cast<T*>(receiver.get());
		}
	};
	
	template <typename T, typename R, typename... Args>
	struct SlotForTypeWithSignature : SlotForType<typename std::remove_const<T>::type>, Slot<Args...> {
		typedef typename GetMemberFunctionPointerType<T, R, Args...>::Type FunctionPointer;
		FunctionPointer member_;
		
		FunctionPointer method() const { return member_; }
		
		SlotForTypeWithSignature(String name, String description, FunctionPointer function)
		: SlotForType<typename std::remove_const<T>::type>(std::move(name), std::move(description))
		, member_(function)
		{}
		
		String signature_description() const { return get_signature_description<Args...>(); }
		void invoke_polymorphic(ObjectPtr<> receiver, Args... args) const {
			T* object = this->get_object_polymorphic(receiver);
			if (object != nullptr) {
				(object->*member_)(std::forward<Args>(args)...);
			} else {
				warn_signal_receiver_type_mismatch(receiver, get_type<typename std::remove_const<T>::type>());
			}
		}
		
		void invoke_with_serialized_arguments(ObjectPtr<> object, const ArchiveNode& arg_list, UniverseBase& universe) const {
			std::tuple<Args...> deserialized_args;
			deserialize_list_into_tuple<0, 0>(arg_list, deserialized_args, universe);
			T* ptr = this->get_object_polymorphic(object);
			if (ptr != nullptr) {
				apply_tuple_to_member(ptr, member_, deserialized_args);
			} else {
				warn_signal_receiver_type_mismatch(object, get_type<typename std::remove_const<T>::type>());
			}
		}
	};
}

#endif
