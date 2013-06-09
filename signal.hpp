//
//  signal.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 03/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_signal_hpp
#define grace_signal_hpp

#include "base/basic.hpp"
#include "object/objectptr.hpp"
#include "serialization/archive_node.hpp"
#include "object/slot.hpp"
#include "object/object.hpp"
#include "object/object_type.hpp"
#include "memory/unique_ptr.hpp"
#include "base/function.hpp"
#include <type_traits>

namespace grace {
    struct SignalInvokerBase {
        virtual ObjectPtr<> receiver() const = 0;
        virtual const ISlot* slot() const = 0;
    };

    template <typename... Args>
    struct SignalInvoker : ListLinkBase<SignalInvoker<Args...>>, SignalInvokerBase {
        virtual void invoke(Args... args) const = 0;
    };
	
	struct SignalConnectionID {
	private:
		void* signal;
		SignalInvokerBase* invoker;
		template <typename... Args> friend class Signal;
	};

    template <typename... Args>
    class Signal {
    public:
		Signal(IAllocator& alloc = default_allocator()) : allocator_(alloc) {}
		~Signal();
	
        // Should catch raw functions and lambdas:
		template <typename T>
		SignalConnectionID connect(T value);
        template <typename R>
        SignalConnectionID connect(Function<R(Args...)> function);

        // Should catch raw member functions:
        template <typename T, typename R>
		SignalConnectionID connect(T* receiver, R(T::*member)(Args...));
		
		template <typename T, typename R>
		SignalConnectionID connect(const T* receiver, R(T::*member)(Args...) const);

        // Should catch slots with ObjectPtr:
        template <typename T, typename R>
        SignalConnectionID connect(ObjectPtr<T> ptr, R(T::*member)(Args...));

        template <typename T, typename R>
        SignalConnectionID connect(ObjectPtr<T> ptr, R(T::*member)(Args...) const);

        // Should catch named slots:
        Maybe<SignalConnectionID> connect(ObjectPtr<> ptr, StringRef slot_name);
		
		void disconnect(SignalConnectionID& connid);

        void invoke(const Args&...) const;
        void operator()(const Args&... args) const { invoke(args...); }
		
		using iterator = typename BareLinkList<SignalInvoker<Args...>>::iterator;
		using const_iterator = typename BareLinkList<SignalInvoker<Args...>>::const_iterator;
		iterator begin() { return invokers_.begin(); }
		iterator end() { return invokers_.end(); }
		const_iterator begin() const { return invokers_.begin(); }
		const_iterator end() const { return invokers_.end(); }
    private:
		IAllocator& allocator_;
		BareLinkList<SignalInvoker<Args...>> invokers_;
		
		SignalConnectionID link_and_make_id(SignalInvoker<Args...>* invoker) {
			invokers_.link_tail(invoker);
			SignalConnectionID connid;
			connid.signal = this;
			connid.invoker = invoker;
			return connid;
		}
    };

    template <typename R, typename... Args>
    struct FunctionInvoker : public SignalInvoker<Args...> {
        Function<R(Args...)> function;
        FunctionInvoker(Function<R(Args...)> function) : function(std::move(function)) {}
        ObjectPtr<> receiver() const { return nullptr; }
        const ISlot* slot() const { return nullptr; }
        void invoke(Args... args) const { function(std::forward<Args>(args)...); }
    };

    template <typename T, typename R, typename... Args>
    struct MemberFunctionInvoker : public SignalInvoker<Args...> {
		typedef typename std::remove_const<T>::type RawT;
        ObjectPtr<RawT> receiver_;
        typedef typename GetMemberFunctionPointerType<T, R, Args...>::Type FunctionPointerType;
        FunctionPointerType function_;

        MemberFunctionInvoker(ObjectPtr<RawT> receiver, FunctionPointerType member) : receiver_(receiver), function_(member) {}
        ObjectPtr<> receiver() const { return receiver_; }
        const ISlot* slot() const {
			const ObjectTypeBase* type = get_type<RawT>();
			return type->find_slot_for_method<T,R,Args...>(function_);
		}
        void invoke(Args... args) const { (receiver_.get()->*function_)(std::forward<Args>(args)...); }
    };

