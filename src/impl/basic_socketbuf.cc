/*
 * socketbuf.cc
 * Author: Mark Swoope
 * Date: Jul 2017
 */
#include <iostream>
namespace swoope {

	template <class SocketTraits>
	inline basic_socketbuf<SocketTraits>::basic_socketbuf() :
		std::streambuf(),
		socketbuf_base()
	{
	}

	template <class SocketTraits>
	inline
	basic_socketbuf<SocketTraits>::basic_socketbuf(basic_socketbuf&& rhs) :
		std::streambuf(),
		socketbuf_base()
	{
		swap(rhs);
	}

	template <class SocketTraits>
	inline
	basic_socketbuf<SocketTraits>::~basic_socketbuf()
	{
		try {
			close(); 
		} catch(...) {}
	}

	template <class SocketTraits>
	inline basic_socketbuf<SocketTraits>&
	basic_socketbuf<SocketTraits>::operator=(basic_socketbuf&& rhs)
	{
		basic_socketbuf temp{*this};

		temp.close();
		temp.base_.reset();
		temp.swap(rhs);
		swap(temp);
		return *this;
	}

	template <class SocketTraits>
	inline void 
	basic_socketbuf<SocketTraits>::swap(basic_socketbuf& rhs)
	{
		std::streambuf::swap(rhs);
		socketbuf_base::swap(rhs);
	}

	template <class SocketTraits>
	inline bool
	basic_socketbuf<SocketTraits>::is_open() const
	{
		return (socket_ != socket_traits::invalid());
	}

	template <class SocketTraits>
	inline basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::open(socket_type socket,
						std::ios_base::openmode mode)
	{
		if (is_open() != false) return nullptr;
		if (socket_ == socket_traits::invalid()) return nullptr;
		socket_ = socket;
		mode_ = mode;
		return this;
	}

	template <class SocketTraits>
	inline basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::open(const std::string& host,
						const std::string& service,
						std::ios_base::openmode mode)
	{
		socket_type client{};

		if (is_open() != false) return nullptr;
		client = socket_traits::open(host, service);
		if (client == socket_traits::invalid())
			return nullptr;
		socket_ = client;
		mode_ = mode;
		return this;
	}

	template <class SocketTraits>
	inline basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::shutdown(std::ios_base::openmode m)
	{
		typedef typename socket_traits::shutdown_mode shutdown_mode;

		basic_socketbuf* result{this};
		static const std::unordered_map<int, shutdown_mode>
		modes{
			{ static_cast<int>(std::ios_base::in |
						std::ios_base::out),
						shutdown_mode::rdwr },
			{ static_cast<int>(std::ios_base::out),
						shutdown_mode::write },
			{ static_cast<int>(std::ios_base::in),
						shutdown_mode::read }
		};
		typename std::unordered_map<int,
				shutdown_mode>::const_iterator found;

		if (is_open() == false) return nullptr;
		found = modes.find(static_cast<int>(m));
		if (found == modes.end()) return nullptr;
		if (sync() == -1) result = nullptr;
		if (socket_traits::shutdown(socket_, found->second) != 0)
			result = nullptr;
		return result;
	}

	template <class SocketTraits>
	inline basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::close()
	{
		basic_socketbuf* result{this};

		if (is_open() == false) return nullptr;
		if (sync() == -1) result = nullptr;
		if (socket_traits::close(socket_) == -1) 
			result = nullptr;
		socket_ = socket_traits::invalid();
		setg(nullptr, nullptr, nullptr);
		setp(nullptr, nullptr);
		return result;
	}

	template <class SocketTraits>
	inline typename basic_socketbuf<SocketTraits>::socket_type
	basic_socketbuf<SocketTraits>::socket()
	{
		return socket_;
	}

	template <class SocketTraits>
	inline basic_socketbuf<SocketTraits>*
	basic_socketbuf<SocketTraits>::setbuf(char_type* s, std::streamsize n)
	{
		auto nop_deleter = [](char_type*){};

		if (s != nullptr) {
			if (n < 1) return nullptr;
			base_.reset(s, nop_deleter);
		} else {
			if (n <= 1) {
				base_.reset(std::addressof(buf_[0]),
							nop_deleter);
				n = 1;
			} else {
				base_.reset(
				new char_type[static_cast<std::size_t>(n)]);
			}
		}

		auto d = std::div(n, std::streamsize(2));
		gasize_ = d.quot + d.rem;
		pasize_ = d.quot;
		return this;
	}

	template <class SocketTraits>
	inline int
	basic_socketbuf<SocketTraits>::sync()
	{
		int_type eof{traits_type::eof()};
		int result{0};
		
		if (pptr() != nullptr)
			result = (overflow(eof) != eof) ? 0 : -1;
		return result;
	}

	template <class SocketTraits>
	inline std::streamsize
	basic_socketbuf<SocketTraits>::xsgetn(char_type* s, std::streamsize n)
	{
		
		std::streamsize result{0}, avail;

		if (is_open() == false) return result;
		if ((mode_ & std::ios_base::in) == 0) return result;
		if (gptr() == nullptr) init_io();
		avail = egptr() - gptr();
		if (avail >= n) {
			std::copy_n(gptr(), n, s);
			gbump(static_cast<std::size_t>(n));
			result = n;
		} else {
			s = std::copy_n(gptr(), avail, s);
			gbump(static_cast<std::size_t>(avail));
			result = avail + read(s, n - avail);
		}
		return result;
	}

