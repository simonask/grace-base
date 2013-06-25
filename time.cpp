//
//  time.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 31/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "base/time.hpp"
#include "io/formatted_stream.hpp"
#include "io/formatters.hpp"

#include <sys/time.h>
#include <time.h>

#if defined(__MACH__)
// OS X and iOS don't have clock_gettime, so use Mach API.
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#define USE_HIGH_PRECISION_CLOCK 0

namespace grace {
	template struct Time<Timeline::System>;
	template struct Time<Timeline::Game>;
	template struct Time<Timeline::Process>;
	template struct TimeDelta<Timeline::System>;
	template struct TimeDelta<Timeline::Game>;
	template struct TimeDelta<Timeline::Process>;

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
		struct tm tm;
		time_t seconds = (time_t)time.seconds_since_epoch();
		localtime_r(&seconds, &tm);
		stream << pad_or_truncate(tm.tm_hour, 2, '0', true) << ':';
		stream << pad_or_truncate(tm.tm_min, 2, '0', true) << ':';
		stream << pad_or_truncate(tm.tm_sec, 2, '0', true);
	}
	
	SystemTime system_now() {
#if USE_HIGH_PRECISION_CLOCK
		struct timespec ts;
#  if defined(__MACH__)
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), REALTIME_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		ts.tv_sec = mts.tv_sec;
		ts.tv_nsec = mts.tv_nsec;
#  else
		clock_gettime(CLOCK_REALTIME, &ts);
#  endif // __MACH__
		return SystemTime() + SystemTime::seconds((int64)ts.tv_sec) + SystemTime::nanoseconds(ts.tv_nsec);
#else
		struct timeval tv;
		gettimeofday(&tv, nullptr);
		auto d = SystemTime::seconds((int64)tv.tv_sec) + SystemTime::microseconds(tv.tv_usec);
		return SystemTime() + d;
#endif
	}
	
	ProcessTime process_now() {
#if defined(__MACH__)
		uint64 ns = mach_absolute_time();
#endif
		return ProcessTime() + ProcessTime::nanoseconds(ns);
	}
}
