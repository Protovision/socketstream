#ifndef SOCKETBUF_HH
#define SOCKETBUF_HH
#include "socket_traits.hh"
#include <streambuf>
#include <algorithm>

class socketbuf : public std::streambuf {
public:
	using socket_type = socket_traits::socket_type;
private:
	socket_type socket_;
	char_type* buf_;
	std::streamsize gasize_, pasize_;
	bool userbuf_;
protected:
	socketbuf* setbuf(char_type* s, std::streamsize n);
	int sync();
	int_type underflow();
	int_type overflow(int_type c = traits_type::eof());
	std::streamsize xsgetn(char_type* s, std::streamsize n);
	std::streamsize xsputn(const char_type* s, std::streamsize n);
public:
	socketbuf();
	socketbuf(const socketbuf&) = delete;
	socketbuf(socketbuf&& other);
	virtual ~socketbuf();
	socketbuf& operator=(const socketbuf&) = delete;
	socketbuf& operator=(socketbuf&&);

	socketbuf* close();
	
	std::streamsize out_pending() const
	{
		return (pptr() - pbase());
	}

	void purge(std::ios_base::openmode which =
			std::ios_base::in | std::ios_base::out)
	{
		if (which & std::ios_base::in)
			setg(eback(), eback(), eback());
		if (which & std::ios_base::out)
			setp(pbase(), epptr());
	}

	socketbuf* socket(socket_type socket);

	socket_type socket() const
	{
		return socket_;
	}

	void swap(socketbuf& other);

	bool valid() const
	{
		return (socket_ != socket_traits::invalid());
	}
};

namespace std {
	void swap(socketbuf& a, socketbuf& b)
	{
		a.swap(b);
	}
};

#include "inline/socketbuf.icc"

#endif