	template <class SocketTraits>
	inline typename basic_socketbuf<SocketTraits>::int_type
	basic_socketbuf<SocketTraits>::underflow()
	{
		int_type result{traits_type::eof()};
		std::streamsize got;

		if (is_open() == false) return result;
		if ((mode_ & std::ios_base::in) == 0) return result;
		if (gptr() == nullptr) init_io();
		got = read(eback(), gasize_);
		if (got > 0) {
			setg(eback(), eback(), eback() + got);
			result = *gptr();
		}
		return result;
	}

	template <class SocketTraits>
	inline std::streamsize
	basic_socketbuf<SocketTraits>::xsputn(const char_type* s,
						std::streamsize n)
	{
		int_type eof{traits_type::eof()};
		std::streamsize result{0}, pending, put;

		if (is_open() == false) return result;
		if ((mode_ & std::ios_base::out) == 0) return result;
		if (pptr() == nullptr) init_io();
		pending = pptr() - pbase();
		if (pending + n <= pasize_) {
			std::copy_n(s, n, pptr());
			pbump(static_cast<std::size_t>(n));
			result += n;	
		} else {
			if (overflow(eof) != 0) return result;
			auto d = std::div(n, std::streamsize(pasize_));
			if (d.quot > 0) {
				d.quot *= pasize_;
				put = write(s, d.quot);
				if (put < d.quot) return put;
				s += put;
				result += put;
			}
			std::copy_n(s, d.rem, pbase());
			pbump(static_cast<std::size_t>(d.rem));
			result += d.rem;
		}
		return result;
	}

	template <class SocketTraits>
	inline typename basic_socketbuf<SocketTraits>::int_type
	basic_socketbuf<SocketTraits>::overflow(int_type c)
	{
		int_type result{traits_type::eof()};
		std::streamsize put, pending;

		if (is_open() == false) return result;
		if ((mode_ & std::ios_base::out) == 0) return result;
		if (pptr() == nullptr) init_io();
		if (pptr() < epptr() && c != result)
			return sputc(traits_type::to_char_type(c));
		if (pbase() == epptr()) {
			if (c == result) {
				result = traits_type::not_eof(c);
			} else {
				char_type tmp{traits_type::to_char_type(c)};
				if (write(std::addressof(tmp), 1) == 1)
					result = traits_type::not_eof(c);
			}
		} else {
			pending = pptr() - pbase();
			if (pending == 0) {
				put = 0;
			} else {
				put = write(pbase(), pending);
				pbump(static_cast<std::size_t>(-pending));
			}
			if (put == pending) {
				if (c == result)
					result = traits_type::not_eof(c);
				else 
					result = sputc(
						traits_type::to_char_type(c));
			}
		}
		return result;
	}

	template <class SocketTraits>
	basic_socketbuf<SocketTraits>::basic_socketbuf(
					const basic_socketbuf& rhs) :
		std::streambuf(rhs),
		socketbuf_base(rhs)
	{
	}

	template <class SocketTraits>
	inline void
	basic_socketbuf<SocketTraits>::init_io()
	{
		char_type *gbase, *pbase;

		if (base_.get() == nullptr)
			setbuf(nullptr, BUFSIZ);
		gbase = base_.get();
		pbase = gbase + gasize_;
		if ((mode_ & std::ios_base::in) != 0)
			setg(gbase, gbase, gbase);
		if ((mode_ & std::ios_base::out) != 0)
			setp(pbase, pbase + pasize_);
	}

	template <class SocketTraits>
	inline std::streamsize
	basic_socketbuf<SocketTraits>::read(char_type* s, std::streamsize n)
	{
		std::streamsize got, result{0};

		got = socket_traits::read(socket_, s, n);
		if (got > 0) result = got;
		return result;
	}

	template <class SocketTraits>
	inline std::streamsize
	basic_socketbuf<SocketTraits>::write(const char_type* s,
						std::streamsize n)
	{
		std::streamsize put, result{0};

		while (result < n) {
			put = socket_traits::write(socket_, s, n - result);
			if (put < 0) break;
			s += put;
			result += put;
		}
		return result;
	}

	template <class SocketTraits>
	inline void
	swap(basic_socketbuf<SocketTraits>& a,
		basic_socketbuf<SocketTraits>& b)
	{
		a.swap(b);
	}

	template <class SocketTraits>
	inline
	basic_socketbuf_base<SocketTraits>::basic_socketbuf_base() :
		socket_(SocketTraits::invalid()),
		buf_(),
		base_(),
		gasize_(0),
		pasize_(0),
		mode_()
	{
	}

	template <class SocketTraits>
	inline void
	basic_socketbuf_base<SocketTraits>::swap(basic_socketbuf_base& rhs)
	{
		using std::swap;
		swap(socket_, rhs.socket_);
		swap(buf_, rhs.buf_);
		swap(base_, rhs.base_);
		swap(gasize_, rhs.gasize_);
		swap(pasize_, rhs.pasize_);
		swap(mode_, rhs.mode_);
	}

	template <class SocketTraits>
	inline void
	swap(basic_socketbuf_base<SocketTraits>& a,
		basic_socketbuf_base<SocketTraits>& b)
	{
		a.swap(b);
	}

}