    template <typename... Args>
    struct SlotInvoker : SignalInvoker<Args...> {
        const Slot<Args...>* slot_;
        ObjectPtr<> object_;
        SlotInvoker(ObjectPtr<> receiver, const Slot<Args...>* slot) : object_(receiver), slot_(slot) {}
		const ISlot* slot() const { return dynamic_cast<const ISlot*>(slot_); }
		ObjectPtr<> receiver() const { return object_; }
		void invoke(Args... args) const { slot_->invoke_polymorphic(object_, std::forward<Args>(args)...); }
    };

	template <typename... Args>
	Signal<Args...>::~Signal() {
		while (!invokers_.empty()) {
			destroy(invokers_.head(), allocator_);
		}
	}
	
	template <typename... Args>
	template <typename T>
	SignalConnectionID Signal<Args...>::connect(T function_object) {
		using R = typename std::result_of<T(Args...)>::type;
		return connect(Function<R(Args...)>(move(function_object)));
	};

    template <typename... Args>
    template <typename R>
    SignalConnectionID Signal<Args...>::connect(Function<R(Args...)> function) {
		auto f = Function<R(Args...)>(move(function), allocator_);
        return link_and_make_id((new(allocator_, alignof(FunctionInvoker<R, Args...>)) FunctionInvoker<R, Args...>(move(f))));
    }

    template <typename... Args>
    template <typename T, typename R>
    SignalConnectionID Signal<Args...>::connect(T* receiver, R(T::*member)(Args...)) {
        // Convert member function call to free function call:
		return connect(bind_method(receiver, member));
    }
	
	template <typename... Args>
    template <typename T, typename R>
    SignalConnectionID Signal<Args...>::connect(const T* receiver, R(T::*member)(Args...) const) {
        // Convert member function call to free function call:
		return connect(bind_method(receiver, member));
    }

    template <typename... Args>
    template <typename T, typename R>
    SignalConnectionID Signal<Args...>::connect(ObjectPtr<T> receiver, R(T::*member)(Args...)) {
        return link_and_make_id(new(allocator_) MemberFunctionInvoker<T,R,Args...>(receiver, member));
    }

    template <typename... Args>
    template <typename T, typename R>
    SignalConnectionID Signal<Args...>::connect(ObjectPtr<T> receiver, R(T::*member)(Args...) const) {
        return link_and_make_id(new(allocator_) MemberFunctionInvoker<const T, R, Args...>(receiver, member));
    }
	
	template <typename... Args>
	void Signal<Args...>::disconnect(SignalConnectionID& connid) {
		ASSERT(connid.signal == this);
		auto invoker = dynamic_cast<SignalInvoker<Args...>*>(connid.invoker);
		destroy(invoker, allocator_);
		connid.signal = nullptr;
		connid.invoker = nullptr;
	}
	
	void nonexistent_slot_warning(ObjectPtr<> receiver, StringRef slot_name);
	void slot_type_mismatch_warning(ObjectPtr<> receiver, StringRef slot_name, String expected_signature_description, String signature_description);

    template <typename... Args>
    Maybe<SignalConnectionID> Signal<Args...>::connect(ObjectPtr<> receiver, StringRef slot_name) {
        const ISlot* slot_base = receiver->object_type()->find_slot_by_name(slot_name);
        if (slot_base == nullptr) {
            nonexistent_slot_warning(receiver, slot_name);
            return Nothing;
        }
        auto slot = dynamic_cast<const Slot<Args...>*>(slot_base);
        if (slot == nullptr) {
            slot_type_mismatch_warning(receiver, slot_name, slot_base->signature_description(), get_signature_description<Args...>(default_allocator()));
			return Nothing;
        }
		return link_and_make_id(new(allocator_) SlotInvoker<Args...>(receiver, slot));
    }
	
	template <typename... Args>
	void Signal<Args...>::invoke(const Args&... args) const {
		for (auto it = invokers_.begin(); it != invokers_.end();) {
			// Signal handlers may remove themselves from the list,
			// so save an iterator to the next element before invoking.
			auto next = it;
			++next;
			it->invoke(args...);
			it = next;
		}
	}
}

#endif
