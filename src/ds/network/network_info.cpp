#include "network_info.h"

namespace ds {
	namespace network {
		networkInfo::networkInfo() : mAddress(""){
			IP_ADAPTER_INFO  *pAdapterInfo;
			ULONG            ulOutBufLen;
			DWORD            dwRetVal;

			pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
			ulOutBufLen = sizeof(IP_ADAPTER_INFO);


			if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) {
				free(pAdapterInfo);
				pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
			}

			if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS) {
				DS_LOG_WARNING("Gst warning: " "GetAdaptersInfo call failed with " << dwRetVal );
				
			}

			PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
			//Find first 'valid' address
			while (pAdapter) {
				if (checkAddress(pAdapter)) {
					if (mAddress.empty()) {
						mAddress = pAdapter->IpAddressList.IpAddress.String;
						break;
					}
				}
				pAdapter = pAdapter->Next;

			}
			if (pAdapterInfo)
				free(pAdapterInfo);
			}



		void networkInfo::showInfo() {
			IP_ADAPTER_INFO  *pAdapterInfo;
			ULONG            ulOutBufLen;
			DWORD            dwRetVal;

			pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
			ulOutBufLen = sizeof(IP_ADAPTER_INFO);


			if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) {
				free(pAdapterInfo);
				pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
			}

			if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS) {
				std::cout << "GetAdaptersInfo call failed with " << dwRetVal << std::endl;
			}

			PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
			while (pAdapter) {
				if (checkAddress(pAdapter)) {
					if (mAddress.empty())
						mAddress = pAdapter->IpAddressList.IpAddress.String;

					std::cout << "Adapter Name: " << pAdapter->AdapterName << std::endl;
					std::cout << "Adapter Desc: " << pAdapter->Description << std::endl;
					std::cout << "\tAdapter Addr: " << std::endl;
					for (UINT i = 0; i < pAdapter->AddressLength; i++) {

						if (i == (pAdapter->AddressLength - 1))
							std::cout << (int)pAdapter->Address[i];
						else
							std::cout << (int)pAdapter->Address[i];
					}
					std::cout << "DHCP  server: " << pAdapter->DhcpServer.IpAddress.String << std::endl;
					std::cout << "primary Wins: " << pAdapter->PrimaryWinsServer.IpAddress.String << std::endl;
					std::cout << "IP Address: " << pAdapter->IpAddressList.IpAddress.String << std::endl;
					std::cout << "IP Mask: " << pAdapter->IpAddressList.IpMask.String << std::endl;
					std::cout << "\tGateway: " << pAdapter->GatewayList.IpAddress.String << std::endl;
					std::cout << "	***" << std::endl;
					if (pAdapter->DhcpEnabled) {
						std::cout << "DHCP Enabled: Yes" << std::endl;;
						std::cout << "DHCP Server: " << pAdapter->DhcpServer.IpAddress.String << std::endl;
					}
					else
						std::cout << "DHCP Enabled: No" << std::endl;
				}

					pAdapter = pAdapter->Next;
			}
			if (pAdapterInfo)
				free(pAdapterInfo);

		}


		bool networkInfo::checkAddress(PIP_ADAPTER_INFO iai){
			std::string ip = iai->IpAddressList.IpAddress.String;
			// throw out 0.0.0.0 addresses.
			int x = 0;

			for (int i = 0; i < ip.length(); ++i) {
				char c = ip.c_str()[i];
				if (atoi(&c) >0)
					x++;
			}
			return x>0;
		}

		std::string networkInfo::getAddress() {
			return mAddress;
		}

	}
}