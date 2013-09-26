#pragma once
#ifndef GRACE_REACTOR_HPP_INCLUDED
#define GRACE_REACTOR_HPP_INCLUDED

#include "base/string.hpp"
#include "base/function.hpp"
#include "memory/unique_ptr.hpp"
#include "event/event_handle.hpp"
#include "event/event_loop.hpp"
#include "base/error.hpp"

namespace grace {
	struct IEventLoop;
	struct INetworkStream;
	struct Server;
	struct Process;
	struct ConsoleStream;

	enum class NetworkConnectionEvent {
		Read    = 1,
		Write   = 1 << 1,
		Error   = 1 << 2,
		Timeout = 1 << 3,
		Closed  = 1 << 4,
		Ready   = 1 << 5,
		Any     = 0xff,
	};
	ENUM_IS_FLAGS(NetworkConnectionEvent);

	enum class ServerEvent {
		Accept = 1,
		Error  = 1 << 2,
		Closed = 1 << 4,
		Ready  = 1 << 5,
		Any    = 0xff
	};
	ENUM_IS_FLAGS(ServerEvent);

	enum class ProcessEvent {
		StdOut  = 1,
		Read    = StdOut,
		StdErr  = 1 << 1,
		Timeout = 1 << 3,
		Closed  = 1 << 4,
		Ready   = 1 << 5,
		Any     = 0xff
	};
	ENUM_IS_FLAGS(ProcessEvent);

	enum class StdInEvent {
		Read    = 1,
		Timeout = 1 << 3,
		Any     = 0xff
	};
	ENUM_IS_FLAGS(StdInEvent);

	struct ReactorError : ErrorBase<ReactorError> {};

	namespace reactor {
		UniquePtr<IEventHandle> connect(IEventLoop&, StringRef host, uint16 port, Function<void(NetworkConnectionEvent, INetworkStream&)> callback, SystemTimeDelta timeout = SystemTimeDelta::forever(), bool allow_synchronous = false);
		UniquePtr<IEventHandle> listen(IEventLoop&,  uint16 port, Function<void(ServerEvent, Server&)> callback, bool allow_synchronous = false);
		UniquePtr<IEventHandle> popen(IEventLoop&,   StringRef command, ArrayRef<StringRef> arguments, Function<void(ProcessEvent, Process&)> callback, SystemTimeDelta timeout = SystemTimeDelta::forever(), bool allow_synchronous = false);
		UniquePtr<IEventHandle> stdin(IEventLoop&,   Function<void(StdInEvent, ConsoleStream&)> callback, SystemTimeDelta timeout = SystemTimeDelta::forever(), bool allow_synchronous = false);
	}
}

#endif
