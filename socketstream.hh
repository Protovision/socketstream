#ifndef SWOOPE_SOCKETSTREAM_HH
#define SWOOPE_SOCKETSTREAM_HH

#define SWOOPE_SOCKETSTREAM_VERSION 170731L

#include "src/native_socket_traits.hh"
#include "src/basic_socketbuf.hh"
#include "src/basic_socketstream.hh"

namespace swoope {

	typedef basic_socketbuf<native_socket_traits> socketbuf;
	typedef basic_socketstream<native_socket_traits> socketstream;

	/*
	 * TODO: Make interface to an SSL socket library
	 *
	 * typedef basic_socketstream<openssl_socket_traits> openssl_socketstream;
	 */
}

#endif
