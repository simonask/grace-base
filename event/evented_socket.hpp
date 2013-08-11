//
//  evented_socket.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 20/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_evented_socket_hpp
#define grace_evented_socket_hpp

#include "io/input_stream.hpp"
#include "io/output_stream.hpp"
#include "base/basic.hpp"
#include "event/event_handle.hpp"
#include "base/function.hpp"

#include <algorithm>

namespace grace {
	enum class SocketEvent : uint8 {
		Read  = 1 << 0,
		Write = 1 << 1,
		AllEventsMask = 0xff
	};
	ENUM_IS_FLAGS(SocketEvent);

	/*
	 IEventedSocket represents a buffered socket connection.
	 That is, received data must be buffered by the backend, but 
	*/
	struct IEventedSocket : IEventHandle, InputStream, OutputStream {
		using DataCallback = Function<void(IEventedSocket&, SocketEvent event)>;
		using ConnectionErrorCallback = Function<void(StringRef, bool fatal)>;
		using ConnectionEstablishedCallback = Function<void()>;
		using ConnectionClosedCallback = Function<void()>;
		
		virtual void on_data(uint8 event_mask, DataCallback callback) = 0;
		virtual void on_error(ConnectionErrorCallback callback) = 0;
		virtual void on_connect(ConnectionEstablishedCallback callback) = 0;
		virtual void on_close(ConnectionClosedCallback callback) = 0;
		
		virtual size_t data_available() const = 0;
		virtual void send_data(ArrayRef<byte> data) = 0;
		virtual void close() = 0; // Implementors should call this in destructor.
	};
	
	// Implements InputStream and OutputStream in terms of received_data/send_data
	struct EventedSocketBase : IEventedSocket {
		// InputStream interface
		bool is_readable() const {
			return true;
		}
		size_t tell_read() const {
			return 0;
		}
		bool seek_read(size_t position) {
			return false;
		}
		bool has_length() const {
			return false;
		}
		size_t length() const {
			return SIZE_MAX;
		}
		
		// OutputStream interface
		bool is_writable() const {
			return true;
		}
		size_t write(const byte* buffer, size_t max) {
			send_data(ArrayRef<byte>((byte*)buffer, (byte*)buffer + max));
			return max;
		}
		size_t tell_write() const {
			return 0;
		}
		bool seek_write(size_t position) {
			return false;
		}
		void flush() final {
		}
	};
}

#endif
