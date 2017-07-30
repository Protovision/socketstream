/*
 * socketbuf.cc
 * Author: Mark Swoope
 * Date: Jul 2017
 */
#include <iostream>
namespace swoope {

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
	basic_socketbuf() :
	__streambuf_type(),
	__socketbuf_base_type()
	{
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
	basic_socketbuf(basic_socketbuf&& rhs) :
	__streambuf_type(),		
	__socketbuf_base_type()
	{
		swap(rhs);
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::
	~basic_socketbuf()
	{
		try {
			close(); 
		} catch (...) {}
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>&
	basic_socketbuf<SocketTraits>::
	operator=(basic_socketbuf&& rhs)
	{
		basic_socketbuf temp{*this};

		temp.close();
		temp.base.reset();
		temp.swap(rhs);
		swap(temp);
		return *this;
	}

	template <class SocketTraits>
	void 
	basic_socketbuf<SocketTraits>::
	swap(basic_socketbuf& rhs)
	{
		__streambuf_type::swap(rhs);
		__socketbuf_base_type::swap(rhs);
	}

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
	open(socket_type&& socket, std::ios_base::openmode mode)
	{
		using std::swap;

		if (is_open() != false) return nullptr;
		if (socket == this->socket) return nullptr;
		swap(this->socket, socket);
		this->mode = mode;
		this->__socketbuf_base_type::is_open = true;
		return this;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::
	open(const std::string& host, const std::string& service,
					std::ios_base::openmode mode)
	{
		if (is_open() != false) return nullptr;
		return open(std::move(socket_traits_type::
					open(host, service)), mode);
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::shutdown(std::ios_base::openmode m)
	{
		typedef typename socket_traits_type::shutdown_mode
							shutdown_mode;

		basic_socketbuf* result{this};
		static const std::unordered_map<int, shutdown_mode> modes{
			{ static_cast<int>(std::ios_base::in | std::ios_base::
						out), shutdown_mode::rdwr },
			{ static_cast<int>(std::ios_base::out), 
					shutdown_mode::write },
			{ static_cast<int>(std::ios_base::in), shutdown_mode::
									read }
		};
		typename std::unordered_map<int, shutdown_mode>::const_iterator
									found;

		if (is_open() == false) return nullptr;
		found = modes.find(static_cast<int>(m));
		if (found == modes.end()) return nullptr;
		if (sync() == -1) result = nullptr;
		if (socket_traits_type::shutdown(this->socket,
						found->second) != 0)
			result = nullptr;
		return result;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::close()
	{
		using std::swap;
		basic_socketbuf* result{this};
		socket_type invalid{std::move(socket_traits_type::invalid())};

		if (is_open() == false) return nullptr;
		if (sync() == -1) result = nullptr;
		if (socket_traits_type::close(this->socket) == -1) 
			result = nullptr;
		swap(this->socket, invalid);	
		this->setg(nullptr, nullptr, nullptr);
		this->setp(nullptr, nullptr);
		this->__socketbuf_base_type::is_open = false;
		return result;
	}

	template <class SocketTraits>
	typename basic_socketbuf<SocketTraits>::socket_type*
	basic_socketbuf<SocketTraits>::
	rdsocket()
	{
		return std::addressof(this->socket);
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::
	setbuf(char_type* s, std::streamsize n)
	{
		auto nop_deleter = [](char_type*){};

		if (s != nullptr) {
			if (n < 1) return nullptr;
			this->base.reset(s, nop_deleter);
		} else {
			if (n <= 1) {
				this->base.reset(std::addressof(
						this->buf[0]), nop_deleter);
				n = 1;
			} else {
				this->base.reset(new char_type[static_cast<
							std::size_t>(n)]);
			}
		}

		auto d = std::div(n, std::streamsize(2));
		this->gasize = d.quot + d.rem;
		this->pasize = d.quot;
		return this;
	}

	template <class SocketTraits>
	int
	basic_socketbuf<SocketTraits>::
	sync()
	{
		int_type eof{traits_type::eof()};
		int result{0};
		
		if (this->pptr() != nullptr)
			result = (overflow(eof) != eof) ? 0 : -1;
		return result;
	}

	template <class SocketTraits>
	std::streamsize
	basic_socketbuf<SocketTraits>::
	xsgetn(char_type* s, std::streamsize n)
	{
		
		std::streamsize result{0}, avail;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::in) == 0) return result;
		if (this->gptr() == nullptr) init_io();
		avail = this->egptr() - this->gptr();
		if (avail >= n) {
			std::copy_n(this->gptr(), n, s);
			this->gbump(static_cast<std::size_t>(n));
			result = n;
		} else {
			s = std::copy_n(this->gptr(), avail, s);
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
		int_type result{traits_type::eof()};
		std::streamsize got;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::in) == 0) return result;
		if (this->gptr() == nullptr) init_io();
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
		int_type eof{traits_type::eof()};
		std::streamsize result{0}, pending, put;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::out) == 0) return result;
		if (this->pptr() == nullptr) init_io();
		pending = this->pptr() - this->pbase();
		if (pending + n <= this->pasize) {
			std::copy_n(s, n, this->pptr());
			this->pbump(static_cast<std::size_t>(n));
			result += n;	
		} else {
			if (overflow(eof) != 0) return result;
			auto d = std::div(n, std::streamsize(this->pasize));
			if (d.quot > 0) {
				d.quot *= this->pasize;
				put = write(s, d.quot);
				if (put < d.quot) return put;
				s += put;
				result += put;
			}
			std::copy_n(s, d.rem, this->pbase());
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
		int_type result{traits_type::eof()};
		std::streamsize put, pending;

		if (is_open() == false) return result;
		if ((this->mode & std::ios_base::out) == 0) return result;
		if (this->pptr() == nullptr) init_io();
		if (this->pptr() < this->epptr() && c != result)
			return this->sputc(traits_type::to_char_type(c));
		if (this->pbase() == this->epptr()) {
			if (c == result) {
				result = traits_type::not_eof(c);
			} else {
				char_type tmp{traits_type::to_char_type(c)};
				if (write(std::addressof(tmp), 1) == 1)
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
	basic_socketbuf<SocketTraits>::
	basic_socketbuf(const basic_socketbuf& rhs) :
	__streambuf_type(rhs),
	__socketbuf_base_type(rhs)
	{
	}

	template <class SocketTraits>
	void
	basic_socketbuf<SocketTraits>::
	init_io()
	{
		char_type *gbase, *pbase;

		if (this->base.get() == nullptr)
			this->setbuf(nullptr, BUFSIZ);
		gbase = this->base.get();
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
		std::streamsize got, result{0};

		got = socket_traits_type::read(this->socket, s, n);
		if (got > 0) result = got;
		return result;
	}

	template <class SocketTraits>
	std::streamsize
	basic_socketbuf<SocketTraits>::
	write(const char_type* s, std::streamsize n)
	{
		std::streamsize put, result{0};

		while (result < n) {
			put = socket_traits_type::write(this->socket, s,
							n - result);
			if (put < 0) break;
			s += put;
			result += put;
		}
		return result;
	}

	template <class SocketTraits>
	void
	swap(basic_socketbuf<SocketTraits>& a, basic_socketbuf<
						SocketTraits>& b)
	{
		a.swap(b);
	}

	template <class SocketTraits>
	basic_socketbuf_base<SocketTraits>::
	basic_socketbuf_base() :
	socket(std::move(SocketTraits::invalid())),
	buf(),
	base(nullptr),
	gasize(0),
	pasize(0),
	mode(),
	is_open(false)
	{
	}

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
	}

	template <class SocketTraits>
	void
	swap(basic_socketbuf_base<SocketTraits>& a, basic_socketbuf_base<
							SocketTraits>& b)
	{
		a.swap(b);
	}

}
