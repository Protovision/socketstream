/*
 * socketbuf.icc
 * Author: Mark Swoope
 * Date: Jul 2017
 */

inline socketbuf::socketbuf() :
	std::streambuf(),
	socket_(socket_traits::invalid()),
	buf_(nullptr),
	gasize_(0),
	pasize_(0),
	userbuf_(false)
{}

inline socketbuf::socketbuf(socketbuf&& other) :
	std::streambuf(),
	socket_(socket_traits::invalid()),
	buf_(nullptr),
	gasize_(0),
	pasize_(0),
	userbuf_(false)
{
	swap(other);
}

inline void socketbuf::create_buffer()
{
	if (buf_ == nullptr) {
		setbuf(nullptr, BUFSIZ);
	} else {
		setg(buf_, buf_, buf_);
		setp(buf_ + gasize_, buf_ + gasize_ + pasize_);
	}
}


inline void socketbuf::destroy_buffer()
{
	if (buf_ != nullptr && !userbuf_)
		delete[] buf_;
	buf_ = nullptr;
	gasize_ = 0;
	pasize_ = 0;
	userbuf_ = false;
	setg(nullptr, nullptr, nullptr);
	setp(nullptr, nullptr);
}

inline socketbuf& socketbuf::operator=(socketbuf&& other)
{
	close();
	destroy_buffer();
	swap(other);
	return *this;
}

inline void socketbuf::swap(socketbuf& other)
{
	std::streambuf::swap(other);
	std::swap(socket_, other.socket_);
	std::swap(buf_, other.buf_);
	std::swap(gasize_, other.gasize_);
	std::swap(pasize_, other.pasize_);
	std::swap(userbuf_, other.userbuf_);
}

inline socketbuf::~socketbuf()
{
	close();
	destroy_buffer();
}

inline socketbuf* socketbuf::setbuf(char_type* s, std::streamsize n)
{
	if (n == 0) n = 1;
	if (s != nullptr) {
		buf_ = s;
		userbuf_ = true;
	} else {
		buf_ = new char_type[n];
		userbuf_ = false;
	}
	auto d = std::div(n, std::streamsize(2));
	gasize_ = d.quot + d.rem;
	pasize_ = d.quot;
	setg(buf_, buf_, buf_);
	setp(buf_ + gasize_, buf_ + n);
	return this;
}

inline int socketbuf::sync()
{
	int_type eof{traits_type::eof()};

	return (overflow(eof) == eof) ? -1 : 0;
}

inline socketbuf::int_type socketbuf::overflow(int_type c)
{
	int_type result{traits_type::eof()};
	std::streamsize put, pending;

	if (!valid()) return result;
	if (pptr() == nullptr) create_buffer();
	if (pptr() < epptr() && c != result) return sputc(c);
	if (pbase() == epptr()) {
		if (c == result) {
			result = traits_type::not_eof(c);
		} else {
			char_type tmp{traits_type::to_char_type(c)};
			if (socket_traits::write(socket_, &tmp, 1) == 1)
				result = traits_type::not_eof(c);
		}
	} else {
		pending = pptr() - pbase();
		if (pending == 0) {
			put = 0;
		} else {
			put = socket_traits::write(socket_, pbase(), pending);
			pbump(-pending);
		}
		if (put == pending) {
			if (c == result) result = traits_type::not_eof(c);
			else result = sputc(c);
		}
	}
	return result;
}

inline socketbuf::int_type socketbuf::underflow()
{
	int_type result{traits_type::eof()};
	std::streamsize got;

	if (!valid()) return result;
	if (gptr() == nullptr) create_buffer();
	if (gptr() < egptr()) return *gptr();
	got = socket_traits::read(socket_, eback(), gasize_);
	if (got > 0) {
		setg(eback(), eback(), eback() + got);
		result = *gptr();
	}
	return result;
}

inline std::streamsize socketbuf::xsputn(const char_type* s, std::streamsize n)
{
	std::streamsize result{-1}, pending;

	if (!valid()) return -1;
	if (pptr() == nullptr) create_buffer();
	if (pptr() + n < epptr()) {
		std::copy_n(s, n, pptr());
		pbump(n);
		result = n;
	} else {
		sync();
		result = socket_traits::write(socket_, s, n);
		if (result < 0) result = 0;
	}
	return result;
}

inline std::streamsize socketbuf::xsgetn(char_type* s, std::streamsize n)
{
	std::streamsize avail, got, result;

	if (!valid()) return -1;
	if (gptr() == nullptr) create_buffer();
	avail = egptr() - gptr();
	if (avail >= n) {
		std::copy_n(gptr(), n, s);
		gbump(n);
		result = n;
	} else {
		s = std::copy_n(gptr(), avail, s);
		gbump(avail);
		got = socket_traits::read(socket_, s, n - avail);
		if (got < 0) got = 0;
		result = avail + got;
	}
	return result;
}

inline socketbuf* socketbuf::close()
{
	socketbuf* result{this};

	if (!valid()) return nullptr;
	if (sync() == -1) result = nullptr;
	if (socket_traits::close(socket_) == -1) result = nullptr;
	socket_ = socket_traits::invalid();
	setg(nullptr, nullptr, nullptr);
	setp(nullptr, nullptr);
	return result;
}

inline socketbuf* socketbuf::socket(socket_type socket)
{
	if (valid()) return nullptr;
	if (socket == socket_traits::invalid()) return nullptr;
	socket_ = socket;
	return this;
}
