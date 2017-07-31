/*
 * socketbuf.cc
 * Author: Mark Swoope
 * Date: Jul 2017
 */

namespace swoope {

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
	basic_socketbuf() :
	__streambuf_type(),
	__socketbuf_base_type()
	{
	}

#if __cplusplus >= 201103L
	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
	basic_socketbuf(basic_socketbuf&& rhs) :
	__streambuf_type(),		
	__socketbuf_base_type()
	{
		swap(rhs);
	}
#endif

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
	~basic_socketbuf()
	{
		try {
			close(); 
		} catch (...) {}
	}

#if __cplusplus >= 201103L
	template <class SocketTraits>
	basic_socketbuf<SocketTraits>&
	basic_socketbuf<SocketTraits>::
	operator=(basic_socketbuf&& rhs)
	{
		close();
		this->reset_base(0, false);
		swap(rhs);
	}

	template <class SocketTraits>
	void 
	basic_socketbuf<SocketTraits>::
	swap(basic_socketbuf& rhs)
	{
		__streambuf_type::swap(rhs);
		__socketbuf_base_type::swap(rhs);
	}
#endif

	template <class SocketTraits>
	bool
	basic_socketbuf<SocketTraits>::
	is_open() const
	{
		return this->__socketbuf_base_type::is_open;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::
	open(socket_type socket, std::ios_base::openmode m)
	{
		if (is_open() != false) return 0;
		if (socket == this->__socketbuf_base_type::socket) return 0;
		this->__socketbuf_base_type::socket = socket;
		this->mode = m;
		this->__socketbuf_base_type::is_open = true;
		return this;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::
	open(const std::string& host, const std::string& service,
					std::ios_base::openmode m)
	{
		if (is_open() != false) return 0;
		return open(socket_traits_type::open(host, service), m);
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::shutdown(std::ios_base::openmode m)
	{
		basic_socketbuf* result((this));

		if (is_open() == false) return 0;
		if (sync() == -1) result = 0;
		if (socket_traits_type::shutdown(this->__socketbuf_base_type::
							socket, m) != 0)
			result = 0;
		return result;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::close()
	{
		using std::swap;
		basic_socketbuf* result((this));
		socket_type invalid((socket_traits_type::invalid()));

		if (is_open() == false) return 0;
		if (sync() == -1) result = 0;
		if (socket_traits_type::close(this->__socketbuf_base_type::
								socket) != 0)
			result = 0;
		swap(this->__socketbuf_base_type::socket, invalid);	
		this->setg(0, 0, 0);
		this->setp(0, 0);
		this->__socketbuf_base_type::is_open = false;
		return result;
	}

	template <class SocketTraits>
	typename basic_socketbuf<SocketTraits>::socket_type
	basic_socketbuf<SocketTraits>::
	socket()
	{
		return this->__socketbuf_base_type::socket;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::
	setbuf(char_type* s, std::streamsize n)
	{
		if (s != 0) {
			if (n < 1) return 0;
			this->reset_base(s, false);
		} else {
			if (n <= 1) {
				this->reset_base(&this->buf[0], false);
				n = 1;
			} else {
				this->reset_base(new char_type[static_cast<
						std::size_t>(n)], true);
			}
		}

		std::ldiv_t d((std::ldiv(static_cast<long int>(n),
					static_cast<long int>(2))));
		this->gasize = d.quot + d.rem;
		this->pasize = d.quot;
		return this;
	}

	template <class SocketTraits>
	int
	basic_socketbuf<SocketTraits>::
	sync()
	{
		int_type eof((traits_type::eof()));
		int result(0);
		
		if (this->pptr() != 0)
			result = (overflow(eof) != eof) ? 0 : -1;
		return result;
	}

	template <class SocketTraits>
	std::streamsize
	basic_socketbuf<SocketTraits>::
	xsgetn(char_type* s, std::streamsize n)
	{
		
		std::streamsize result(0), avail;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::in) == 0) return result;
		if (this->gptr() == 0) init_io();
		avail = this->egptr() - this->gptr();
		if (avail >= n) {
			std::copy(this->gptr(), this->gptr() + n, s); 
			this->gbump(static_cast<std::size_t>(n));
			result = n;
		} else {
			s = std::copy(this->gptr(), this->gptr() + avail, s);
			this->gbump(static_cast<std::size_t>(avail));
			result = avail + read(s, n - avail);
		}
		return result;
	}

	template <class SocketTraits>
	typename basic_socketbuf<SocketTraits>::int_type
	basic_socketbuf<SocketTraits>::
	underflow()
	{
		int_type result((traits_type::eof()));
		std::streamsize got;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::in) == 0) return result;
		if (this->gptr() == 0) init_io();
		got = read(this->eback(), this->gasize);
		if (got > 0) {
			this->setg(this->eback(), this->eback(), 
						this->eback() + got);
			result = *this->gptr();
		}
		return result;
	}

	template <class SocketTraits>
	std::streamsize
	basic_socketbuf<SocketTraits>::
	xsputn(const char_type* s, std::streamsize n)
	{
		int_type eof((traits_type::eof()));
		std::streamsize result(0), pending, put;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::out) == 0) return result;
		if (this->pptr() == 0) init_io();
		pending = this->pptr() - this->pbase();
		if (pending + n <= this->pasize) {
			std::copy(s, s + n, this->pptr());
			this->pbump(static_cast<std::size_t>(n));
			result += n;	
		} else {
			if (overflow(eof) != 0) return result;
			std::ldiv_t d((std::div(static_cast<long int>(n),
					static_cast<long int>(this->pasize))));
			if (d.quot > 0) {
				d.quot *= static_cast<long int>(this->pasize);
				put = write(s, d.quot);
				if (put < d.quot) return put;
				s += put;
				result += put;
			}
			std::copy(s, s + d.rem, this->pbase());
			this->pbump(static_cast<std::size_t>(d.rem));
			result += d.rem;
		}
		return result;
	}

	template <class SocketTraits>
	typename basic_socketbuf<SocketTraits>::int_type
	basic_socketbuf<SocketTraits>::
	overflow(int_type c)
	{
		int_type result((traits_type::eof()));
		std::streamsize put, pending;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::out) == 0) return result;
		if (this->pptr() == 0) init_io();
		if (this->pptr() < this->epptr() && c != result)
			return this->sputc(traits_type::to_char_type(c));
		if (this->pbase() == this->epptr()) {
			if (c == result) {
				result = traits_type::not_eof(c);
			} else {
				char_type tmp((traits_type::to_char_type(c)));
				if (write(&tmp, 1) == 1)
					result = traits_type::not_eof(c);
			}
		} else {
			pending = this->pptr() - this->pbase();
			if (pending == 0) {
				put = 0;
			} else {
				put = write(this->pbase(), pending);
				this->pbump(static_cast<std::size_t>(
							-pending));
			}
			if (put == pending) {
				if (c == result)
					result = traits_type::not_eof(c);
				else 
					result = this->sputc(traits_type::
							to_char_type(c));
			}
		}
		return result;
	}

	template <class SocketTraits>
	void
	basic_socketbuf<SocketTraits>::
	init_io()
	{
		char_type *gbase, *pbase;

		if (this->base == 0)
			this->setbuf(0, BUFSIZ);
		gbase = this->base;
		pbase = gbase + this->gasize;
		if ((this->mode & std::ios_base::in) != 0)
			this->setg(gbase, gbase, gbase);
		if ((this->mode & std::ios_base::out) != 0)
			this->setp(pbase, pbase + this->pasize);
	}

	template <class SocketTraits>
	std::streamsize
	basic_socketbuf<SocketTraits>::
	read(char_type* s, std::streamsize n)
	{
		std::streamsize got, result(0);

		got = socket_traits_type::read(this->__socketbuf_base_type::
								socket, s, n);
		if (got > 0) result = got;
		return result;
	}

	template <class SocketTraits>
	std::streamsize
	basic_socketbuf<SocketTraits>::
	write(const char_type* s, std::streamsize n)
	{
		std::streamsize put, result(0);

		while (result < n) {
			put = socket_traits_type::write(
						this->__socketbuf_base_type::
							socket, s, n - result);
			if (put < 0) break;
			s += put;
			result += put;
		}
		return result;
	}

	template <class SocketTraits>
	basic_socketbuf_base<SocketTraits>::
	basic_socketbuf_base() :
	socket(SocketTraits::invalid()),
	buf(),
	base(0),
	gasize(0),
	pasize(0),
	mode(),
	is_open(false),
	auto_delete_base(false)
	{
	}
	
	template <class SocketTraits>
	basic_socketbuf_base<SocketTraits>::
	~basic_socketbuf_base()
	{
		reset_base(0, false);
	}

#if __cplusplus >= 201103L
	template <class SocketTraits>
	void
	basic_socketbuf_base<SocketTraits>::
	swap(basic_socketbuf_base& rhs)
	{
		using std::swap;
		swap(socket, rhs.socket);
		swap(buf, rhs.buf);
		swap(base, rhs.base);
		swap(gasize, rhs.gasize);
		swap(pasize, rhs.pasize);
		swap(mode, rhs.mode);
		swap(is_open, rhs.is_open);
		swap(auto_delete_base, rhs.auto_delete_base);
	}
#endif

	template <class SocketTraits>
	void
	basic_socketbuf_base<SocketTraits>::
	release_base()
	{
		base = 0;
		auto_delete_base = false;
	}

	template <class SocketTraits>
	void
	basic_socketbuf_base<SocketTraits>::
	reset_base(char* p, bool auto_delete)
	{
		if (base != 0 && auto_delete_base == true) {
			delete[] base;
		}
		base = p;
		auto_delete_base = auto_delete;
	}

}
