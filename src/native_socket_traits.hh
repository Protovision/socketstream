#ifndef SWOOPE_NATIVE_SOCKET_TRAITS_HH
#define SWOOPE_NATIVE_SOCKET_TRAITS_HH

/*
 * native_socket_traits.hh
 * Author: Mark Swoope
 * Date: July 2017
 */

#if defined(__linux__) || \
	defined(__APPLE__) || \
	defined(_XOPEN_SOURCE)
#include "detail/posix_native_socket_traits.hh"
#elif defined(__WINDOWS__) || \
	defined(_WIN32) || \
	defined(__WIN32__)
#include "detail/winsock_init.hh"
#include "detail/winsock_native_socket_traits.hh"
#endif

#endif
