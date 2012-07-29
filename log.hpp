//
//  log.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 02/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_log_hpp
#define falling_log_hpp

#include "io/string_stream.hpp"

#include "object/signal.hpp"

namespace falling {
	enum LogLevel {
		LogLevelError,
		LogLevelWarning,
		LogLevelInformation,
		LogLevelDebug,
	};
	
#if defined(DEBUG)
	static const LogLevel DefaultLogLevel = LogLevelDebug;
#else
	static const LogLevel DefaultLogLevel = LogLevelInformation;
#endif
	
	struct LogEntry {
		explicit LogEntry(LogLevel level) : level(level) {}
		LogEntry(LogEntry&& other) : initialized(other.initialized), level(other.level), ss(std::move(other.ss)) {
			other.initialized = false;
		}
		~LogEntry();
		
		template <typename T>
		LogEntry& operator<<(T value) {
			initialized = true;
			ss << std::forward<T>(value);
			return *this;
		}
		
		bool initialized = false;
		LogLevel level;
		StringStream ss;
	};
	
	
	struct LogManager {
		static void log(const LogEntry& entry);
		static Signal<const LogEntry&>& on_log();
		static LogLevel& current_log_level();
	private:
		struct Impl;
		static Impl& get();
	};
	
	inline LogEntry::~LogEntry() {
		if (initialized) {
			LogManager::log(*this);
		}
	}
	
	inline LogEntry Log(LogLevel level = LogLevelInformation) {
		return LogEntry(level);
	}
	
	inline LogEntry Error() {
		return Log(LogLevelError);
	}
	
	inline LogEntry Information() {
		return Log(LogLevelInformation);
	}
	
	inline LogEntry Warning() {
		return Log(LogLevelWarning);
	}
	
	struct DummyLogEntry {
		template <typename T>
		DummyLogEntry& operator<<(const T&) {
			return *this;
		}
	};
	
#if defined(DEBUG)
	inline LogEntry Debug() {
		return LogEntry(LogLevelDebug);
	}
#else
	inline DummyLogEntry Debug() {
		return DummyLogEntry();
	}
#endif
}

#endif
