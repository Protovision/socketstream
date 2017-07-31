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
	
		static socket_type invalid()
		{
			return static_cast<socket_type>(-1);
		}

		static socket_type open(const std::string& host,
					const std::string& service)
		{
			using std::swap;
			addrinfo *ai, hints = addrinfo();
			socket_type result((invalid()));

			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			if (::getaddrinfo(host.c_str(), service.c_str(),
							&hints, &ai) != 0)
				return result;
			socket_type socket((::socket(ai->ai_family,
							ai->ai_socktype,
							ai->ai_protocol)));
			if (socket != result && ::connect(socket, ai->ai_addr,
							ai->ai_addrlen) == 0)
				swap(result, socket);
			::freeaddrinfo(ai);
			return result;
		}

		static std::streamsize read(socket_type socket,
						void* buf,
						std::streamsize n)
		{
			return ::recv(socket, buf, n, 0);
		}

		static std::streamsize write(socket_type socket,
						const void* buf,
						std::streamsize n)
		{
			return ::send(socket, buf, n, 0);
		}

		static int shutdown(socket_type socket, std::ios_base::
							openmode how)
		{
			int result(-1);

			if (how == (std::ios_base::in | std::ios_base::out))
				result = ::shutdown(socket, SHUT_RDWR);
			else if (how == std::ios_base::out)
				result = ::shutdown(socket, SHUT_WR);
			else if (how == std::ios_base::in)
				result = ::shutdown(socket, SHUT_RD);
			return result;
		}

		static int close(socket_type socket)
		{
			return ::close(socket);
		}
	};

}

#endif
