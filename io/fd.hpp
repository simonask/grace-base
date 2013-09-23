#pragma once
#ifndef GRACE_FD_HPP_INCLUDED
#define GRACE_FD_HPP_INCLUDED

namespace grace {
	using FileDescriptor = int;

	bool is_nonblocking(FileDescriptor fd);
	void set_nonblocking(FileDescriptor fd, bool);
}

#endif