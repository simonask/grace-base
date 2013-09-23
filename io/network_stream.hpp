#pragma once
#ifndef GRACE_NETWORK_STREAM_HPP_INCLUDED
#define GRACE_NETWORK_STREAM_HPP_INCLUDED

#include "io/input_stream.hpp"
#include "io/output_stream.hpp"
#include "event/event_loop.hpp"
#include "base/error.hpp"

namespace grace {
	struct NetworkStreamError : ErrorBase<NetworkStreamError> {};

	struct INetworkStream : IInputStream, IInputStreamNonblocking, IOutputStream, IOutputStreamNonblocking {
		virtual uintptr_t handle() const = 0; // sockfd
		virtual bool is_open() const = 0;
		virtual void close() = 0;
		virtual StringRef host() const = 0;
		virtual StringRef address() const = 0;
		virtual uint16 port() const = 0;
		virtual uint16 local_port() const = 0;
	};
	
	struct NetworkStream : INetworkStream {
		// Connect synchronously:
		static UniquePtr<INetworkStream> connect(StringRef host, uint16 port, IAllocator& = default_allocator());
	};
}

#endif
