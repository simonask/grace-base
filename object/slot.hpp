//
//  slot.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_slot_hpp
#define grace_slot_hpp

#include "base/basic.hpp"
#include "type/type.hpp"
#include "object/object.hpp"
#include "object/objectptr.hpp"
#include "serialization/deserialize_tuple.hpp"

namespace grace {
	struct DocumentNode;
	struct IUniverse;
	class Any;

	void warn_signal_receiver_type_mismatch(ObjectPtr<> receiver, const IType* expected_type);
	void warn_signal_receiver_argument_type_mismatch(ArrayRef<const IType*> signature);
	
	struct ISlot {
		virtual StringRef name() const = 0;
		virtual StringRef description() const = 0;
		virtual Array<const IType*> signature(IAllocator& alloc = default_allocator()) const = 0;
		virtual bool invoke(ObjectPtr<> receiver, ArrayRef<Any> args) const = 0;
		
		// Deprecated:
		virtual void invoke_with_serialized_arguments(ObjectPtr<> receiver, const DocumentNode& arg_list, IUniverse& universe) const = 0;
		virtual String signature_description(IAllocator& alloc = default_allocator()) const = 0;
	};
	
	template <typename... Args>
	struct Slot {
		virtual void invoke_polymorphic(ObjectPtr<> receiver, Args... args) const = 0;
	};
	
	template <typename T>
	struct SlotForType : ISlot {
		SlotForType(IAllocator& alloc, StringRef name, StringRef description) : name_(std::move(name), alloc), description_(std::move(description), alloc) {}
		
		T* get_object_polymorphic(ObjectPtr<> receiver) const {
			return dynamic_cast<T*>(receiver.get());
		}
		
		StringRef name() const final { return name_; }
		StringRef description() const final { return description_; }
		
		IAllocator& allocator() const { return name_.allocator(); }
	protected:
		String name_;
		String description_;
	};
	
	template <typename T, typename R, typename... Args>
	struct SlotForTypeWithSignature : Slot<Args...>, SlotForType<typename std::remove_const<T>::type> {
		typedef typename GetMemberFunctionPointerType<T, R, Args...>::Type FunctionPointer;
		FunctionPointer member_;
		
		FunctionPointer method() const { return member_; }
		
		SlotForTypeWithSignature(IAllocator& alloc, StringRef name, StringRef description, FunctionPointer function)
		: SlotForType<typename std::remove_const<T>::type>(alloc, std::move(name), std::move(description))
		, member_(function)
		{}
		
		Array<const IType*> signature(IAllocator& alloc) const {
			Array<const IType*> s({get_type<Args>()...}, alloc);
			return move(s);
		}
		
		bool invoke(ObjectPtr<> receiver, ArrayRef<Any> args) const {
			std::tuple<Args...> extracted_args;
			if (!extract_anies<0, 0>(args, extracted_args)) {
				warn_signal_receiver_argument_type_mismatch(signature(default_allocator()));
				return false;
			}
			T* ptr = this->get_object_polymorphic(receiver);
			if (ptr != nullptr) {
				apply_tuple_to_member(ptr, member_, extracted_args);
				return true;
			} else {
				warn_signal_receiver_type_mismatch(receiver, get_type<typename std::remove_const<T>::type>());
				return false;
			}
		}
		
		String signature_description(IAllocator& alloc) const { return get_signature_description<Args...>(alloc); }
		void invoke_polymorphic(ObjectPtr<> receiver, Args... args) const {
			T* object = this->get_object_polymorphic(receiver);
			if (object != nullptr) {
				(object->*member_)(std::forward<Args>(args)...);
			} else {
				warn_signal_receiver_type_mismatch(receiver, get_type<typename std::remove_const<T>::type>());
			}
		}
		
		void invoke_with_serialized_arguments(ObjectPtr<> object, const DocumentNode& arg_list, IUniverse& universe) const {
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
