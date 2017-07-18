/*
 * net_utility.hh
 * Author: Mark Swoope
 * Date: July 2017
 */
#ifndef NET_UTILITY_HH
#define NET_UTILITY_HH
#include "socket_traits.hh"
#include <string>
#include <cstring>

/*
 * Initializes and deinitializes the operating system's socket layer.
 * This is here only for Windows, does nothing on UNIX conforming OS's.
 */
int sockets_init();
int sockets_quit();

/*
 * Combines the functionality of getaddrinfo, socket, and connect.
 *
 * Looks up the host node, creates a socket, and connects that socket 
 * to the host on a specified service.
 *
 * Returns the connected socket or socket_traits::invalid() on error.
 */
socket_traits::socket_type make_connected_socket(int family,
						int socktype,
						int protocol,
						const std::string& node,
						const std::string& service);

#include "inline/net_utility.cc"

#endif
