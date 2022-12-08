#pragma once
#ifndef DS_DEBUG_COMPUTER_INFO_H
#define DS_DEBUG_COMPUTER_INFO_H

#include <cinder/Cinder.h>

#ifdef CINDER_MSW

// #include <WinSock2.h>
#include <pdh.h>
#include <psapi.h>
#include <string>
#include <tchar.h>
#include <windows.h>

namespace ds {

/**
 * \class ComputerInfo
 * \brief Provide system metrics.
 */
class ComputerInfo {
  public:
	enum MemoryConversion { MEGABYTE, GIGABYTE, KILOBYTE, NONE };
	/// Determine which metrics are active.
	static const int MAIN_ON  = (1 << 0);
	static const int VIDEO_ON = (1 << 1);
	static const int ALL_ON	  = 0xffffffff;

  public:
	ComputerInfo(const MemoryConversion = MEGABYTE, const int on = MAIN_ON);

	MemoryConversion getConversion() const;
	double			 getConversionNumber() const;

	void update();

	/// MAIN
	double getTotalVirtualMemory() const;
	double getCurrentVirtualMemory() const;
	double getVirtualMemoryUsedByProcess() const;
	double getTotalPhysicalMemory() const;
	double getCurrentPhysicalMemory() const;
	double getPhysicalMemoryUsedByProcess() const;
	double getPercentUsageCPU() const;
	int	   getNumberOfProcessors() const;
	/// VIDEO
	double		getTotalVideoMemory() const;
	std::string getVideoDriverVersion() const;
	std::string getVideoDriverVendor() const;
	std::string getVideoCardName() const;
	int			getVideoRefreshRate() const;

  private:
	void updateMain();
	void updateVideo();

	MEMORYSTATUSEX			   mMemoryStatus;
	PROCESS_MEMORY_COUNTERS_EX mProcessMemoryCounters;
	double					   mConversionNumber;
	MemoryConversion		   mMemoryConversion;
	int						   mOn;

	int	   mNumProcessors;
	HANDLE mProcessSelf;
	/// MAIN
	ULARGE_INTEGER mLastCPU;
	ULARGE_INTEGER mLastSysCPU;
	ULARGE_INTEGER mLastUserCPU;
	double		   mPercentCPU;
	/// VIDEO
	double		mTotalVideoMemory;
	std::string mVideoDriverVersion; // Long driver number, such as 21.21.13.6909
	std::string mVideoVendor;		 // Such as "NVIDIA"
	std::string mVideoCardName;		 // Such as "NVIDIA Quadro K5000"
	int			mRefreshRate;
};

} // namespace ds

#else // Something else

namespace ds {

class ComputerInfo {
  public:
	enum MemoryConversion { MEGABYTE, GIGABYTE, KILOBYTE, NONE };

	ComputerInfo(const MemoryConversion& memoryConversion = MEGABYTE);
	~ComputerInfo();
	MemoryConversion getConversion() const;
	double			 getConversionNumber() const;

	void   update();
	double getTotalVirtualMemory() const;
	double getCurrentVirtualMemory() const;
	double getVirtualMemoryUsedByProcess() const;
	double getTotalPhysicalMemory() const;
	double getCurrentPhysicalMemory() const;
	double getPhysicalMemoryUsedByProcess() const;

	double getPercentUsageCPU() const;

  private:
};

} // namespace ds

#endif

#endif // DS_DEBUG_COMPUTER_INFO_H
