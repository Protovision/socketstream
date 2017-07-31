#ifndef SWOOPE_WINSOCK_INIT_HH
#define SWOOPE_WINSOCK_INIT_HH

/*
 * winsock_init.hh
 * Author: Mark Swoope
 * Date: July 2017
 */

#include <Winsock2.h>
#include <Windows.h>

#include <utility>

namespace swoope {

	struct winsock {
		typedef BYTE byte_type;
		typedef std::pair<byte_type, byte_type> version_type;

		static version_type default_version()
		{
			return version_type(static_cast<byte_type>(2),
						static_cast<byte_type>(2));
		}

		static int init(const version_type& ver = default_version())
		{
			WSADATA wsd = WSADATA();
			return ::WSAStartup(MAKEWORD(ver.first, ver.second),
							&wsd) == 0 ? 0 : -1;
		}

		static int term()
		{
			return ::WSACleanup() == 0 ? 0 : -1;
		}
	};

	/*
	 * It's ok to call WSAStartup multiple times as long as an equal 
	 * number of WSACleanup calls are made. They are also thread-safe.
	 *
	 * Using thread-safe reference counting so that WSAStartup and
	 * WSACleanup only get called once can yield performance benefits,
	 * but implementing this requires using either the C++11 std::atomic 
	 * class or the Interlocked Windows functions on a 32-bit aligned
	 * integer.
	 *
	 * Using std::atomic breaks compatibility with C++03. Using alignas 
	 * to fulfill the alignment requirement for the Interlocked operations
	 * also breaks compatibility with C++03. Non-standard alignment
	 * specifiers exist but they vary across compilers. 
	 *
	 * Windows has a function called _align_malloc, but it is not 
	 * guaranteed to zero out the memory it allocates and not all 
	 * compilers on Windows have their header files updated to provide
	 * this function.
	 *
	 * The Boost libraries provide a type_with_alignment class but 
	 * I do not want this library to have any dependencies.
	 */

	class winsock_auto_init {
	public:
		winsock_auto_init()
		{
				winsock::init();
		}

		~winsock_auto_init()
		{
				winsock::term();
		}
	};

#ifndef SWOOPE_MANUAL_WINSOCK_INIT
	static const winsock_auto_init winsock_auto_init_instance;
#endif

}

#endif
