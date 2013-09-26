#pragma once
#ifndef GRACE_EVENT_CAPABILITIES_HPP_INCLUDED
#define GRACE_EVENT_CAPABILITIES_HPP_INCLUDED

#include "memory/unique_ptr.hpp"
#include "base/string.hpp"
#include "base/function.hpp"
#include "event/event_handle.hpp"

namespace grace {
	struct INetworkStream;
	struct Server;
	struct Process;
	struct ConsoleStream;

	enum class NetworkConnectionEvent;
	enum class ServerEvent;
	enum class ProcessEvent;
	enum class StdInEvent;

	namespace capability {
		struct Connect {
			virtual UniquePtr<IEventHandle> connect(StringRef host, uint16 port, Function<void(NetworkConnectionEvent, INetworkStream&)> callback, SystemTimeDelta timeout) = 0;
		};
		struct Listen {
			virtual UniquePtr<IEventHandle> listen(uint16 port, Function<void(ServerEvent, Server&)> callback) = 0;
		};
		struct POpen {
			virtual UniquePtr<IEventHandle> popen(StringRef command, ArrayRef<StringRef> arguments, Function<void(ProcessEvent, Process&)> callback, SystemTimeDelta timeout) = 0;
		};
		struct StdIn {
			virtual UniquePtr<IEventHandle> stdin(Function<void(StdInEvent, ConsoleStream&)> callback, SystemTimeDelta timeout) = 0;
		};
	}
}

#endif