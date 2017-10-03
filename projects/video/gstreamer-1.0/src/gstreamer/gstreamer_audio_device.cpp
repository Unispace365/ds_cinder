#include "gstreamer_audio_device.h"


#ifdef _WIN32
#include <windows.h>
#include <WinUser.h>
#include <mmsystem.h>
#include <dsound.h>

#include <ds/util/string_util.h>
#include <ds/debug/logger.h>

namespace {

struct SoundCardInfo {
	LPGUID lpGuid;
	char devName[100];
	WCHAR description[100];
	char stringGuid[100];
	std::wstring wstringGuid;
	WCHAR wwstringGuid[100];

};



BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext) {

	// this is a list of sound cards and GUIDs
	std::vector<SoundCardInfo *> *soundCardList = (std::vector<SoundCardInfo *> *)lpContext;

	SoundCardInfo *sci = new SoundCardInfo();

	// move the needed info into the data structure   
	std::stringstream ss;
	ss << lpszDesc << std::endl;
	std::string tmpString = ss.str();
	sci->lpGuid = NULL;


	wcscpy(sci->description, lpszDesc);

	std::stringstream ss2;
	ss2 << lpszDrvName << std::endl;
	std::string tmpString2 = ss2.str();
	strcpy(sci->devName, tmpString2.c_str());


	// NULL is the primary sound driver. Make sure you don't memcpy from NULL. Visual Studio doesn't like it when you do that
	if(lpGUID != NULL){
		// put the sound card information into the vector of pointers
		sci->lpGuid = new GUID();

		// something went wrong trying to allocate the RAM. Remember that "lp" in lpGUID means "long pointer", which is about the same as *GUID
		if(sci->lpGuid == NULL)
		{
			return TRUE;
		}
		// copy the GUID over
		memcpy(sci->lpGuid, lpGUID, sizeof(GUID));
		sci->wstringGuid = (WCHAR*)(lpGUID);
		StringFromGUID2(*lpGUID, sci->wwstringGuid, sizeof(sci->wwstringGuid));

	}

	// and add the information to the list
	soundCardList->push_back(sci);

	return TRUE;

}

}
#endif

namespace ds{

GstAudioDevice::GstAudioDevice(const std::string& deviceName, const std::string& channelMask /*= "0x3"*/, const std::string& deviceGuid /*= ""*/, const double theVolume /*= 1.0*/) 
	: mDeviceName(deviceName)
	, mDeviceGuid(deviceGuid)
	, mChannelMask(channelMask)
	, mVolume(1.0)
{
}

void GstAudioDevice::initialize(){

#ifdef _WIN32
	if(mDeviceGuid.empty()){

		BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext);

		bool found = false;

		std::vector<SoundCardInfo *> soundCardList;
		// this is all you have to do. You don't even need to have a DirectSound device set up
		if((DirectSoundEnumerate(DSEnumProc, (LPVOID)&soundCardList)) == DS_OK){
			for(auto it : soundCardList){
				std::wstringstream ss;
				ss << it->description;
				if(ss.str().find(ds::wstr_from_utf8(mDeviceName)) != std::wstring::npos){
					std::wstringstream guidSs;
					guidSs << it->wwstringGuid;

					mDeviceGuid = ds::utf8_from_wstr(guidSs.str());
					found = true;
					break;
				}
			}
		}

		if(!found){
			DS_LOG_WARNING("GstAudioDevice: couldn't find a matching audio device for device name = " << mDeviceName);
		}
	}

#endif // _WIN32

}

}