#ifndef BASIC_SOCKETSTREAM_HH
#define BASIC_SOCKETSTREAM_HH

/*
 * basic_socketstream.hh
 * Author: Mark Swoope
 * Date: July 2017
 */

#include "basic_socketbuf.hh"
#include <iostream>

namespace swoope {

	template <class SocketTraits>
	class basic_socketstream :
	public  std::iostream {
	public:
		typedef basic_socketbuf<SocketTraits> __socketbuf_type;
		typedef std::iostream __iostream_type;

		typedef typename __socketbuf_type::socket_type socket_type;
		typedef typename __socketbuf_type::socket_traits_type 
						socket_traits_type;

		typedef char char_type;
		typedef std::char_traits<char> traits_type;
		typedef typename traits_type::int_type int_type;
		typedef typename traits_type::pos_type pos_type;
		typedef typename traits_type::off_type off_type;

		basic_socketstream() :
			__iostream_type(&buf),
			buf()
		{
		}

		explicit basic_socketstream(socket_type s,
				std::ios_base::openmode mode =
				std::ios_base::in | std::ios_base::out) :
			__iostream_type(&buf),
			buf()
		{
			open(s, mode);
		}

		explicit basic_socketstream(
				const std::string& host,
				const std::string& service,
				std::ios_base::openmode mode =
				std::ios_base::in | std::ios_base::out) :
			__iostream_type(&buf),
			buf()
		{
			open(host, service, mode);
		}

#if __cplusplus >= 201103L
		basic_socketstream(const basic_socketstream&) = delete;

		basic_socketstream(basic_socketstream&& rhs) :
			__iostream_type(&buf),
			buf(std::move(rhs.buf))
		{
		}
#endif

		virtual ~basic_socketstream() {}

#if __cplusplus >= 201103L
		basic_socketstream& operator=(
				const basic_socketstream&) = delete;

		basic_socketstream& operator=(basic_socketstream&& rhs)
		{
			this->__iostream_type::move(rhs);
			buf = std::move(rhs.buf);
			this->set_rdbuf(&buf);
			return *this;
		}

		void swap(basic_socketstream& rhs)
		{
			this->__iostream_type::swap(rhs);
			buf.swap(rhs.buf);
		}
#endif

		__socketbuf_type* rdbuf() const
		{
			return const_cast<__socketbuf_type*>(&buf);
		}

		bool is_open() const
		{
			return rdbuf()->is_open();
		}

		void open(socket_type s, std::ios_base::openmode mode =
					std::ios_base::in | std::ios_base::out)
		{
			if (rdbuf()->open(s, mode) == 0)
				this->setstate(std::ios_base::failbit);
			else
				this->clear();
		}

		void open(const std::string& host, const std::string& service,
					std::ios_base::openmode mode =
					std::ios_base::in | std::ios_base::out)
		{
			if (rdbuf()->open(host, service, mode) == 0)
				this->setstate(std::ios_base::failbit);
			else
				this->clear();
		}

		void shutdown(std::ios_base::openmode how)
		{
			if (rdbuf()->shutdown(how) == 0)
				this->setstate(std::ios_base::failbit);
		}

		void close()
		{
			if (rdbuf()->close() == 0)
				this->setstate(std::ios_base::failbit);
		}

	private:
		__socketbuf_type buf;
	};

#if __cplusplus >= 201103L
	template <class SocketTraits>
	inline void swap(basic_socketstream<SocketTraits>& a,
			basic_socketstream<SocketTraits>& b)
	{
		a.swap(b);
	}
#endif

}

#endif
