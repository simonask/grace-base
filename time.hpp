//
//  time.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 15/06/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_time_hpp
#define grace_time_hpp

#include "base/basic.hpp"

namespace grace {
	enum Timeline : byte {
		System,
		Game,
		Process,
	};
	
	template <Timeline T>
	struct TimeDelta {
	public:
		TimeDelta() : nanoseconds_(0) {}
		TimeDelta(const TimeDelta<T>&) = default;
		TimeDelta<T>& operator=(const TimeDelta<T>&) = default;
		
		TimeDelta<T> operator+(TimeDelta<T>) const;
		TimeDelta<T>& operator+=(TimeDelta<T>);
		TimeDelta<T> operator-(TimeDelta<T>) const;
		TimeDelta<T>& operator-=(TimeDelta<T>);
		TimeDelta<T> operator*(int64 n) const;
		TimeDelta<T>& operator*=(int64 n);
		TimeDelta<T> operator*(float64 n) const;
		TimeDelta<T>& operator*=(float64 n);

		bool operator==(TimeDelta<T> other) const { return nanoseconds_ == other.nanoseconds_; }
		bool operator!=(TimeDelta<T> other) const { return nanoseconds_ != other.nanoseconds_; }
		bool operator<(TimeDelta<T> other) const { return nanoseconds_ < other.nanoseconds_; }
		bool operator<=(TimeDelta<T> other) const { return nanoseconds_ <= other.nanoseconds_; }
		bool operator>(TimeDelta<T> other) const { return nanoseconds_ > other.nanoseconds_; }
		bool operator>=(TimeDelta<T> other) const { return nanoseconds_ >= other.nanoseconds_; }

		int64 nanoseconds() const { return nanoseconds_; }
		float64 microseconds() const { return nanoseconds_ / 1000.0; }
		float64 milliseconds() const { return microseconds() / 1000.0; }
		float64 seconds() const { return milliseconds() / 1000.0; }
		
		static TimeDelta<T> forever() { return TimeDelta<T>(INT64_MAX); }
	private:
		TimeDelta(int64 nanoseconds) : nanoseconds_(nanoseconds) {}
		int64 nanoseconds_;
		template <Timeline> friend struct Time;
	};
	
	template <Timeline T>
	struct Time {
	public:
		Time(const Time<T>&) = default;
		Time(Time<T>&&) = default;
		Time<T>& operator=(const Time<T>& other) = default;
		Time<T>& operator=(Time<T>&& other) = default;
		
		uint64 nanoseconds_since_epoch() const { return nanoseconds_since_epoch_; }
		uint64 microseconds_since_epoch() const { return nanoseconds_since_epoch() / 1000; }
		uint64 milliseconds_since_epoch() const { return microseconds_since_epoch() / 1000; }
		uint64 seconds_since_epoch() const { return milliseconds_since_epoch() / 1000; }
		uint64 minutes_since_epoch() const { return seconds_since_epoch() / 60; }
		uint64 hours_since_epoch() const { return minutes_since_epoch() / 60; }
		uint64 days_since_epoch() const { return hours_since_epoch() / 24; }
		
		Time<T> operator+(TimeDelta<T>) const;
		Time<T>& operator+=(TimeDelta<T>);
		Time<T> operator-(TimeDelta<T>) const;
		TimeDelta<T> operator-(Time<T>) const;
		Time<T>& operator-=(TimeDelta<T>);
		bool operator==(Time<T> other) const { return nanoseconds_since_epoch_ == other.nanoseconds_since_epoch_; }
		bool operator!=(Time<T> other) const { return nanoseconds_since_epoch_ != other.nanoseconds_since_epoch_; }
		bool operator<(Time<T> other) const { return nanoseconds_since_epoch_ < other.nanoseconds_since_epoch_; }
		bool operator<=(Time<T> other) const { return nanoseconds_since_epoch_ <= other.nanoseconds_since_epoch_; }
		bool operator>(Time<T> other) const { return nanoseconds_since_epoch_ > other.nanoseconds_since_epoch_; }
		bool operator>=(Time<T> other) const { return nanoseconds_since_epoch_ >= other.nanoseconds_since_epoch_; }
		
		void extract_components(int64& hours, int64& minutes, int64& seconds, int64& ms, int64& us) const;
		void extract_components(int64& minutes, int64& seconds, int64& ms, int64& us) const;
		void extract_components(int64& seconds, int64& ms, int64& us) const;

		static TimeDelta<T> hours(int64 h) { return minutes(h * 60); }
		static TimeDelta<T> minutes(int64 m) { return seconds(m * 60); }
		static TimeDelta<T> seconds(int64 s) { return milliseconds(s * 1000); }
		static TimeDelta<T> seconds(float32 f) { return microseconds((int64)((float64)f * 1000000.0)); }
		static TimeDelta<T> milliseconds(int64 ms) { return microseconds(ms * 1000); }
		static TimeDelta<T> microseconds(int64 us) { return nanoseconds(us * 1000); }
		static TimeDelta<T> nanoseconds(int64 ns) { return TimeDelta<T>(ns); }
		static Time<T> forever() { return Time(UINT64_MAX); }
	
