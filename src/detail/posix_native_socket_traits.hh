#ifndef SWOOPE_POSIX_SOCKETS_HH
#define SWOOPE_POSIX_SOCKETS_HH

/*
 * posix_native_socket_traits.hh
 * Author: Mark Swoope
 * Date: July 2017
 */

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <ios>
#include <string>

namespace swoope {

	struct native_socket_traits {
		typedef int socket_type;
	
		enum class shutdown_mode {
			read = SHUT_RD,
			write = SHUT_WR,
			rdwr = SHUT_RDWR
		};

		static socket_type invalid()
		{
			return std::move(-1);
		}

		static socket_type open(const std::string& host,
					const std::string& service)
		{
			using std::swap;
			addrinfo *ai, hints{};
			socket_type result{std::move(invalid())};

			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			if (::getaddrinfo(host.c_str(), service.c_str(),
							&hints, &ai) != 0)
				return std::move(result);
			socket_type socket{std::move(::socket(ai->ai_family,
							ai->ai_socktype,
							ai->ai_protocol))};
			if (socket != result && ::connect(socket, ai->ai_addr,
							ai->ai_addrlen) == 0)
				swap(result, socket);
			::freeaddrinfo(ai);
			return std::move(result);
		}

		static std::streamsize read(socket_type& socket,
						void* buf,
						std::streamsize n)
		{
			return ::recv(socket, buf, n, 0);
		}

		static std::streamsize write(socket_type& socket,
						const void* buf,
						std::streamsize n)
		{
			return ::send(socket, buf, n, 0);
		}

		static int shutdown(socket_type& socket, shutdown_mode how)
		{
			return ::shutdown(socket, (int)how);
		}

		static int close(socket_type& socket)
		{
			return ::close(socket);
		}
	};

}

#endif
