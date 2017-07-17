#ifndef SOCKETSTREAM_HH
#define SOCKETSTREAM_HH
#include "socketbuf.hh"
#include <iostream>

class socketstream : public std::iostream {
public:
	using socket_type = socket_traits::socket_type;
private:
	mutable socketbuf buf_;
public:
	socketstream() :
		std::iostream{&buf_},
		buf_{}
	{
	}

	explicit socketstream(socket_type s) :
		std::iostream(&buf_),
		buf_()
	{
		socket(s);
	}

	socketstream(const socketstream&) = delete;

	socketstream(socketstream&& other) :
		std::iostream{std::move(other)},
		buf_(std::move(other.buf_))
	{
	}

	~socketstream()
	{
		close();
	}

	socketstream& operator=(const socketstream&) = delete;

	socketstream& operator=(socketstream&& other)
	{
		std::iostream::operator=(std::move(other));
		buf_.operator=(std::move(other.buf_));
		return *this;
	}

	void close()
	{
		if (buf_.close() == nullptr)
			setstate(failbit);
	}

	socketbuf* rdbuf() const
	{
		return &buf_;
	}

	socket_type socket() const
	{
		return buf_.socket();
	}

	void socket(socket_type s)
	{
		if (buf_.socket(s) == nullptr)
			setstate(failbit);
		else
			clear();
	}

	void swap(socketstream& other)
	{
		std::iostream::swap(other);
		buf_.swap(other.buf_);
	}

	bool valid() const
	{
		return buf_.valid();
	}
};

namespace std {
	void swap(socketstream& a, socketstream& b)
	{
		a.swap(b);
	}
};

std::ostream& crlf(std::ostream& os)
{
	return (os << "\r\n" << std::flush);
}

#endif
