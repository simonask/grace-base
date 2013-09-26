#include "io/reactor.hpp"
#include "event/capabilities.hpp"
#include "base/raise.hpp"

namespace grace {
	namespace reactor {
		namespace {
			template <typename Capability, typename Base>
			Capability* check_capability(Base& base) {
				return dynamic_cast<Capability*>(&base);
			}
		}

		UniquePtr<IEventHandle> connect(IEventLoop& loop, StringRef host, uint16 port, Function<void(NetworkConnectionEvent, INetworkStream&)> callback, SystemTimeDelta timeout, bool allow_synchronous) {
			auto c = check_capability<capability::Connect>(loop);
			if (c) {
				return c->connect(host, port, std::move(callback), timeout);
			} else if (allow_synchronous) {
				ASSERT(false); // TODO
			} else {
				raise<ReactorError>("Reactive connect() called with an event loop that doesn't support asyncronous connect, and allow_synchronous was false.");
			}
		}

		UniquePtr<IEventHandle> listen(IEventLoop& loop, uint16 port, Function<void(ServerEvent, Server&)> callback, bool allow_synchronous) {
			auto c = check_capability<capability::Listen>(loop);
			if (c) {
				return c->listen(port, std::move(callback));
			} else if (allow_synchronous) {
				ASSERT(false); // TODO
			} else {
				raise<ReactorError>("Reactive listen() called with an event loop that doesn't support asyncronous listen, and allow_synchronous was false.");
			}
		}

		UniquePtr<IEventHandle> popen(IEventLoop& loop, StringRef command, ArrayRef<StringRef> arguments, Function<void(ProcessEvent, Process&)> callback, SystemTimeDelta timeout, bool allow_synchronous) {
			auto c = check_capability<capability::POpen>(loop);
			if (c) {
				return c->popen(command, arguments, std::move(callback), timeout);
			} else if (allow_synchronous) {
				ASSERT(false); // TODO
			} else {
				raise<ReactorError>("Reactive popen() called with an event loop that doesn't support asyncronous popen, and allow_synchronous was false.");
			}
		}

		UniquePtr<IEventHandle> stdin(IEventLoop& loop, Function<void(StdInEvent, ConsoleStream&)> callback, SystemTimeDelta timeout, bool allow_synchronous) {
			auto c = check_capability<capability::StdIn>(loop);
			if (c) {
				return c->stdin(std::move(callback), timeout);
			} else if (allow_synchronous) {
				ASSERT(false); // TODO
			} else {
				raise<ReactorError>("Reactive stdin() called with an event loop that doesn't support asyncronous stdin, and allow_synchronous was false.");
			}
		}
	}
}