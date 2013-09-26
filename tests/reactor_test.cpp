#include "tests/test.hpp"
#include "io/reactor.hpp"
#include "event/event_loop.hpp"
#include "base/process.hpp"
#include "io/util.hpp"

using namespace grace;

SUITE(Reactor) {
	feature("popen", []() {
		auto loop = create_event_loop();
		StringRef args[] = {"-n", "Hello,", "World!"};
		bool was_ready = false;
		bool timeout = false;
		bool err_output = false;
		StringStream ss;
		UniquePtr<IEventHandle> handle = reactor::popen(*loop, "echo", AREF(args), [&](ProcessEvent event, Process& process) {
			switch (event) {
				case ProcessEvent::Ready: was_ready = true; break;
				case ProcessEvent::StdOut: {
					read_until_event(process.stdout(), ss);
					break;
				}
				case ProcessEvent::Closed: {
					loop->quit();
					break;
				}
				case ProcessEvent::Timeout: {
					timeout = true;
					loop->quit();
					break;
				}
				case ProcessEvent::StdErr: {
					err_output = true;
					break;
				}
				default: break;
			}
		}, SystemTime::seconds(1.f));
		loop->run();

		TEST(was_ready).should == true;
		TEST(timeout).should == false;
		TEST(err_output).should == false;
		TEST(ss.string()).should == "Hello, World!";
	});

	feature("stdin");
	feature("listen");
	feature("connect");
}