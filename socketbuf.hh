/*
 * socketbuf.hh
 * Author: Mark Swoope
 * Date: July 2017
 */
#ifndef SOCKETBUF_HH
#define SOCKETBUF_HH
#include "socket_traits.hh"
#include <streambuf>
#include <algorithm>

/*
 * socketbuf is a std::streambuf whose associated character sequence is a 
 * socket. Both the input sequence and output sequence are associated with 
 * the same socket.
 *
 * The functions underflow() and overflow()/sync() perform the actual I/O 
 * between the socket and the get and put areas of the buffer.
 *
 * Reads and writes can be used interchangeably on a socketbuf without any 
 * intervening calls to sync().
 */
class socketbuf : public std::streambuf {
public:
	/*
	 * The member type socket_type shall be an alias for the OS-specific
	 * native handle to a socket.
	 */
	using socket_type = socket_traits::socket_type;
private:
	socket_type socket_;
	char_type* buf_;
	std::streamsize gasize_, pasize_;
	bool userbuf_;
protected:
	/*
	 * Sets the buffer to use for the get and put areas. The size of the
	 * get and put areas will be roughly divided equally within the size 
	 * of the buffer specified by n. If n is 0 or 1, then the stream 
	 * will be unbuffered. If s == nullptr, then a buffer of size n will
	 * be allocated internally.
	 */
	socketbuf* setbuf(char_type* s, std::streamsize n);

	/*
	 * Writes all pending characters from the put area into the associated
	 * socket, then clears the put area. The get area will remain
	 * untouched.
	 */
	int sync();

	int_type underflow();
	int_type overflow(int_type c = traits_type::eof());
	std::streamsize xsgetn(char_type* s, std::streamsize n);
	std::streamsize xsputn(const char_type* s, std::streamsize n);
public:

	/*
	 * Default constructor. All members are set to zero and the 
	 * base class is default constructed. After default construction, 
	 * socketbuf is not associated with a socket and valid() returns 
	 * false.
	 */
	socketbuf();

	/* Cannot be copy-constructed */
	socketbuf(const socketbuf&) = delete;

	/*
	 * Move constructor. Moves the base class state and member state 
	 * from other into *this. After the move, other will be left in the
	 * default state.
	 */
	socketbuf(socketbuf&& other);

	/*
	 * Calls close() and destructs all socketbuf members.
	 */
	virtual ~socketbuf();

	/* Cannot be copy-assigned */
	socketbuf& operator=(const socketbuf&) = delete;

	/* Calls close() and moves other into *this */
	socketbuf& operator=(socketbuf&& other);

	/*
	 * Calls sync(), closes the associated socket, and clears the get and
	 * put areas. Returns *this on success or nullptr on failure. 
	 */
	socketbuf* close();

	/*
	 * Returns the number of characters in the put area, waiting to be 
	 * written to the associated socket.
	 */
	std::streamsize out_pending() const
	{
		return (pptr() - pbase());
	}

	/*
	 * Clears the get area and/or put area base on the value of 
	 * which.
	 */
	void purge(std::ios_base::openmode which =
			std::ios_base::in | std::ios_base::out)
	{
		if (which & std::ios_base::in)
			setg(eback(), eback(), eback());
		if (which & std::ios_base::out)
			setp(pbase(), epptr());
	}

	/*
	 * Associates an open socket with this socketbuf. 
	 * Returns *this on success or nullptr on failure.
	 */
	socketbuf* socket(socket_type socket);

	/*
	 * Returns the underlying native socket handle.
	 */
	socket_type socket() const
	{
		return socket_;
	}

	/*
	 * Exchanges this socketbuf with another. The states of the base 
	 * classes are swapped as well as the socketbuf member states.
	 */
	void swap(socketbuf& other);

	/*
	 * Returns true if this socketbuf is associated with an open socket.
	 */
	bool valid() const
	{
		return (socket_ != socket_traits::invalid());
	}
};

/* Specializes the std::swap algorithm for socketbuf */
namespace std {
	inline void swap(socketbuf& a, socketbuf& b)
	{
		a.swap(b);
	}
};

#include "inline/socketbuf.cc"

#endif
