#include "io/network_stream.hpp"
#include "base/raise.hpp"
#include "memory/unique_ptr.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

namespace grace {
	struct SocketNetworkStream : NetworkStream {
		int fd = -1;
		String host_;
		String address_;
		uint16 port_ = 0;
		uint16 local_port_ = 0;

		explicit SocketNetworkStream(int fd) : fd(fd) {}

		~SocketNetworkStream() {
			close();
		}

		// NetworkStream
		bool is_open() const final {
			return fd >= 0;
		}

		void close() final {
			if (fd >= 0) {
				::close(fd);
				fd = -1;
			}
		}

		StringRef host() const final { return host_; }
		StringRef address() const final { return address_; }
		uint16 port() const final { return port_; }
		uint16 local_port() const final { return local_port_; }
		uintptr_t handle() const final { return (uintptr_t)fd; }

		// InputStream
		bool is_readable() const final { return true; }
		size_t read(byte* buffer, size_t max) final;
		size_t read_if_available(byte* byffer, size_t max, bool& would_block) final;
		size_t tell_read() const final { return 0; }
		bool seek_read(size_t position) final { return false; }
		bool has_length() const final { return false; }
		size_t length() const final { return SIZE_T_MAX; }

		// OutputStream
		bool is_writable() const final { return true; }
		size_t write(const byte* buffer, size_t max) final;
		size_t write_if_available(const byte* buffer, size_t max, bool& would_block) final;
		size_t tell_write() const final { return 0; }
		bool seek_write(size_t position) final { return false; }
		void flush() final {}
	};

	UniquePtr<NetworkStream> NetworkStream::connect(StringRef host, uint16 port, IAllocator& alloc) {
		int fd = ::socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0) {
			raise<NetworkStreamError>("socket: {0}", ::strerror(errno));
		}
		auto stream = make_unique<SocketNetworkStream>(default_allocator(), fd);
		
		COPY_STRING_REF_TO_CSTR_BUFFER(host_cstr, host);
		struct hostent* server = ::gethostbyname2(host_cstr.data(), AF_INET);
		if (server == nullptr) {
			raise<NetworkStreamError>("gethostbyname2: No such host.");
		}
		stream->host_ = server->h_name;
		stream->address_ = server->h_addr_list[0];
		struct sockaddr_in serv_addr;
		serv_addr.sin_family = AF_INET;
		::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		serv_addr.sin_port = htons(port);
		stream->port_ = port;
		
		if (::connect(fd, (const sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			raise<NetworkStreamError>("connect: {0}", ::strerror(errno));
		}
		
		return move(stream);
	}

	size_t SocketNetworkStream::read(byte* buffer, size_t max) {
		ssize_t n = ::read(fd, buffer, max);
		if (n >= 0) {
			return n;
		} else {
			raise<NetworkStreamError>("read: {0}", ::strerror(errno));
			return 0;
		}
	}

	size_t SocketNetworkStream::read_if_available(byte* buffer, size_t max, bool& would_block) {
		ssize_t n = ::read(fd, buffer, max);
		if (n < 0) {
			if (errno == EAGAIN) {
				would_block = true;
			} else {
				raise<NetworkStreamError>("read: {0}", ::strerror(errno));
			}
			return 0;
		} else {
			would_block = false;
			return (size_t)n;
		}
	}

	size_t SocketNetworkStream::write(const byte* buffer, size_t max) {
		ssize_t n = ::write(fd, buffer, max);
		if (n >= 0) {
			return n;
		} else {
			raise<NetworkStreamError>("write: {0}", ::strerror(errno));
			return 0;
		}
	}

	size_t SocketNetworkStream::write_if_available(const byte* buffer, size_t max, bool& out_would_block) {
		ssize_t n = ::write(fd, buffer, max);
		if (n < 0) {
			if (errno == EAGAIN) {
				out_would_block = true;
			} else {
				raise<NetworkStreamError>("write: {0}", ::strerror(errno));
			}
			return 0;
		} else {
			out_would_block = false;
			return (size_t)n;
		}
	}
}