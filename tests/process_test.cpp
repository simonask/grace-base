#include "tests/test.hpp"
#include "base/process.hpp"
#include "io/util.hpp"
#include "event/event_loop.hpp"
#include "io/reactor.hpp"

using namespace grace;

SUITE(Process) {
	feature("read from child process stdout", []() {
		StringRef args[] = {"-n", "hello"};
		Process p = Process::popen("echo", AREF(args));
		auto output = read_string(p.stdout());
		p.wait();
		TEST(p.exit_status()).should == 0;
		TEST(output).should == "hello";
	});

	feature("read from child process stderr", []() {
		StringRef args[] = {"-j"};
		Process p = Process::popen("ls", AREF(args));
		p.wait();
		auto out = read_string(p.stdout());
		auto err = read_string(p.stderr());
		TEST(out).should == "";
		TEST(err.size()).should > 0;
		TEST(p.exit_status()).should != 0;
	});

	feature("reactive read", []() {
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
					bool available = true;
					byte buffer[128];
					while (available) {
						either_switch(process.stdout().read(buffer, 128),
							[&](size_t n) {
								ss.write(buffer, n);
							},
							[&](IOEvent ev) {
								available = false;
							}
						);
					}
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
		//TEST(timeout).should == false;
		TEST(err_output).should == false;
		TEST(ss.string()).should == "Hello, World!";
	});
}