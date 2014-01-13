#include "computer_info.h"

#ifdef CINDER_MSW
#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>

# pragma comment(lib, "wbemuuid.lib")
#endif

namespace {
const double BYTE_2_MEGABYTE = 1.0 / (1024.0 * 1024.0);
const double BYTE_2_GIGABYTE = 9.31323e-10;
const double BYTE_2_KILOBYTE = 0.000976562;

// Utilities to help with COM stuff

// COM-INIT
class ComInit {
public:
	ComInit() : mInited(false), mOk(false) {
		HRESULT hres =  CoInitializeEx(0, COINIT_APARTMENTTHREADED); 
		if (FAILED(hres)) {
			// In this case, I'm already initialized
			if (hres == S_FALSE) {
				mOk = true;
			}
		} else {
			mInited = true;
			mOk = true;
		}
	}
	~ComInit() {
		if (mInited) CoUninitialize();
	}
	bool			ok() const { return mOk; }

private:
	bool			mInited;
	bool			mOk;
};

// WEBM-LOCATOR
class WebmLocator {
public:
	WebmLocator() : mPLoc(NULL) {
		HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0,  CLSCTX_INPROC_SERVER,  IID_IWbemLocator, (LPVOID *) &mPLoc);
		if (FAILED(hres)) mPLoc = NULL;
	}
	~WebmLocator() {
		if (mPLoc) mPLoc->Release();
	}

	IWbemLocator*	mPLoc;
};

// WEBM-SERVICES
class WebmServices {
public:
	WebmServices(WebmLocator& loc) : mPSvc(NULL) {
		// Connect to the root\cimv2 namespace with
		// the current user and obtain pointer pSvc
		// to make IWbemServices calls.
		HRESULT hres = loc.mPLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
												NULL,                    // User name. NULL = current user
												NULL,                    // User password. NULL = current
												0,                       // Locale. NULL indicates current
												NULL,                    // Security flags.
												0,                       // Authority (for example, Kerberos)
												0,                       // Context object 
												&mPSvc);                 // pointer to IWbemServices proxy
		if (FAILED(hres)) mPSvc = NULL;
	}
	~WebmServices() {
		if (mPSvc) mPSvc->Release();
	}

	IWbemServices*	mPSvc;
};

}

namespace ds {

#ifdef CINDER_MSW

ComputerInfo::ComputerInfo(const MemoryConversion memoryConversion, const int on)
	: mMemoryConversion(memoryConversion)
	, mOn(on)
	, mTotalVideoMemory(0.0)
{
	if (mMemoryConversion == MEGABYTE) mConversionNumber = BYTE_2_MEGABYTE;
	else if (mMemoryConversion == GIGABYTE) mConversionNumber = BYTE_2_GIGABYTE;
	else mConversionNumber = BYTE_2_KILOBYTE;

	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;

	GetSystemInfo(&sysInfo);
	mNumProcessors = sysInfo.dwNumberOfProcessors;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&mLastCPU, &ftime, sizeof(FILETIME));

	mProcessSelf = GetCurrentProcess();
	GetProcessTimes(mProcessSelf, &ftime, &ftime, &fsys, &fuser);
	memcpy(&mLastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&mLastUserCPU, &fuser, sizeof(FILETIME));

	update();
}

void ComputerInfo::update()
{
	mMemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&mMemoryStatus);
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&mProcessMemoryCounters, sizeof(PROCESS_MEMORY_COUNTERS_EX));

	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));
  
	GetProcessTimes(mProcessSelf, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = static_cast<double>((sys.QuadPart - mLastSysCPU.QuadPart) + (user.QuadPart - mLastUserCPU.QuadPart));
	percent /= (now.QuadPart - mLastCPU.QuadPart);
	percent /= mNumProcessors;
	mLastCPU = now;
	mLastUserCPU = user;
	mLastSysCPU = sys;

	mPercentCPU = percent * 100.0;

	if ((mOn&MAIN_ON) != 0) updateMain();
	if ((mOn&VIDEO_ON) != 0) updateVideo();
}

double ComputerInfo::getTotalVirtualMemory() const
{
  return mMemoryStatus.ullTotalPageFile * mConversionNumber;
}

double ComputerInfo::getCurrentVirtualMemory() const
{
	return (mMemoryStatus.ullTotalPageFile - mMemoryStatus.ullAvailPageFile) * mConversionNumber;
}

double ComputerInfo::getVirtualMemoryUsedByProcess() const
{
  return mProcessMemoryCounters.PrivateUsage * mConversionNumber;
}

double ComputerInfo::getTotalPhysicalMemory() const
{
	return mMemoryStatus.ullTotalPhys * mConversionNumber;
}

