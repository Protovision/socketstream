#ifndef SWOOPE_BASIC_SOCKETBUF_HH
#define SWOOPE_BASIC_SOCKETBUF_HH

/*
 * basic_socketbuf.hh
 * Author: Mark Swoope
 * Date: July 2017
 */

#include <streambuf>
#include <ios>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace swoope {

	template <class SocketTraits>
	class basic_socketbuf_base {
	public:
		/* Socket handle */
		typename SocketTraits::socket_type socket;

		/* Buffer used for unbuffered I/O */
		char buf[1];

		/* Start of buffer */
		char* base;

		std::streamsize
			gasize, /* get area size */
			pasize; /* put area size */

		std::ios_base::openmode mode;

		bool is_open, auto_delete_base;

		basic_socketbuf_base();
#if __cplusplus >= 201103L
		basic_socketbuf_base(const basic_socketbuf_base&) = delete;
		void swap(basic_socketbuf_base& rhs);
#endif
		virtual ~basic_socketbuf_base();
		void release_base();
		void reset_base(char* p, bool auto_delete);

#if __cplusplus < 201103L
	private:
		basic_socketbuf_base(const basic_socketbuf_base&);
#endif
	};

#if __cplusplus >= 201103L
	template <class SocketTraits>
	inline void 
	swap(basic_socketbuf_base<SocketTraits>& a, basic_socketbuf_base<
							SocketTraits>& b)
	{
		a.swap(b);
	}
#endif

	template <class SocketTraits>
	class basic_socketbuf :
	public std::streambuf,
	private basic_socketbuf_base<SocketTraits> {
	public:
		typedef basic_socketbuf_base<SocketTraits>
					__socketbuf_base_type;
		typedef std::streambuf __streambuf_type;
	
		typedef SocketTraits socket_traits_type;
		typedef typename socket_traits_type::socket_type socket_type;

		typedef char char_type;
		typedef std::char_traits<char_type> traits_type;
		typedef typename traits_type::int_type int_type;
		typedef typename traits_type::pos_type pos_type;
		typedef typename traits_type::off_type off_type;

		basic_socketbuf();
#if __cplusplus >= 201103L
		basic_socketbuf(basic_socketbuf&& rhs);
#endif
		virtual ~basic_socketbuf();

#if __cplusplus >= 201103L
		basic_socketbuf& operator=(
				const basic_socketbuf& rhs) = delete;
		basic_socketbuf& operator=(basic_socketbuf&& rhs);
		void swap(basic_socketbuf& rhs);
#endif
		bool is_open() const;
		/* 
		 * Associate existing socket descriptor s to this streambuf. Returns this
		 * on success.
		 */
		basic_socketbuf* open(socket_type s,
					std::ios_base::openmode mode);
		/* 
		 * Create an underlying TCP/IP socket and connect it to the specified host on
		 * the specified port or service. Returns this on success.
		 */
		basic_socketbuf* open(const std::string& host,
					const std::string& service,
					std::ios_base::openmode mode);
		/* 
		 * Create an underlying TCP/IP socket, bind it to the specified port or
		 * service, then make it listen for connections with the specified backlog.
		 * Returns this on success.
		 */
		basic_socketbuf* open(const std::string& service, int backlog);
		/*
		 * Accepts a pending connection from this socket and stores the resulting 
		 * connected socket into socketbuf_result. The string representation of 
		 * the connected socket's address will be stored in address_result. Returns
		 * this on success.
		 */
		basic_socketbuf* accept(basic_socketbuf& d_socketbuf);
		/*
		 * Returns a string representing the address to which the socket is
		 * bound.
		 */
		std::string local_address() const;
		/*
		 * Returns a string representing the address of the peer that is connected
		 * to the socket.
		 */
		std::string remote_address() const;
		/*
		 * Shutdowns down the socket for input, output, or both. Possible values
		 * for how are: std::ios_base::in, std::ios_base::out, or
		 * std::ios_base::in | std::ios_base::out. Returns this on success.
		 */
		basic_socketbuf* shutdown(std::ios_base::openmode how);
		/* Closes the socket. Returns this on success. */
		basic_socketbuf* close();
		/* Returns the underlying socket descriptor. */
		socket_type socket() const;
	protected:
		basic_socketbuf* setbuf(char_type* s, std::streamsize n);
		int sync();
		std::streamsize xsgetn(char_type* s, std::streamsize n);
		int_type underflow();
		std::streamsize xsputn(const char_type* s, std::streamsize n);
		int_type overflow(int_type c = traits_type::eof());
	private:
#if __cplusplus < 201103L
		basic_socketbuf& operator=(const basic_socketbuf& rhs);
#endif
		basic_socketbuf(const basic_socketbuf& rhs);
		void init_io();
		std::streamsize read(char_type* s, std::streamsize n);
		std::streamsize write(const char_type* s, std::streamsize n);
	};

#if __cplusplus >= 201103L
	template <class SocketTraits>
	inline void 
	swap(basic_socketbuf<SocketTraits>& a,
		basic_socketbuf<SocketTraits>& b)
	{
		a.swap(b);
	}
#endif

}

#include "impl/basic_socketbuf.cc"

#endif


