#pragma once
#ifndef GRACE_NETWORK_STREAM_HPP_INCLUDED
#define GRACE_NETWORK_STREAM_HPP_INCLUDED

#include "io/input_stream.hpp"
#include "io/output_stream.hpp"
#include "event/event_loop.hpp"
#include "base/error.hpp"

namespace grace {
	struct NetworkStreamError : ErrorBase<NetworkStreamError> {};

	struct INetworkStream : IInputStream, IOutputStream {
		virtual ~INetworkStream() {}
		virtual uintptr_t handle() const = 0; // sockfd
		virtual bool is_open() const = 0;
		virtual void close() = 0;
		virtual StringRef host() const = 0;
		virtual StringRef address() const = 0;
		virtual uint16 port() const = 0;
		virtual uint16 local_port() const = 0;
		virtual bool is_write_nonblocking() const = 0;
		virtual bool is_read_nonblocking() const = 0;
		virtual void set_write_nonblocking(bool) = 0;
		virtual void set_read_nonblocking(bool) = 0;
	};
	
	struct NetworkStream : INetworkStream {
		virtual ~NetworkStream() {}
		// Connect synchronously:
		static UniquePtr<INetworkStream> connect(StringRef host, uint16 port, IAllocator& = default_allocator());
	};
}

#endif
