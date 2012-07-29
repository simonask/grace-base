//
//  time.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 15/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_time_hpp
#define falling_time_hpp

#include "base/basic.hpp"
#include <chrono>

namespace falling {
	
	template <typename TimeType>
	struct TimeInterval {
		typedef TimeInterval<TimeType> Self;
		typedef typename TimeType::DurationRep Rep;
		
	private:
		Rep rep_;
	};
	
	struct SystemTimeImpl {
		typedef std::chrono::time_point<std::chrono::system_clock> Rep;
		typedef typename Rep::duration DurationRep;
		
		static Rep now() { return std::system_clock::now(); }
		static Rep epoch() { return 0; }
		static Rep days_since_epoch(uint64 days) { return hours_since_epoch(days*24); }
		static Rep hours_since_epoch(uint64 hours) { return minutes_since_epoch(hours*60); }
		static Rep minutes_since_epoch(uint64 minutes) { return seconds_since_epoch(minutes*60); }
		static Rep seconds_since_epoch(uint64 seconds) { return milliseconds_since_epoch(seconds*1000); }
		static Rep milliseconds_since_epoch(uint64 ms) { return microseconds_since_epoch(ms*1000); }
		static Rep microseconds_since_epoch(uint64 us) { return us; }
		static DurationRep hours(int64 hours) { return minutes(hours*60); }
		static DurationRep minutes(int64 minutes) { return seconds(minutes*60); }
		static DurationRep seconds(int64 seconds) { return milliseconds(seconds*1000); }
		static DurationRep milliseconds(int64 ms) { return microseconds(ms*1000); }
		static DurationRep microseconds(int64 us) { return us; }
	};
	
	struct GameTimeImpl {
		// microseconds
		typedef uint64 Rep;
		typedef int64 DurationRep;
		
		static Rep epoch() { return 0; }
		static Rep days_since_epoch(uint64 days) { return hours_since_epoch(days*24); }
		static Rep hours_since_epoch(uint64 hours) { return minutes_since_epoch(hours*60); }
		static Rep minutes_since_epoch(uint64 minutes) { return seconds_since_epoch(minutes*60); }
		static Rep seconds_since_epoch(uint64 seconds) { return milliseconds_since_epoch(seconds*1000); }
		static Rep milliseconds_since_epoch(uint64 ms) { return microseconds_since_epoch(ms*1000); }
		static Rep microseconds_since_epoch(uint64 us) { return us; }
		static DurationRep hours(int64 hours) { return minutes(hours*60); }
		static DurationRep minutes(int64 minutes) { return seconds(minutes*60); }
		static DurationRep seconds(int64 seconds) { return milliseconds(seconds*1000); }
		static DurationRep milliseconds(int64 ms) { return microseconds(ms*1000); }
		static DurationRep microseconds(int64 us) { return us; }
	};
	
	template <typename Impl>
	struct Time : public Impl {
		typedef TimeInterval<Time<Impl>> Interval;
		typedef typename Impl::Rep Rep;
		typedef typename Impl::DurationRep DurationRep;
		
		
		
	private:
		Rep rep_;
	};
	
	using GameTime = Time<GameTimeImpl>;
	using SystemTime = Time<SystemTimeImpl>;
	using GameTimeInterval = TimeInterval<GameTime>;
	using SystemTimeInterval = TimeInterval<SystemTime>;
}

#endif
