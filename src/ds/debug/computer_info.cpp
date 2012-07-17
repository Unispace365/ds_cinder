#include "computer_info.h"

namespace {

const double BYTE_2_MEGABYTE = 9.31323e-7;
const double BYTE_2_GIGABYTE = 9.31323e-10;
const double BYTE_2_KILOBYTE = 0.000976562;
}

namespace ds {

ComputerInfo::ComputerInfo(const MemoryConversion &memoryConversion)
  : mMemoryConversion(memoryConversion)
{
  if (mMemoryConversion == MEGABYTE)
    mConversionNumber = BYTE_2_MEGABYTE;
  else if (mMemoryConversion == GIGABYTE)
    mConversionNumber = BYTE_2_GIGABYTE;
  else
    mConversionNumber = BYTE_2_KILOBYTE;

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

ComputerInfo::~ComputerInfo()
{

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
  percent = (sys.QuadPart - mLastSysCPU.QuadPart) +
    (user.QuadPart - mLastUserCPU.QuadPart);
  percent /= (now.QuadPart - mLastCPU.QuadPart);
  percent /= mNumProcessors;
  mLastCPU = now;
  mLastUserCPU = user;
  mLastSysCPU = sys;

  mPercentCPU = percent * 100.0;
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

} // namespace ds
