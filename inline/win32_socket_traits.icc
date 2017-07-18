/*
 * win32_socket_traits.icc
 * Author: Mark Swoope
 * Date: July 2017
 */

#include <Winsock2.h>
#include <ios>

struct socket_traits {
	using socket_type = SOCKET;

	static socket_type invalid()
	{
		return INVALID_SOCKET;
	}

	static std::streamsize read(socket_type socket, void* buf,
						std::streamsize n)
	{
		return ::recv(socket, (char*)buf, n, 0);
	}

	static std::streamsize write(socket_type socket, const void* buf,
							std::streamsize n)
	{
		return ::send(socket, (const char*)buf, n, 0);
	}

	static int close(socket_type socket)
	{
		return (::closesocket(socket) == 0) ? 0 : -1;
	}
};
