//
//  work_queue.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 17/12/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_work_queue_hpp
#define falling_work_queue_hpp

#include "base/priority_queue.hpp"
#include "base/time.hpp"
#include <thread>
#include <condition_variable>
#include <chrono>

namespace falling {
	template <typename Payload>
	class WorkQueue {
	public:
		explicit WorkQueue(Function<void(Payload&)> worker);
		WorkQueue(WorkQueue&&) = default;
		~WorkQueue();
		
		void schedule(Payload p, SystemTime at = SystemTime());
		template <typename InputIterator>
		void insert(InputIterator begin, InputIterator end);
		template <typename InputIterator, typename TimeIterator>
		void schedule(InputIterator begin, InputIterator end, TimeIterator tbegin, TimeIterator tend);
		void cancel_all();
		void wait_until_empty();
	private:
		struct TimedPayload {
			Payload payload;
			SystemTime wake_up_at;
			
			bool operator<(const TimedPayload& other) const { return wake_up_at > other.wake_up_at; }
		};
		PriorityQueue<TimedPayload> queue_;
		std::thread worker_;
		std::mutex lock_;
		std::condition_variable cond_;
		volatile bool running_ = true;
		
		bool should_wait_locked(SystemTime& wait_until) const {
			// Assumes that lock_ is held by current thread!
			if (!running_) return false;
			if (queue_.size() == 0) return true;
			SystemTime now = system_now();
			wait_until = queue_.top().wake_up_at;
			return wait_until > now;
		}
	};
	
	template <typename Payload>
	WorkQueue<Payload>::WorkQueue(Function<void(Payload&)> worker) {
		worker_ = std::thread([=]() {
			std::unique_lock<std::mutex> m(this->lock_);
			while (true) {
				SystemTime wait_until = SystemTime::forever();
				while (this->should_wait_locked(wait_until)) {
					if (this->running_) {
						if (wait_until == SystemTime::forever()) {
							this->cond_.wait(m);
						} else {
							this->cond_.wait_until(m, std::chrono::system_clock::time_point(std::chrono::microseconds(wait_until.microseconds_since_epoch())));
						}
					}
				}
				
				if (this->running_) {
					if (this->queue_.size() && this->queue_.top().wake_up_at <= system_now()) {
						TimedPayload work = this->queue_.pop();
						m.unlock();
						worker(work.payload);
						m.lock();
					}
				} else {
					break;
				}
			}
		});
	}
	
	template <typename Payload>
	WorkQueue<Payload>::~WorkQueue() {
		running_ = false;
		std::unique_lock<std::mutex> m(lock_);
		cond_.notify_one();
		m.unlock();
		worker_.join();
	}
	
	template <typename Payload>
	void WorkQueue<Payload>::schedule(Payload p, SystemTime at) {
		std::unique_lock<std::mutex> m(lock_);
		TimedPayload tp = {p, at};
		queue_.insert(std::move(tp));
		if (at <= queue_.top().wake_up_at) {
			cond_.notify_one();
		}
	}
	
	template <typename Payload>
	template <typename InputIterator>
	void WorkQueue<Payload>::insert(InputIterator begin, InputIterator end) {
		std::unique_lock<std::mutex> m(lock_);
		for (auto it = begin; it != end; ++it) {
			TimedPayload tp = {*it, SystemTime()};
			queue_.insert(std::move(tp));
		}
		cond_.notify_one();
	}
	
	template <typename Payload>
	template <typename InputIterator, typename TimeIterator>
	void WorkQueue<Payload>::schedule(InputIterator begin, InputIterator end, TimeIterator tbegin, TimeIterator tend) {
		std::unique_lock<std::mutex> m(lock_);
		auto it = begin;
		auto t = tbegin;
		for (; it != end && t != tend; ++it, ++t) {
			TimedPayload tp = {*it, *t};
			queue_.insert(std::move(tp));
		}
		cond_.notify_one();
	}
	
	template <typename Payload>
	void WorkQueue<Payload>::cancel_all() {
		std::unique_lock<std::mutex> m(lock_);
		queue_.clear();
	}
	
	template <typename Payload>
	void WorkQueue<Payload>::wait_until_empty() {
		std::unique_lock<std::mutex> m(lock_);
		if (queue_.size() == 0) return;
		while (queue_.size()) {
			auto wake_up_at = queue_.back().wake_up_at;
			m.unlock();
			std::this_thread::sleep_until(std::chrono::system_clock::time_point(std::chrono::microseconds(wake_up_at.microseconds_since_epoch())));
			m.lock();
		}
	}
}

#endif
