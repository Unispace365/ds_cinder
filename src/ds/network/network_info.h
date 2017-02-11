#pragma once

#ifndef _DS_NETWORK_NETWORK_INFO_H_
#define _DS_NETWORK_NETWORK_INFO_H_

#include <string>

namespace ds {
	namespace network {
	class networkInfo {

	public:
		networkInfo();

		void				showInfo() const;
		std::string			getAddress() const;
	private:
		std::string			mAddress;
	};
}

} // !namespace ds

#endif // !_DS_NETWORK_NETWORK_INFO_H_
