//
//  config.h
//  grace
//
//  Created by Simon Ask Ulsnes on 16/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __grace__config__
#define __grace__config__

#include "base/basic.hpp"
#include "base/any.hpp"
#include "base/dictionary.hpp"
#include "base/log.hpp"

// Commonly used option types:
#include "type/types.hpp"

namespace grace {
	struct ConfigOption {
		const IType* type;
		StringRef description;
	};

	struct ConfigScopeDefiner {
		using Option = ConfigOption;
		
		Dictionary<Option> options_;
		
		template <typename T>
		void option(StringRef name, StringRef description = "") {
			options_[name] = Option{get_type<T>(), description};
		}
		
		Dictionary<Option> build() {
			return move(options_);
		}
	};
	
	struct ConfigurationOptionSet {
		using Option = ConfigOption;
		struct OptionValuePair {
			Option option;
			Any value;
		};
		
		bool is_set(StringRef name);
		bool set(StringRef name, Any value);
		
		template <typename T>
		T get(StringRef name, T default_value = T()) const;
		
		Dictionary<OptionValuePair> options_;
	};
	
	class ConfigurationManager {
	public:
		static ConfigurationManager& get();
		
		const ConfigurationOptionSet& get_scope(StringRef scope_name);
		ConfigurationOptionSet* get_mutable_scope(StringRef scope_name);
		void add_definitions(ConfigScopeDefiner& definer, StringRef name);
	private:
		ConfigurationManager() {}
		Dictionary<ConfigurationOptionSet> scopes_;
		ConfigurationOptionSet empty_scope_;
	};
	
#define CONFIG(SCOPE) ConfigurationManager::get().get_scope(#SCOPE)
#define CONFIG_OPTION(SCOPE, NAME, DEFAULT) CONFIG(SCOPE).get(#NAME, DEFAULT)
	
	template <typename T>
	T ConfigurationOptionSet::get(StringRef name, T default_value) const {
		auto it = options_.find(name);
		if (it != options_.end()) {
			if (it->second.value.is_empty()) {
				return move(default_value);
			} else {
				if (it->second.value.is_a<T>()) {
					return it->second.value.get<T>().get_or(move(default_value));
				} else {
					Warning() << "Attempted to read config option '" << name << "' as type '" << get_type<T>()->name() << "', but it is of type '" << it->second.value.type()->name() << "'.";
					return default_value;
				}
			}
		} else {
			Warning() << "Attempted to read config option '" << name << "', but it is not defined.";
			return default_value;
		}
	}

#define BEGIN_CONFIG_SCOPE(SCOPE) \
	struct ConfigScopeDefiner_##SCOPE : ConfigScopeDefiner { \
		void define__(); \
	}; \
	void define_config_scope_##SCOPE##__() { \
		ConfigScopeDefiner_##SCOPE definer; \
		definer.define__(); \
		ConfigurationManager::get().add_definitions(definer, #SCOPE); \
	} \
	void ConfigScopeDefiner_##SCOPE::define__() {
#define END_CONFIG_SCOPE() \
	}

}

#endif /* defined(__grace__config__) */