double ComputerInfo::getCurrentPhysicalMemory() const
{
	return (mMemoryStatus.ullTotalPhys - mMemoryStatus.ullAvailPhys) * mConversionNumber;
}

double ComputerInfo::getPhysicalMemoryUsedByProcess() const
{
	return mProcessMemoryCounters.WorkingSetSize * mConversionNumber;
}

ComputerInfo::MemoryConversion ComputerInfo::getConversion() const
{
	return mMemoryConversion;
}

double ComputerInfo::getConversionNumber() const
{
	return mConversionNumber;
}

double ComputerInfo::getPercentUsageCPU() const
{
	return mPercentCPU;
}

double ComputerInfo::getTotalVideoMemory() const
{
	return mTotalVideoMemory * mConversionNumber;
}

void ComputerInfo::updateMain()
{
	mMemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&mMemoryStatus);
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&mProcessMemoryCounters, sizeof(PROCESS_MEMORY_COUNTERS_EX));

	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));
  
	GetProcessTimes(mProcessSelf, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = static_cast<double>((sys.QuadPart - mLastSysCPU.QuadPart) + (user.QuadPart - mLastUserCPU.QuadPart));
	percent /= (now.QuadPart - mLastCPU.QuadPart);
	percent /= mNumProcessors;
	mLastCPU = now;
	mLastUserCPU = user;
	mLastSysCPU = sys;

	mPercentCPU = percent * 100.0;
}

void ComputerInfo::updateVideo()
{
	// Query the video controller class:
	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa394512(v=vs.85).aspx
	// This code taken from the example here:
	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa390423(v=vs.85).aspx

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------
	ComInit		ci;
	if (ci.ok()) {
#if 0
    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

                      
    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x" 
            << hex << hres << endl;
        CoUninitialize();
//        return 1;                    // Program has failed.
		return;
    }
    
#endif

	    // Step 3: ---------------------------------------------------
	    // Obtain the initial locator to WMI -------------------------
		WebmLocator			loc;
		if (loc.mPLoc) {
			// Step 4: -----------------------------------------------------
			// Connect to WMI through the IWbemLocator::ConnectServer method
			WebmServices	svc(loc);
			if (svc.mPSvc) {
				// Step 5: --------------------------------------------------
				// Set security levels on the proxy -------------------------
				HRESULT hres = CoSetProxyBlanket(
								svc.mPSvc,                   // Indicates the proxy to set
								RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
								RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
								NULL,                        // Server principal name 
								RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
								RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
								NULL,                        // client identity
								EOAC_NONE);                  // proxy capabilities 

				if (!FAILED(hres)) {
					// Step 6: --------------------------------------------------
					// Use the IWbemServices pointer to make requests of WMI ----

					IEnumWbemClassObject* pEnumerator = NULL;
					hres = svc.mPSvc->ExecQuery( bstr_t("WQL"), 
//												bstr_t("SELECT * FROM Win32_VideoController"),
												bstr_t("SELECT AdapterRAM FROM Win32_VideoController"),
												WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
												NULL,
												&pEnumerator);
    
					if (FAILED(hres)) return;

					// Step 7: -------------------------------------------------
					// Get the data from the query in step 6 -------------------
 
					ULONG uReturn = 0;
					while (pEnumerator) {
						IWbemClassObject *pclsObj;
						HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
						if (0 == uReturn) break;

						VARIANT vtProp;
						hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
						if (hr == S_OK) mTotalVideoMemory = vtProp.uintVal;
						VariantClear(&vtProp);

						pclsObj->Release();
					}
				    pEnumerator->Release();
				}
			}
		}
	}
}

#else // something else

ComputerInfo::ComputerInfo(const MemoryConversion &memoryConversion)
{
}

ComputerInfo::~ComputerInfo()
{

}

void ComputerInfo::update()
{
}

double ComputerInfo::getTotalVirtualMemory() const
{
  return 0.0f;
}

double ComputerInfo::getCurrentVirtualMemory() const
{
  return 0.0f;
}

double ComputerInfo::getVirtualMemoryUsedByProcess() const
{
  return 0.0f;
}

double ComputerInfo::getTotalPhysicalMemory() const
{
  return 0.0f;
}

double ComputerInfo::getCurrentPhysicalMemory() const
{
  return 0.0f;
}

double ComputerInfo::getPhysicalMemoryUsedByProcess() const
{
  return 0.0f;
}

ComputerInfo::MemoryConversion ComputerInfo::getConversion() const
{
  return NONE;
}

double ComputerInfo::getConversionNumber() const
{
  return 0.0f;
}

double ComputerInfo::getPercentUsageCPU() const
{
  return 0.0f;
}

#endif

} // namespace ds