		explicit Time(uint64 ns_since_epoch = 0) : nanoseconds_since_epoch_(ns_since_epoch) {}

		private:
		uint64 nanoseconds_since_epoch_;
	};

	extern template struct Time<Timeline::System>;
	extern template struct Time<Timeline::Game>;
	extern template struct Time<Timeline::Process>;
	extern template struct TimeDelta<Timeline::System>;
	extern template struct TimeDelta<Timeline::Game>;
	extern template struct TimeDelta<Timeline::Process>;

	using SystemTime = Time<Timeline::System>;
	using SystemTimeDelta = TimeDelta<Timeline::System>;
	using GameTime = Time<Timeline::Game>;
	using GameTimeDelta = TimeDelta<Timeline::Game>;
	using ProcessTime = Time<Timeline::Process>;
	using ProcessTimeDelta = TimeDelta<Timeline::Process>;

	SystemTime system_now();
	ProcessTime process_now();
	
	template <Timeline T>
	inline TimeDelta<T> TimeDelta<T>::operator+(TimeDelta<T> delta) const {
		return nanoseconds_ + delta.nanoseconds_;
	}

	template <Timeline T>
	inline TimeDelta<T>& TimeDelta<T>::operator+=(TimeDelta<T> delta) {
		nanoseconds_ += delta.nanoseconds_;
		return *this;
	}

	template <Timeline T>
	inline TimeDelta<T> TimeDelta<T>::operator-(TimeDelta<T> delta) const {
		return nanoseconds_ - delta.nanoseconds_;
	}

	template <Timeline T>
	inline TimeDelta<T>& TimeDelta<T>::operator-=(TimeDelta<T> delta) {
		nanoseconds_ -= delta.nanoseconds_;
		return *this;
	}

	template <Timeline T>
	inline TimeDelta<T> TimeDelta<T>::operator*(int64 n) const {
		return nanoseconds_ * n;
	}

	template <Timeline T>
	inline TimeDelta<T>& TimeDelta<T>::operator*=(int64 n) {
		nanoseconds_ *= n;
		return *this;
	}

	template <Timeline T>
	inline TimeDelta<T> TimeDelta<T>::operator*(float64 n) const {
		return (uint64)((float64)nanoseconds_ * n);
	}

	template <Timeline T>
	inline TimeDelta<T>& TimeDelta<T>::operator*=(float64 n) {
		nanoseconds_ = (uint64)((float64)nanoseconds_ * n);
		return *this;
	}

	template <Timeline T>
	inline Time<T> Time<T>::operator+(TimeDelta<T> delta) const {
		return Time<T>(nanoseconds_since_epoch() + delta.nanoseconds_);
	}

	template <Timeline T>
	inline Time<T>& Time<T>::operator+=(TimeDelta<T> delta) {
		nanoseconds_since_epoch_ += delta.nanoseconds_;
		return *this;
	}

	template <Timeline T>
	inline Time<T> Time<T>::operator-(TimeDelta<T> delta) const {
		return Time<T>(nanoseconds_since_epoch_ - delta.nanoseconds_);
	}

	template <Timeline T>
	inline TimeDelta<T> Time<T>::operator-(Time<T> other) const {
		return (int64)nanoseconds_since_epoch_ - (int64)other.nanoseconds_since_epoch_;
	}

	template <Timeline T>
	inline Time<T>& Time<T>::operator-=(TimeDelta<T> delta) {
		nanoseconds_since_epoch_ -= delta.nanoseconds_;
		return *this;
	}

	class FormattedStream;
	
	void write_time_to_stream(FormattedStream& stream, GameTime time);
	void write_time_to_stream(FormattedStream& stream, SystemTime time);
	void write_date_to_stream(FormattedStream& stream, SystemTime time);
	
	template <Timeline T>
	inline FormattedStream& operator<<(FormattedStream& stream, Time<T> time) {
		write_time_to_stream(stream, time);
		return stream;
	}

	template <Timeline T>
	void Time<T>::extract_components(int64 &hours, int64 &minutes, int64 &seconds, int64 &ms, int64 &us) const {
		extract_components(minutes, seconds, ms, us);
		hours = minutes / 60;
		minutes %= 60;
	}
	
	template <Timeline T>
	void Time<T>::extract_components(int64 &minutes, int64 &seconds, int64 &ms, int64 &us) const {
		extract_components(seconds, ms, us);
		minutes = seconds / 60;
		seconds %= 60;
	}
	
	template <Timeline T>
	void Time<T>::extract_components(int64 &seconds, int64 &ms, int64 &us) const {
		ms = microseconds_since_epoch() / 1000;
		us = microseconds_since_epoch() % 1000;
		seconds = ms / 1000;
		ms %= 1000;
	}
}

#endif
