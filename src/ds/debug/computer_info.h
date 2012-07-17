#pragma once
#ifndef DS_DEBUG_COMPUTER_INFO_H
#define DS_DEBUG_COMPUTER_INFO_H
#include "cinder/Cinder.h"

#ifdef CINDER_MSW

#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <pdh.h>

namespace ds {

class ComputerInfo
{
  public:
    enum MemoryConversion
    {
      MEGABYTE,
      GIGABYTE,
      KILOBYTE,
      NONE
    };

    ComputerInfo(const MemoryConversion &memoryConversion = MEGABYTE);
    ~ComputerInfo();
    MemoryConversion getConversion() const;
    double getConversionNumber() const;

    void update();
    double getTotalVirtualMemory() const;
    double getCurrentVirtualMemory() const;
    double getVirtualMemoryUsedByProcess() const;
    double getTotalPhysicalMemory() const;
    double getCurrentPhysicalMemory() const;
    double getPhysicalMemoryUsedByProcess() const;

    double getPercentUsageCPU() const;
  private:
    MEMORYSTATUSEX mMemoryStatus;
    PROCESS_MEMORY_COUNTERS_EX mProcessMemoryCounters;
    double mConversionNumber;
    MemoryConversion mMemoryConversion;

    ULARGE_INTEGER mLastCPU;
    ULARGE_INTEGER mLastSysCPU;
    ULARGE_INTEGER mLastUserCPU;
    int            mNumProcessors;
    HANDLE         mProcessSelf;
    double         mPercentCPU;
};

}

#else // Something else

namespace ds {

class ComputerInfo
{
public:
  enum MemoryConversion
  {
    MEGABYTE,
    GIGABYTE,
    KILOBYTE,
    NONE
  };

  ComputerInfo(const MemoryConversion &memoryConversion = MEGABYTE);
  ~ComputerInfo();
  MemoryConversion getConversion() const;
  double getConversionNumber() const;

  void update();
  double getTotalVirtualMemory() const;
  double getCurrentVirtualMemory() const;
  double getVirtualMemoryUsedByProcess() const;
  double getTotalPhysicalMemory() const;
  double getCurrentPhysicalMemory() const;
  double getPhysicalMemoryUsedByProcess() const;

  double getPercentUsageCPU() const;
private:
};

}

#endif

#endif//DS_DEBUG_COMPUTER_INFO_H
