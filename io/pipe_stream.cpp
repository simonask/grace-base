#include "io/pipe_stream.hpp"

#include <errno.h>
#include "base/raise.hpp"

namespace grace {
	PipeStreamBase::PipeStreamBase(PipeStreamBase&& other) {
		swap(other);
	}

	PipeStreamBase& PipeStreamBase::operator=(PipeStreamBase&& other) {
		close();
		swap(other);
		return *this;
	}

	PipeStreamBase::~PipeStreamBase() {
		close();
	}

	bool PipeStreamBase::is_nonblocking() const {
		return grace::is_nonblocking(fd);
	}

	void PipeStreamBase::set_nonblocking(bool b) {
		grace::set_nonblocking(fd, b);
	}

	bool PipeStreamBase::is_open() const {
		return fd >= 0;
	}

	void PipeStreamBase::close() {
		if (is_open()) {
			::close(fd);
			fd = -1;
		}
	}

	void PipeStreamBase::swap(PipeStreamBase& other) {
		std::swap(fd, other.fd);
		std::swap(position, other.position);
	}

	Either<size_t, IOEvent> InputPipeStream::read(byte* buffer, size_t max) {
		ssize_t n = ::read(fd, buffer, max);
		if (n < 0) {
			if (errno == EWOULDBLOCK) {
				return IOEvent::WouldBlock;
			} else {
				raise<PipeError>("read: {0}", ::strerror(errno));
			}
		} else if (n == 0 && max != 0) {
			close();
			return IOEvent::EndOfStream;
		}
		position += n;
		return (size_t)n;
	}

	Either<size_t, IOEvent> OutputPipeStream::write(const byte* buffer, size_t max) {
		ssize_t n = ::write(fd, buffer, max);
		if (n < 0) {
			if (errno == EWOULDBLOCK) {
				return IOEvent::WouldBlock;
			} else {
				raise<PipeError>("write: {0}", ::strerror(errno));
			}
		} else if (n == 0 && max != 0) {
			close();
			return IOEvent::EndOfStream;
		}
		position += n;
		return (size_t)n;
	}
}