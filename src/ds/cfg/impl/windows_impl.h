#pragma once
#include "base_impl.h"

#ifdef _WIN32
#include <shellapi.h>
#include <tchar.h>
#include <winver.h>
#endif

namespace ds::cfg::impl {

struct WinComputerInfo : public BaseComputerInfo {

	virtual std::string getAppVersionString() override {
		std::string versionOut = "not found";

		HRSRC			  hResInfo;
		DWORD			  dwSize;
		HGLOBAL			  hResData;
		LPVOID			  pRes, pResCopy;
		UINT			  uLen	= 0;
		VS_FIXEDFILEINFO* lpFfi = NULL;
		HINSTANCE		  hInst = ::GetModuleHandle(NULL);

		hResInfo = FindResource(hInst, MAKEINTRESOURCE(1), RT_VERSION);
		if (!hResInfo) return versionOut;

		dwSize	 = SizeofResource(hInst, hResInfo);
		hResData = LoadResource(hInst, hResInfo);
		if (!hResData) return versionOut;

		pRes = LockResource(hResData);
		if (!pRes) return versionOut;

		pResCopy = LocalAlloc(LMEM_FIXED, dwSize);
		if (!pResCopy) return versionOut;


		CopyMemory(pResCopy, pRes, dwSize);

		if (VerQueryValue(pResCopy, _T("\\"), (LPVOID*)&lpFfi, &uLen)) {
			if (lpFfi != NULL) {
				DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
				DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;

				DWORD dwLeftMost	= HIWORD(dwFileVersionMS);
				DWORD dwSecondLeft	= LOWORD(dwFileVersionMS);
				DWORD dwSecondRight = HIWORD(dwFileVersionLS);
				DWORD dwRightMost	= LOWORD(dwFileVersionLS);

				versionOut = std::string() + std::to_string(dwLeftMost) + "." + std::to_string(dwSecondLeft) + "." +
							 std::to_string(dwSecondRight) + "." + std::to_string(dwRightMost);
			}
		}

		LocalFree(pResCopy);
		return versionOut;
	}

	virtual std::string getAppProductName() override {
		std::string versionOut = "DS App";

		HRSRC			  hResInfo;
		DWORD			  dwSize;
		HGLOBAL			  hResData;
		LPVOID			  pRes, pResCopy;
		UINT			  uLen	= 0;
		VS_FIXEDFILEINFO* lpFfi = NULL;
		HINSTANCE		  hInst = ::GetModuleHandle(NULL);

		hResInfo = FindResource(hInst, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
		if (!hResInfo) return versionOut;

		dwSize	 = SizeofResource(hInst, hResInfo);
		hResData = LoadResource(hInst, hResInfo);
		if (!hResData) return versionOut;

		pRes = LockResource(hResData);
		if (!pRes) return versionOut;

		pResCopy = LocalAlloc(LMEM_FIXED, dwSize);
		if (!pResCopy) return versionOut;


		CopyMemory(pResCopy, pRes, dwSize);
		if (VerQueryValueW(pResCopy, _T("\\StringFileInfo\\040904b0\\ProductName"), (LPVOID*)&lpFfi, &uLen)) {
			if (lpFfi != NULL) {
				versionOut = ds::utf8_from_wstr(std::wstring(((wchar_t*)(lpFfi)), uLen));
			}
		}

		LocalFree(pResCopy);
		return versionOut;
	}

	virtual std::string getOsVersion() override { return "Windows 10/11"; }

	virtual std::string getOpenGlVendor() override {
		const GLubyte* vendor = glGetString(GL_VENDOR);
		return std::string((const char*)(vendor), std::strlen((const char*)(vendor)));
	};

	virtual std::string getOpenglVersion() override {
		const GLubyte* version	   = glGetString(GL_VERSION);
		const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

		return std::string((const char*)(version)) + " (GLSL: " + std::string((const char*)(glslVersion)) + ")";
	};
};


using ComputerInfo = WinComputerInfo;

} // namespace ds::cfg::impl
