/* 
 * socketstream.hh
 * Author: Mark Swoope
 * Date: July 2017
 */
#ifndef SOCKETSTREAM_HH
#define SOCKETSTREAM_HH
#include "socketbuf.hh"
#include <iostream>

/*
 * socketstream is a std::iostream that provides formatted I/O on a socket.
 */
class socketstream : public std::iostream {
public:
	/*
	 * The member type socket_type shall be an alias for the OS-specific
	 * native handle to a socket.
	 */
	using socket_type = socket_traits::socket_type;
private:
	mutable socketbuf buf_;
public:
	/* Default constructor */
	socketstream() :
		std::iostream(&buf_),
		buf_()
	{
	}

	/*
	 * Default constructs, then associates a socket with this stream by 
	 * calling socket(s).
	 */
	explicit socketstream(socket_type s) :
		std::iostream(&buf_),
		buf_()
	{
		socket(s);
	}

	/* Cannot be copy-constructed */
	socketstream(const socketstream&) = delete;

	/* 
	 * Move constructor. Moves constructs the std::iostream state and 
	 * the socketbuf, then calls set_rdbuf() to update the rdbuf()
	 * pointer in the base class.
	 */
	socketstream(socketstream&& other) :
		std::iostream(std::move(other)),
		buf_(std::move(other.buf_))
	{
		set_rdbuf(&buf_);
	}

	/* 
	 * Destructs the socketstream, the socketbuf, and closes the socket.
	 */
	virtual ~socketstream() = default;

	/* Cannot be copy-assigned */
	socketstream& operator=(const socketstream&) = delete;

	/*
	 * Move assigns socketstream other into *this. other will be left 
	 * in a default state.
	 */
	socketstream& operator=(socketstream&& other)
	{
		std::iostream::operator=(std::move(other));
		buf_.operator=(std::move(other.buf_));
		return *this;
	}

	/*
	 * Closes the associated socket by calling rdbuf()->close. On error,
	 * setstate(failbit) is called.
	 */
	void close()
	{
		if (buf_.close() == nullptr)
			setstate(failbit);
	}

	/* Returns a pointer to the underlying socketbuf */
	socketbuf* rdbuf() const
	{
		return &buf_;
	}

	/* 
	 * Returns the native socket handle by calling rdbuf()->socket().
	 */
	socket_type socket() const
	{
		return buf_.socket();
	}

	/*
	 * Associates an open socket s with the socket stream by calling 
	 * rdbuf()->socket(s). Calls clear() on success, calls
	 * setstate(failbit) on error.
	 */
	void socket(socket_type s)
	{
		if (buf_.socket(s) == nullptr)
			setstate(failbit);
		else
			clear();
	}

	/*
	 * Exchanges the state of this socketstream with that of another.
	 */
	void swap(socketstream& other)
	{
		std::iostream::swap(other);
		buf_.swap(other.buf_);
	}

	/*
	 * Checks if the socket stream has an associated open socket by 
	 * calling rdbuf()->valid().
	 */
	bool valid() const
	{
		return buf_.valid();
	}
};

/*
 * Specializes the std::swap algorithm for socketstream.
 */
namespace std {
	inline void swap(socketstream& a, socketstream& b)
	{
		a.swap(b);
	}
};

/*
 * Inserts a carriage-return and line-feed characters into the output 
 * sequence os and flushes it.
 *
 * This is an output-only I/O manipulator, it may be called with an expression
 * out << crlf for any out of type std::ostream.
 */
inline std::ostream& crlf(std::ostream& os)
{
	os.put('\r');
	os.put('\n');
	return os.flush();
}

#endif
