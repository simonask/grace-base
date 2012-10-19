#pragma once
#ifndef REFLECT_HPP_WJBCX95G
#define REFLECT_HPP_WJBCX95G

#include "object/object.hpp"
#include "object/object_type.hpp"
#include "type/attribute.hpp"
#include "object/signal.hpp"
#include "type/types.hpp"
#include <algorithm>

namespace falling {

template <typename T> struct IntrusiveListLink;

template <typename T>
struct ObjectTypeBuilder {
	typedef ObjectTypeBuilder<T> Self;
	
	ObjectTypeBuilder() : type_(nullptr) {}
	
	Self& abstract(bool a = true) { type_->is_abstract_ = a; return *this; }
	Self& name(std::string n) { type_->name_ = std::move(n); return *this; }
	Self& description(std::string d) { type_->description_ = std::move(d); return *this; }
	Self& super(const ObjectTypeBase* t) { type_->super_ = t; return *this; }
	template <typename SuperClass>
	Self& super() { return super(get_type<SuperClass>()); }
	
	void check_attribute_name_(const std::string& name) {
		const char* reserved_names[] = {"class", "aspects"};
		for (auto it: reserved_names) {
			if (name == it) {
				fprintf(stderr, "The attribute name '%s' is reserved.\n", name.c_str());
				ASSERT(false);
			}
		}
	}
	
	template <typename MemberType>
	Self& property(MemberType T::* member, std::string name, std::string description/*, MemberType default_value = MemberType()*/) {
		check_attribute_name_(name);
		type_->properties_.push_back(new MemberAttribute<T, MemberType>(std::move(name), std::move(description), member));
		return *this;
	}
	
	template <typename GetterReturnType, typename SetterArgumentType, typename SetterReturnType>
	Self& property(GetterReturnType (T::*getter)() const, SetterReturnType (T::*setter)(SetterArgumentType), std::string name, std::string description) {
		check_attribute_name_(name);
		typedef typename RemoveConstRef<GetterReturnType>::Type RawType;
		type_->properties_.push_back(new MethodAttribute<T, RawType, GetterReturnType, SetterArgumentType, SetterReturnType>(std::move(name), std::move(description), getter, setter));
		return *this;
	}
	
	template <typename... Args>
	Self& signal(Signal<Args...> T::* member, std::string name, std::string description) {
		return property(member, name, description);
	}
	
	template <typename R, typename... Args>
	Self& slot(R(T::*function)(Args...), std::string name, std::string description) {
		type_->slots_.push_back(new SlotForTypeWithSignature<T, R, Args...>(std::move(name), std::move(description), function));
		return *this;
	}
	
	template <typename R, typename... Args>
	Self& slot(R(T::*function)(Args...) const, std::string name, std::string description) {
		type_->slots_.push_back(new SlotForTypeWithSignature<const T, R, Args...>(std::move(name), std::move(description), function));
		return *this;
	}
	
	template <typename ObjectType, size_t MemberOffset>
	Self& intrusive_list(IntrusiveListLink<ObjectType> ObjectType::* member) {
		type_->lists_.push_back(new IntrusiveListRegistrarImpl<T, ObjectType, MemberOffset>(member));
		return *this;
	}
	
	virtual void define__() = 0;
	
	ObjectType<T>* build__() {
		type_ = new ObjectType<T>(nullptr, "", "");
		define__();
		return type_;
	}
	
	ObjectType<T>* type_;
};

#define REFLECT_INTRUSIVE_LIST(T, MEMBER) intrusive_list<T, offsetof(T, MEMBER)>(&T::MEMBER)

#define BEGIN_TYPE_INFO(TYPE) \
const ObjectTypeBase* TYPE::build_type_info__() { \
	static struct ObjectTypeBuilderImpl__ : ObjectTypeBuilder<TYPE> { \
		void define__() override { name(#TYPE);
			
#define END_TYPE_INFO() \
		} \
	} builder__; \
	static const auto t = builder__.build__(); \
	return t; \
}

}

#endif /* end of include guard: REFLECT_HPP_WJBCX95G */
