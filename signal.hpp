//
//  signal.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 03/08/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_signal_hpp
#define falling_signal_hpp

#include "base/basic.hpp"
#include "object/objectptr.hpp"
#include "serialization/archive_node.hpp"
#include "object/slot.hpp"
#include "object/object.hpp"
#include "object/object_type.hpp"

#include <functional>
#include <type_traits>

namespace falling {
    struct SignalInvokerBase {
        virtual ObjectPtr<> receiver() const = 0;
        virtual const SlotBase* slot() const = 0;
    };

    template <typename... Args>
    struct SignalInvoker : SignalInvokerBase {
        virtual void invoke(Args... args) const = 0;
    };

    template <typename... Args>
    class Signal {
    public:
        // Should catch raw functions and lambdas:
        template <typename Function>
        void connect(Function function);

        // Should catch raw member functions:
        template <typename T, typename R>
        typename std::enable_if<!HasReflection<typename std::remove_const<T>::type>::Value, void>::type
        connect(T* receiver, R(T::*member)(Args...));
		
		template <typename T, typename R>
        typename std::enable_if<!HasReflection<typename std::remove_const<T>::type>::Value, void>::type
        connect(const T* receiver, R(T::*member)(Args...) const);

        // Should catch slots with ObjectPtr:
        template <typename T, typename R>
        void connect(ObjectPtr<T> ptr, R(T::*member)(Args...));

        template <typename T, typename R>
        void connect(ObjectPtr<T> ptr, R(T::*member)(Args...) const);

        // Should catch named slots:
        bool connect(ObjectPtr<> ptr, const std::string& slot_name);

        void invoke(const Args&...) const;
        void operator()(const Args&... args) const { invoke(args...); }

        size_t num_connections() const { return invokers_.size(); }
        SignalInvoker<Args...>* connection_at(size_t idx) const { return invokers_[idx].get(); }
    private:
        Array<std::unique_ptr<SignalInvoker<Args...>>> invokers_;
    };

    template <typename R, typename... Args>
    struct FunctionInvoker : public SignalInvoker<Args...> {
        std::function<R(Args...)> function;
        FunctionInvoker(std::function<R(Args...)> function) : function(std::move(function)) {}
        ObjectPtr<> receiver() const { return nullptr; }
        const SlotBase* slot() const { return nullptr; }
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
        const SlotBase* slot() const {
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
		const SlotBase* slot() const { return dynamic_cast<const SlotBase*>(slot_); }
		ObjectPtr<> receiver() const { return object_; }
		void invoke(Args... args) const { slot_->invoke_polymorphic(object_, std::forward<Args>(args)...); }
    };

	

    template <typename... Args>
    template <typename Function>
    void Signal<Args...>::connect(Function function) {
		typedef decltype(function(std::declval<Args>()...)) R;
		auto f = std::function<R(Args...)>(std::move(function));
        invokers_.push_back(make_unique<FunctionInvoker<R, Args...>>(std::move(f)));
    }

    template <typename... Args>
    template <typename T, typename R>
    typename std::enable_if<!HasReflection<typename std::remove_const<T>::type>::Value, void>::type
    Signal<Args...>::connect(T* receiver, R(T::*member)(Args...)) {
        // Convert member function call to free function call:
        connect(std::bind(member, receiver));
    }
	
	template <typename... Args>
    template <typename T, typename R>
    typename std::enable_if<!HasReflection<typename std::remove_const<T>::type>::Value, void>::type
    Signal<Args...>::connect(const T* receiver, R(T::*member)(Args...) const) {
        // Convert member function call to free function call:
        connect(std::bind(member, receiver));
    }

    template <typename... Args>
    template <typename T, typename R>
    void Signal<Args...>::connect(ObjectPtr<T> receiver, R(T::*member)(Args...)) {

        invokers_.push_back(make_unique<MemberFunctionInvoker<T,R,Args...>>(receiver, member));
    }

    template <typename... Args>
    template <typename T, typename R>
    void Signal<Args...>::connect(ObjectPtr<T> receiver, R(T::*member)(Args...) const) {
        invokers_.push_back(make_unique<MemberFunctionInvoker<const T, R, Args...>>(receiver, member));
    }
	
	void nonexistent_slot_warning(ObjectPtr<> receiver, const std::string& slot_name);
	void slot_type_mismatch_warning(ObjectPtr<> receiver, const std::string& slot_name, std::string expected_signature_description, std::string signature_description);

    template <typename... Args>
    bool Signal<Args...>::connect(ObjectPtr<> receiver, const std::string& slot_name) {
        const SlotBase* slot_base = receiver->object_type()->find_slot_by_name(slot_name);
        if (slot_base == nullptr) {
            nonexistent_slot_warning(receiver, slot_name);
            return false;
        }
        auto slot = dynamic_cast<const Slot<Args...>*>(slot_base);
        if (slot == nullptr) {
            slot_type_mismatch_warning(receiver, slot_name, slot_base->signature_description(), get_signature_description<Args...>());
			return false;
        }
		invokers_.push_back(make_unique<SlotInvoker<Args...>>(receiver, slot));
		return true;
    }
	
	template <typename... Args>
	void Signal<Args...>::invoke(const Args&... args) const {
		for (auto& it: invokers_) {
			it->invoke(args...);
		}
	}
}

#endif
