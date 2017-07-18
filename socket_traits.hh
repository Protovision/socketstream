/*
 * socket_traits.hh
 * Author: Mark Swoope
 * Date: July 2017
 */
#ifndef SOCKET_TRAITS_HH
#define SOCKET_TRAITS_HH

/*
 * socket_traits contains common information and operations for the operating
 * system's socket implementation. It contains the following members:
 *
 * Member types:
 * 	socket_type:
 * 		A native handle to a socket.
 * Member functions:
 * 	static socket_type invalid():
 * 		Returns an invalid socket_type value.
 * 	static std::streamsize read(socket_type, void*, std::streamsize):
 * 		Reads from a socket into a byte buffer.
 * 	static std::streamsize write(socket_type, const void*,
 * 						std::streamsize):
 * 		Writes from a byte buffer into a socket.
 * 	static int close(socket_type):
 * 		Closes a socket
 */

#if defined(__linux__) || defined(__APPLE__)
#include "inline/unix_socket_traits.icc"
#elif defined(_WIN32)
#include "inline/win32_socket_traits.icc"
#endif

#endif
