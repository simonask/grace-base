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
}