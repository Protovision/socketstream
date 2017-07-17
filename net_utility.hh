#ifndef NET_UTILITY_HH
#define NET_UTILITY_HH
#include "socket_traits.hh"
#include <string>
#include <cstring>

int sockets_init();
int sockets_quit();
socket_traits::socket_type make_connected_socket(int family,
						int socktype,
						int protocol,
						const std::string& node,
						const std::string& service);

#include "inline/net_utility.icc"

#endif
