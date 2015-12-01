#pragma once

#ifndef _SYNC_VIDEO_PLAYER_APP_NETWORK_INFO_H_
#define _SYNC_VIDEO_PLAYER_APP_NETWORK_INFO_H_

//#include <winsock2.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
//#include <stdlib.h>
#include <ds/debug/logger.h>

namespace ds {
	namespace network {
	class networkInfo {

	public:
		networkInfo();

		void				showInfo();
		std::string			getAddress();
	private:

		bool				checkAddress(PIP_ADAPTER_INFO iai);
		std::string			mAddress;
	};
}

} // !namespace ds

#endif // !_SYNC_VIDEO_PLAYER_APP_NETWORK_INFO_H_