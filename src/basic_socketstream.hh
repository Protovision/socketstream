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
	class basic_socketstream : public std::iostream {
	public:
		typedef SocketTraits socket_traits;
		typedef typename socket_traits::socket_type socket_type;

		basic_socketstream() :
			std::iostream(std::addressof(buf_)),
			buf_()
		{
		}

		explicit basic_socketstream(socket_type s,
				std::ios_base::openmode mode =
				std::ios_base::in | std::ios_base::out) :
			std::iostream(std::addressof(buf_)),
			buf_()
		{
			open(s, mode);
		}

		explicit basic_socketstream(
				const std::string& host,
				const std::string& service,
				std::ios_base::openmode mode =
				std::ios_base::in | std::ios_base::out) :
			std::iostream(std::addressof(buf_)),
			buf_()
		{
			open(host, service, mode);
		}

		basic_socketstream(const basic_socketstream&) = delete;

		basic_socketstream(basic_socketstream&& rhs) :
			std::iostream(std::addressof(buf_)),
			buf_(std::move(rhs.buf_))
		{
		}

		virtual ~basic_socketstream() = default;

		basic_socketstream& operator=(
				const basic_socketstream&) = delete;

		basic_socketstream& operator=(basic_socketstream&& rhs)
		{
			std::iostream::move(rhs);
			buf_ = std::move(rhs.buf_);
			set_rdbuf(std::addressof(buf_));
			return *this;
		}

		void swap(basic_socketstream& rhs)
		{
			std::iostream::swap(rhs);
			buf_.swap(rhs.buf_);
		}

		basic_socketbuf<socket_traits>* rdbuf() const
		{
			return const_cast< basic_socketbuf<socket_traits>* >(
							std::addressof(buf_));
		}

		bool is_open() const
		{
			return rdbuf()->is_open();
		}

		void open(socket_type s, std::ios_base::openmode mode =
				std::ios_base::in | std::ios_base::out)
		{
			if (rdbuf()->open(s, mode) == nullptr)
				setstate(failbit);
			else
				clear();
		}

		void open(const std::string& host, const std::string& service,
					std::ios_base::openmode mode =
					std::ios_base::in | std::ios_base::out)
		{
			if (rdbuf()->open(host, service, mode) == nullptr)
				setstate(failbit);
			else
				clear();
		}

		void shutdown(std::ios_base::openmode how)
		{
			if (rdbuf()->shutdown(how) == nullptr)
				setstate(failbit);
		}

		void close()
		{
			if (rdbuf()->close() == nullptr)
				setstate(failbit);
		}

		socket_type socket()
		{
			return rdbuf()->socket();
		}

	private:
		basic_socketbuf<socket_traits> buf_;
	};

	template <class SocketTraits>
	inline void swap(basic_socketstream<SocketTraits>& a,
			basic_socketstream<SocketTraits>& b)
	{
		a.swap(b);
	}

}

#endif
