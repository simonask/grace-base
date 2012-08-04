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

namespace falling {
	enum Timeline : byte {
		System,
		Game,
	};
	
	template <Timeline T>
	struct TimeDelta {
	public:
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

		bool operator==(TimeDelta<T> other) const { return microseconds_ == other.microseconds_; }
		bool operator!=(TimeDelta<T> other) const { return microseconds_ != other.microseconds_; }
		bool operator<(TimeDelta<T> other) const { return microseconds_ < other.microseconds_; }
		bool operator<=(TimeDelta<T> other) const { return microseconds_ <= other.microseconds_; }
		bool operator>(TimeDelta<T> other) const { return microseconds_ > other.microseconds_; }
		bool operator>=(TimeDelta<T> other) const { return microseconds_ >= other.microseconds_; }

		int64 microseconds() const { return microseconds_; }
	private:
		TimeDelta(int64 microseconds) : microseconds_(microseconds) {}
		int64 microseconds_;
		template <Timeline> friend struct Time;
	};
	
	template <Timeline T>
	struct Time {
	public:
		Time(const Time<T>&) = default;
		Time(Time<T>&&) = default;
		Time<T>& operator=(const Time<T>& other) = default;
		Time<T>& operator=(Time<T>&& other) = default;
		
		uint64 microseconds_since_epoch() const { return microseconds_since_epoch_; }
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
		bool operator==(Time<T> other) const { return microseconds_since_epoch_ == other.microseconds_since_epoch_; }
		bool operator!=(Time<T> other) const { return microseconds_since_epoch_ != other.microseconds_since_epoch_; }
		bool operator<(Time<T> other) const { return microseconds_since_epoch_ < other.microseconds_since_epoch_; }
		bool operator<=(Time<T> other) const { return microseconds_since_epoch_ <= other.microseconds_since_epoch_; }
		bool operator>(Time<T> other) const { return microseconds_since_epoch_ > other.microseconds_since_epoch_; }
		bool operator>=(Time<T> other) const { return microseconds_since_epoch_ >= other.microseconds_since_epoch_; }
		
		void extract_components(int64& hours, int64& minutes, int64& seconds, int64& ms, int64& us) const;
		void extract_components(int64& minutes, int64& seconds, int64& ms, int64& us) const;
		void extract_components(int64& seconds, int64& ms, int64& us) const;

		static TimeDelta<T> hours(int64 h) { return minutes(h * 60); }
		static TimeDelta<T> minutes(int64 m) { return seconds(m * 60); }
		static TimeDelta<T> seconds(int64 s) { return milliseconds(s * 1000); }
		static TimeDelta<T> milliseconds(int64 ms) { return microseconds(ms * 1000); }
		static TimeDelta<T> microseconds(int64 us) { return TimeDelta<T>(us); }
	
		explicit Time(uint64 us_since_epoch = 0) : microseconds_since_epoch_(us_since_epoch) {}

		private:
		uint64 microseconds_since_epoch_;
	};

	extern template struct Time<Timeline::System>;
	extern template struct Time<Timeline::Game>;

	using SystemTime = Time<Timeline::System>;
	using SystemTimeDelta = TimeDelta<Timeline::System>;
	using GameTime = Time<Timeline::Game>;
	using GameTimeDelta = TimeDelta<Timeline::Game>;

	SystemTime system_now();
	
	template <Timeline T>
	inline TimeDelta<T> TimeDelta<T>::operator+(TimeDelta<T> delta) const {
		return microseconds_ + delta.microseconds_;
	}

	template <Timeline T>
	inline TimeDelta<T>& TimeDelta<T>::operator+=(TimeDelta<T> delta) {
		microseconds_ += delta.microseconds_;
		return *this;
	}

	template <Timeline T>
	inline TimeDelta<T> TimeDelta<T>::operator-(TimeDelta<T> delta) const {
		return microseconds_ - delta.microseconds_;
	}

	template <Timeline T>
	inline TimeDelta<T>& TimeDelta<T>::operator-=(TimeDelta<T> delta) {
		microseconds_ -= delta.microseconds_;
		return *this;
	}

	template <Timeline T>
	inline TimeDelta<T> TimeDelta<T>::operator*(int64 n) const {
		return microseconds_ * n;
	}

	template <Timeline T>
	inline TimeDelta<T>& TimeDelta<T>::operator*=(int64 n) {
		microseconds_ *= n;
		return *this;
	}

	template <Timeline T>
	inline TimeDelta<T> TimeDelta<T>::operator*(float64 n) const {
		return (uint64)((float64)microseconds_ * n);
	}

	template <Timeline T>
	inline TimeDelta<T>& TimeDelta<T>::operator*=(float64 n) {
		microseconds_ = (uint64)((float64)microseconds_ * n);
		return *this;
	}

	template <Timeline T>
	inline Time<T> Time<T>::operator+(TimeDelta<T> delta) const {
		return Time<T>(microseconds_since_epoch_ + delta.microseconds_);
	}

	template <Timeline T>
	inline Time<T>& Time<T>::operator+=(TimeDelta<T> delta) {
		microseconds_since_epoch_ += delta.microseconds_;
		return *this;
	}

	template <Timeline T>
	inline Time<T> Time<T>::operator-(TimeDelta<T> delta) const {
		return Time<T>(microseconds_since_epoch_ - delta.microseconds_);
	}

	template <Timeline T>
	inline TimeDelta<T> Time<T>::operator-(Time<T> other) const {
		return (int64)microseconds_since_epoch_ - (int64)other.microseconds_since_epoch_;
	}

	template <Timeline T>
	inline Time<T>& Time<T>::operator-=(TimeDelta<T> delta) {
		microseconds_since_epoch_ -= delta.microseconds_;
		return *this;
	}

	class FormattedStream;
	
	void write_time_to_stream(FormattedStream& stream, GameTime time);
	void write_time_to_stream(FormattedStream& stream, SystemTime time);
	
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
		ms = microseconds_since_epoch_ / 1000;
		us = microseconds_since_epoch_ % 1000;
		seconds = ms / 1000;
		ms %= 1000;
	}
}

#endif
