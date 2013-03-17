//
//  config.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 16/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "base/config.hpp"

namespace falling {
	namespace {
		byte ConfigurationManager_memory[sizeof(ConfigurationManager)];
	}
	
	ConfigurationManager& ConfigurationManager::get() {
		static ConfigurationManager* p = nullptr;
		if (p == nullptr) {
			p = new(ConfigurationManager_memory) ConfigurationManager;
		}
		return *p;
	}
	
	const ConfigurationOptionSet& ConfigurationManager::get_scope(StringRef scope_name) {
		auto p = get_mutable_scope(scope_name);
		if (p == nullptr) {
			return empty_scope_;
		}
		return *p;
	}
	
	ConfigurationOptionSet* ConfigurationManager::get_mutable_scope(StringRef scope_name) {
		auto it = scopes_.find(scope_name);
		if (it != scopes_.end()) {
			return &it->second;
		}
		return nullptr;
	}
	
	void ConfigurationManager::add_definitions(ConfigScopeDefiner &definer, StringRef name) {
		auto it = scopes_.find(name);
		if (it == scopes_.end()) {
			it = scopes_.set(name, ConfigurationOptionSet());
		}
		auto& options = it->second;
		for (auto pair: definer.options_) {
			options.options_[pair.first] = ConfigurationOptionSet::OptionValuePair{pair.second, Nothing};
		}
	}
	
	bool ConfigurationOptionSet::set(StringRef name, Any value) {
		auto it = options_.find(name);
		if (it != options_.end()) {
			if (value.is_empty()) {
				it->second.value = Nothing;
				return true;
			} else {
				if (value.type() == it->second.option.type) {
					it->second.value = move(value);
					return true;
				} else {
					Warning() << "Attempted to set config option '" << name << "' with value of type '" << value.type()->name() << "', but a value of type '" << it->second.option.type->name() << "' was expected. Resetting.";
					it->second.value = Nothing;
					return false;
				}
			}
		} else {
			Warning() << "Attempted to set config option '" << name << "', but it is not defined.";
			return false;
		}
	}

}