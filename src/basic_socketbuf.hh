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
		typename SocketTraits::socket_type socket_;

		/* Buffer used for unbuffered I/O */
		char buf_[1];

		/* Start of buffer */
		std::shared_ptr<char> base_;

		std::streamsize
			gasize_, /* get area size */
			pasize_; /* put area size */

		std::ios_base::openmode mode_;

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
		typedef SocketTraits socket_traits;
		typedef typename socket_traits::socket_type socket_type;

		basic_socketbuf();
		basic_socketbuf(basic_socketbuf&& rhs);
		virtual ~basic_socketbuf();

		basic_socketbuf& operator=(
				const basic_socketbuf& rhs) = delete;
		basic_socketbuf& operator=(basic_socketbuf&& rhs);
		void swap(basic_socketbuf& rhs);

		bool is_open() const;
		basic_socketbuf* open(socket_type s,
					std::ios_base::openmode mode);
		basic_socketbuf* open(const std::string& host,
					const std::string& service,
					std::ios_base::openmode mode);
		basic_socketbuf* shutdown(std::ios_base::openmode how);
		basic_socketbuf* close();
		socket_type socket();
	protected:
		basic_socketbuf* setbuf(char_type* s, std::streamsize n);
		int sync();
		std::streamsize xsgetn(char_type* s, std::streamsize n);
		int_type underflow();
		std::streamsize xsputn(const char_type* s, std::streamsize n);
		int_type overflow(int_type c = traits_type::eof());
	private:
		typedef basic_socketbuf_base<socket_traits> socketbuf_base;
		using socketbuf_base::socket_;
		using socketbuf_base::buf_;
		using socketbuf_base::base_;
		using socketbuf_base::gasize_;
		using socketbuf_base::pasize_;
		using socketbuf_base::mode_;

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
