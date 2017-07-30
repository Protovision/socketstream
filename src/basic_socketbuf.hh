#ifndef SWOOPE_BASIC_SOCKETBUF_HH
#define SWOOPE_BASIC_SOCKETBUF_HH

/*
 * basic_socketbuf.hh
 * Author: Mark Swoope
 * Date: July 2017
 */

#include <streambuf>
#include <ios>
#include <memory>
#include <algorithm>
#include <unordered_map>

namespace swoope {

	template <class SocketTraits>
	struct basic_socketbuf_base {
		/* Socket handle */
		typename SocketTraits::socket_type socket;

		/* Buffer used for unbuffered I/O */
		char buf[1];

		/* Start of buffer */
		std::shared_ptr<char> base;

		std::streamsize
			gasize, /* get area size */
			pasize; /* put area size */

		std::ios_base::openmode mode;

		bool is_open;

		basic_socketbuf_base();
		basic_socketbuf_base(
				const basic_socketbuf_base& rhs) = default;
		~basic_socketbuf_base() = default;

		void swap(basic_socketbuf_base& rhs);
	};

	template <class SocketTraits>
	void swap(basic_socketbuf_base<SocketTraits>& a,
			basic_socketbuf_base<SocketTraits>& b);

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
		basic_socketbuf(basic_socketbuf&& rhs);
		virtual ~basic_socketbuf();

		basic_socketbuf& operator=(
				const basic_socketbuf& rhs) = delete;
		basic_socketbuf& operator=(basic_socketbuf&& rhs);
		void swap(basic_socketbuf& rhs);

		bool is_open() const;
		basic_socketbuf* open(socket_type&& s,
					std::ios_base::openmode mode);
		basic_socketbuf* open(const std::string& host,
					const std::string& service,
					std::ios_base::openmode mode);
		basic_socketbuf* shutdown(std::ios_base::openmode how);
		basic_socketbuf* close();
		socket_type* rdsocket();
	protected:
		basic_socketbuf* setbuf(char_type* s, std::streamsize n);
		int sync();
		std::streamsize xsgetn(char_type* s, std::streamsize n);
		int_type underflow();
		std::streamsize xsputn(const char_type* s, std::streamsize n);
		int_type overflow(int_type c = traits_type::eof());
	private:
		basic_socketbuf(const basic_socketbuf& rhs);
		void init_io();
		std::streamsize read(char_type* s, std::streamsize n);
		std::streamsize write(const char_type* s, std::streamsize n);
	};

	template <class SocketTraits>
	void swap(basic_socketbuf<SocketTraits>& a,
		basic_socketbuf<SocketTraits>& b);
}

#include "impl/basic_socketbuf.cc"

#endif
