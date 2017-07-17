#ifndef SOCKET_TRAITS_HH
#define SOCKET_TRAITS_HH

#if defined(__linux__) || defined(__APPLE__)
#include "inline/unix_socket_traits.icc"
#elif defined(_WIN32)
#include "inline/win32_socket_traits.icc"
#endif

#endif
