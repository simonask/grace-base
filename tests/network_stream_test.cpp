#include "io/network_stream.hpp"
#include "tests/test.hpp"
#include "base/either_switch.hpp"

using namespace grace;

SUITE(NetworkStream) {
	feature("blocking connect", []() {
		auto econn = NetworkStream::connect("google.com", 80);
		TEST(econn.get()).should != nullptr;
	});

	feature("blocking HTTP", []() {
		auto stream = NetworkStream::connect("google.com", 80);
		TEST(stream.get()).should != nullptr;
		FormattedStream fs(*stream);
		fs.printf("GET / HTTP/1.0\nUser-Agent: {0}\n\n", "grace-network-test");

		StringStream ss;
		byte buffer[256];
		size_t len;
		bool running = true;
		while (running) {
			either_switch(stream->read(buffer, 256),
				[&](size_t n) {
					ss << StringRef((char*)buffer, n);
				},
				[&](IOEvent ev) {
					running = false;
				}
			);
		}

		String response = ss.string();
		Regex r("HTTP/1.0");

		TEST(r.match(response)).should == true;
	});
}