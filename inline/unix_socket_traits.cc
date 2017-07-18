/*
 * unix_socket_traits.icc
 * Author: Mark Swoope
 * Date: July 2017
 */

#include <unistd.h>
#include <sys/ioctl.h>
#include <ios>

struct socket_traits {
	using socket_type = int;
	
	static socket_type invalid()
	{
		return -1;
	}

	static std::streamsize read(socket_type socket, void* buf,
						std::streamsize n)
	{
		return ::read(socket, buf, n);
	}

	static std::streamsize write(socket_type socket, const void* buf,
							std::streamsize n)
	{
		return ::write(socket, buf, n);
	}

	static int close(socket_type socket)
	{
		return ::close(socket);
	}
};
