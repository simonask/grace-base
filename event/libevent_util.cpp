//
//  libevent_util.cpp
//  interface
//
//  Created by Simon Ask Ulsnes on 02/06/13.
//
//

#include "libevent_util.hpp"
#include "base/basic.hpp"

#include <mutex>
#include <condition_variable>
#include <thread>

#include "event2/thread.h"
#include "sys/time.h"

namespace grace {
	namespace {
		void* alloc_condition_cb(unsigned int condtype) {
			return new std::condition_variable_any;
		}
		void free_condition_cb(void* cond) {
			delete reinterpret_cast<std::condition_variable_any*>(cond);
		}
		int signal_condition_cb(void* cond, int broadcast) {
			auto c = reinterpret_cast<std::condition_variable_any*>(cond);
			if (broadcast) {
				c->notify_all();
			} else {
				c->notify_one();
			}
			return 0;
		}
		int wait_condition_cb(void* cond, void* lock, const struct timeval* timeout) {
			auto c = reinterpret_cast<std::condition_variable_any*>(cond);
			auto l = reinterpret_cast<std::recursive_mutex*>(lock);
			std::unique_lock<std::recursive_mutex> ul(*l);
			std::chrono::seconds t(timeout->tv_sec);
			std::chrono::microseconds ut(timeout->tv_usec);
			auto status = c->wait_for(ul, t+ut);
			return status == std::cv_status::timeout ? 1 : 0;
		}
		
		const evthread_condition_callbacks std_condition_callbacks = {
			.condition_api_version = EVTHREAD_CONDITION_API_VERSION,
			.alloc_condition = alloc_condition_cb,
			.free_condition = free_condition_cb,
			.signal_condition = signal_condition_cb,
			.wait_condition = wait_condition_cb,
		};
		
		void* alloc_mutex_cb(unsigned int locktype) {
			//ASSERT(locktype == EVTHREAD_LOCKTYPE_RECURSIVE);
			return new std::recursive_mutex;
		}
		
		void free_mutex_cb(void* lock, unsigned int locktype) {
			delete reinterpret_cast<std::recursive_mutex*>(lock);
		}
		
		int lock_mutex_cb(unsigned int mode, void *lock) {
			auto p = reinterpret_cast<std::recursive_mutex*>(lock);
			if (mode & EVTHREAD_TRY) {
				return p->try_lock() ? 0 : 1;
			} else {
				p->lock();
				return 0;
			}
		}
		
		int unlock_mutex_cb(unsigned int mode, void* lock) {
			auto p = reinterpret_cast<std::recursive_mutex*>(lock);
			p->unlock();
			return 0;
		}
		
		const evthread_lock_callbacks std_lock_callbacks = {
			.lock_api_version = EVTHREAD_LOCK_API_VERSION,
			
			.supported_locktypes = EVTHREAD_LOCKTYPE_RECURSIVE,
			.alloc = alloc_mutex_cb,
			.free = free_mutex_cb,
			.lock = lock_mutex_cb,
			.unlock = unlock_mutex_cb,
		};
		
		unsigned long thread_id_cb() {
			return std::hash<std::thread::id>()(std::this_thread::get_id());
		}
	}

	void libevent_evthread_use_stdlib() {
		evthread_set_lock_callbacks(&std_lock_callbacks);
		evthread_set_condition_callbacks(&std_condition_callbacks);
		evthread_set_id_callback(thread_id_cb);
	}
}
