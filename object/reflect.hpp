#pragma once
#ifndef REFLECT_HPP_WJBCX95G
#define REFLECT_HPP_WJBCX95G

#include "object/object.hpp"
#include "object/object_type.hpp"
#include "type/attribute.hpp"
#include "object/signal.hpp"
#include "type/types.hpp"
#include <algorithm>

namespace grace {

template <typename T> struct AutoListLink;
	template <typename T, typename ObjectType, size_t MemberOffset>
	struct AutoListRegistrarImpl : AutoListRegistrarForObject<T> {
		typedef AutoListLink<ObjectType> ObjectType::* LinkMemberType;
		LinkMemberType link_;
		
		AutoListRegistrarImpl(LinkMemberType link) : link_(link) {}
		
		void link_object_in_universe(T& object, IUniverse& universe) const {
			UniverseBase* universe_base = dynamic_cast<UniverseBase*>(&universe);
			auto& list = universe_base->get_auto_list<ObjectType, MemberOffset>();
			AutoListLink<ObjectType>* link = &(object.*link_);
			list.link_head(link);
		}
	};

template <typename T>
struct ObjectTypeBuilder {
	typedef ObjectTypeBuilder<T> Self;
	
	ObjectTypeBuilder() : type_(nullptr) {}
	
	Self& abstract(bool a = true) { type_->is_abstract_ = a; return *this; }
	Self& name(StringRef n) { type_->name_ = n; return *this; }
	Self& description(String d) { type_->description_ = std::move(d); return *this; }
	Self& super(const ObjectTypeBase* t) { type_->super_ = t; return *this; }
	Self& enable_updates() { type_->set_wants_game_update(true); return *this; }
	template <typename SuperClass>
	Self& super() {
		static_assert(std::is_base_of<SuperClass, T>::value, "Class does not derive from SuperClass.");
		return super(get_type<SuperClass>());
	}
	
	void check_attribute_name_(StringRef name) {
		const char* reserved_names[] = {"aspects", "__editor_data"};
		for (auto it: reserved_names) {
			if (name == StringRef(it)) {
				Error() << "The attribute name '" << name << "' is reserved.";
				ASSERT(false);
			}
		}
	}
	
	template <typename MemberType>
	Self& property(MemberType T::* member, StringRef name, StringRef description = ""/*, MemberType default_value = MemberType()*/) {
		check_attribute_name_(name);
		type_->properties_.push_back(new_static MemberAttribute<T, MemberType>(static_allocator(), std::move(name), std::move(description), member));
		return *this;
	}
	
	template <typename GetterReturnType, typename SetterArgumentType, typename SetterReturnType>
	Self& property(GetterReturnType (T::*getter)() const, SetterReturnType (T::*setter)(SetterArgumentType), StringRef name, StringRef description = "") {
		check_attribute_name_(name);
		typedef typename RemoveConstRef<GetterReturnType>::Type RawType;
		type_->properties_.push_back(new_static MethodAttribute<T, RawType, GetterReturnType, SetterArgumentType, SetterReturnType>(static_allocator(), std::move(name), std::move(description), getter, setter));
		return *this;
	}
	
	template <typename GetterReturnType>
	Self& property(GetterReturnType (T::*getter)() const, NullPtr null, StringRef name, StringRef description = "") {
		check_attribute_name_(name);
		typedef typename RemoveConstRef<GetterReturnType>::Type RawType;
		type_->properties_.push_back(new_static ReadOnlyMethodAttribute<T, RawType, GetterReturnType>(static_allocator(), move(name), move(description), getter));
		return *this;
	}
	
	template <typename MemberType>
	Self& property(NullPtr null, NullPtr null2, StringRef name, StringRef description = "") {
		check_attribute_name_(name);
		typedef typename RemoveConstRef<MemberType>::Type RawType;
		type_->properties_.push_back(new_static OpaqueAttribute<T, MemberType>(static_allocator(), move(name), move(description)));
		return *this;
	}
	
	template <typename... Args>
	Self& signal(Signal<Args...> T::* member, StringRef name, StringRef description = "") {
		return property(member, name, description);
	}
	
	template <typename R, typename... Args>
	Self& slot(R(T::*function)(Args...), StringRef name, StringRef description = "") {
		type_->slots_.push_back(new_static SlotForTypeWithSignature<T, R, Args...>(static_allocator(), std::move(name), std::move(description), function));
		return *this;
	}
	
	template <typename R, typename... Args>
	Self& slot(R(T::*function)(Args...) const, StringRef name, StringRef description = "") {
		type_->slots_.push_back(new_static SlotForTypeWithSignature<const T, R, Args...>(static_allocator(), std::move(name), std::move(description), function));
		return *this;
	}
	
	template <typename ObjectType, size_t MemberOffset>
	Self& auto_list(AutoListLink<ObjectType> ObjectType::* member) {
		type_->lists_.push_back(new_static AutoListRegistrarImpl<T, ObjectType, MemberOffset>(member));
		return *this;
	}
	
	virtual void define__() = 0;
	
	ObjectType<T>* build__() {
		type_ = new_static ObjectType<T>(static_allocator(), nullptr, "", "");
		define__();
		return type_;
	}
	
	ObjectType<T>* type_;
};

#define REFLECT_AUTO_LIST(T, MEMBER) auto_list<T, offsetof(T, MEMBER)>(&T::MEMBER)

#define BEGIN_TYPE_INFO(TYPE) \
const grace::ObjectTypeBase* TYPE::build_type_info__() { \
	static struct ObjectTypeBuilderImpl__ : grace::ObjectTypeBuilder<TYPE> { \
		void define__() override { name(#TYPE);
			
#define END_TYPE_INFO() \
		} \
	} builder__; \
	static const auto t = builder__.build__(); \
	return t; \
}

}

#endif /* end of include guard: REFLECT_HPP_WJBCX95G */
