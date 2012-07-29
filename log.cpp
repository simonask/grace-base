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
	};
	
	LogManager::Impl& LogManager::get() {
		static Impl* impl = new Impl;
		return *impl;
	}
	
	Signal<const LogEntry&>& LogManager::on_log() {
		return get().on_log;
	}
	
	LogLevel& LogManager::current_log_level() {
		return get().current_log_level;
	}
	
	void LogManager::log(const LogEntry& entry) {
		if (entry.level <= current_log_level()) {
			get().on_log(entry);
			
			if (get().on_log.num_connections() == 0) {
				auto io = entry.level == LogLevelInformation ? stdout : stderr;
				switch (entry.level) {
					case LogLevelError: fprintf(io, "ERROR: "); break;
					case LogLevelWarning: fprintf(io, "WARNING: "); break;
					case LogLevelInformation: fprintf(io, "INFO: "); break;
					case LogLevelDebug: fprintf(io, "DEBUG: "); break;
				}
				fprintf(io, "%s\n", entry.ss.str().c_str());
			}
		}
	}
}