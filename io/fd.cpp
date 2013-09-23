#include "io/fd.hpp"
#include <fcntl.h>

namespace grace {
	bool is_nonblocking(FileDescriptor fd) {
		int flags = ::fcntl(fd, F_GETFL, 0);
		return (flags & O_NONBLOCK) != 0;
	}

	void set_nonblocking(FileDescriptor fd, bool b) {
		int flags = ::fcntl(fd, F_GETFL, 0);
		if (b) {
			flags |= O_NONBLOCK;
		} else {
			flags &= ~O_NONBLOCK;
		}
		::fcntl(fd, F_SETFL, flags);
	}
}