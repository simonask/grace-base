//
//  time.cpp
//  falling
//
//  Created by Simon Ask Ulsnes on 31/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/time.hpp"
#include "io/formatted_stream.hpp"
#include "io/formatters.hpp"

#include <sys/time.h>

namespace falling {
	template class Time<Timeline::System>;
	template class Time<Timeline::Game>;

	namespace {
		template <Timeline T>
		void write_time(FormattedStream& stream, Time<T> time) {
			int64 h, m, s, ms, us;
			time.extract_components(h, m, s, ms, us);
			if (h != 0) {
				stream << h << ':';
			}
			stream << pad_or_truncate(m, 2, '0', true) << ':';
			stream << pad_or_truncate(s, 2, '0', true) << ':';
			stream << pad_or_truncate(ms, 3, '0', true) << '.';
			stream << pad_or_truncate(us, 3, '0', true);
		}
	}
	
	void write_time_to_stream(FormattedStream& stream, GameTime time) {
		write_time(stream, time);
	}

	void write_time_to_stream(FormattedStream& stream, SystemTime time) {
		write_time(stream, time);
	}
	
	SystemTime system_now() {
		struct timeval tv;
		gettimeofday(&tv, nullptr);
		auto d = SystemTime::seconds(tv.tv_sec) + SystemTime::microseconds(tv.tv_usec);
		return SystemTime() + d;
	}
}
