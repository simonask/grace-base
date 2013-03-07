//
//  log.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 02/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/log.hpp"
#include <stdio.h>

namespace falling {
	struct LogManager::Impl {
		Signal<const LogEntry&> on_log;
		LogLevel current_log_level = DefaultLogLevel;
		bool stdio_echo = true;
	};
	
	LogManager::Impl& LogManager::get() {
		static Impl* impl = new_static Impl;
		return *impl;
	}
	
	Signal<const LogEntry&>& LogManager::on_log() {
		return get().on_log;
	}
	
	LogLevel& LogManager::current_log_level() {
		return get().current_log_level;
	}
	
	bool LogManager::stdio_echo_enabled() {
		return get().stdio_echo;
	}
	
	void LogManager::set_stdio_echo_enabled(bool e) {
		get().stdio_echo = e;
	}
	
	void LogManager::log(const LogEntry& entry) {
		if (entry.level <= current_log_level()) {
			get().on_log(entry);
			
			bool stdio_echo = get().stdio_echo;
			
			if (stdio_echo) {
				auto io = entry.level == LogLevelInformation ? stdout : stderr;
				switch (entry.level) {
					case LogLevelNone: break;
					case LogLevelError: fprintf(io, "ERROR: "); break;
					case LogLevelWarning: fprintf(io, "WARNING: "); break;
					case LogLevelInformation: fprintf(io, "INFO: "); break;
					case LogLevelDebug: fprintf(io, "DEBUG: "); break;
				}
				ScratchAllocator scratch;
				String str = entry.ss.string(scratch);
				fwrite(str.data(), str.size(), 1, io);
				fwrite("\n", 1, 1, io);
			}
		}
	}
}
