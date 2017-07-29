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
#include <cstdint>
#include <atomic>

namespace swoope {

	struct winsock {
		typedef std::pair<std::uint8_t, std::uint8_t> version_type;

		static version_type default_version()
		{
			return version_type(2, 2);
		}

		static int init(const version_type& ver = default_version())
		{
			WSADATA wsd{};
			return ::WSAStartup(MAKEWORD(ver.first, ver.second),
							&wsd) == 0 ? 0 : -1;
		}

		static int term()
		{
			return ::WSACleanup() == 0 ? 0 : -1;	
		}
	};

	class winsock_auto_init {
	public:
		winsock_auto_init()
		{
			if (++refcounter() == 1) {
				winsock::init();
			}
		}

		~winsock_auto_init()
		{
			if (--refcounter() == 0) {
				winsock::term();
			}
		}
	private:
		static std::atomic_long& refcounter()
		{
			static std::atomic_long rc;
			return rc;
		}
	};

#ifndef SWOOPE_MANUAL_WINSOCK_INIT
	static const winsock_auto_init winsock_auto_init_instance{};
#endif

}

#endif
