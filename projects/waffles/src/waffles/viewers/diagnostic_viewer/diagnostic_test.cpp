#include "stdafx.h"

#include "diagnostic_test.h"

#include <mmsystem.h>
// dsound.h MUST come after mmsystem.h
#include <dsound.h>

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/data/resource.h>
#include <ds/debug/computer_info.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>
#include <ds/network/network_info.h>

// #include "app/waffles_app_defs.h"
// #include "events/app_events.h"

struct SoundCardInfo {
	LPGUID		 lpGuid;
	char		 devName[100];
	WCHAR		 description[100];
	char		 stringGuid[100];
	std::wstring wstringGuid;
	WCHAR		 wwstringGuid[100];
};

BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext) {
	// this is a list of sound cards and GUIDs
	std::vector<SoundCardInfo*>* soundCardList = (std::vector<SoundCardInfo*>*)lpContext;

	SoundCardInfo* sci = new SoundCardInfo();

	// move the needed info into the data structure
	std::stringstream ss;
	ss << lpszDesc << std::endl;
	std::string tmpString = ss.str();
	sci->lpGuid			  = NULL;


	wcscpy_s(sci->description, lpszDesc);

	std::stringstream ss2;
	ss2 << lpszDrvName << std::endl;
	std::string tmpString2 = ss2.str();
	strcpy_s(sci->devName, tmpString2.c_str());


	// NULL is the primary sound driver. Make sure you don't memcpy from NULL. Visual Studio doesn't like it when you do
	// that
	if (lpGUID != NULL) {
		// put the sound card information into the vector of pointers
		sci->lpGuid = new GUID();

		// something went wrong trying to allocate the RAM. Remember that "lp" in lpGUID means "long pointer", which is
		// about the same as *GUID
		if (sci->lpGuid == NULL) {
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

namespace waffles {

DiagnosticTest::DiagnosticTest(ds::ui::SpriteEngine& g, const float widdy, const int type, const std::string& url)
	: ds::ui::SmartLayout(g, "waffles/diagnostics/diagnostic_test.xml")
	, mTestType(type)
	, mPassed(false)
	, mUrl(url)
	, mHttpsRequest(g)
	, mComputerInfo(ds::ComputerInfo::MEGABYTE, ds::ComputerInfo::ALL_ON) {

	std::string titley	 = "";
	std::string detailsy = "";

	if (type == TEST_TYPE_URL) {
		titley	 = "Connectivity";
		detailsy = mUrl;
	} else if (type == TEST_TYPE_TOUCH) {
		titley	 = "Touch Input";
		detailsy = "Windows Native Touch availability";
	} else if (type == TEST_TYPE_MEMORY_INFO) {
		titley	 = "Memory Info";
		detailsy = "Physical memory available";
	} else if (type == TEST_TYPE_SCREEN_SETUP) {
		titley	 = "Screen Setup";
		detailsy = "Number of monitors and pixels";
	} else if (type == TEST_TYPE_SOUND_CARD_GUIDS) {
		titley	 = "Sound Card GUIDs";
		detailsy = "Enumerate the GUIDs for all sound cards attached";
	} else if (type == TEST_TYPE_VIDEO_CARD_INFO) {
		titley	 = "Video Card";
		detailsy = "Graphics card info.";
	} else if (type == TEST_TYPE_IP) {
		titley	 = "IP Address";
		detailsy = "Current IP address for this machine.";
	}

	setSpriteText("title", titley);
	setSpriteText("details", detailsy);

	setSize(widdy, getHeight());
	layout();
}

void DiagnosticTest::runTest() {

	setSpriteText("status", "Running...");

	if (mTestType == TEST_TYPE_URL) {
		mHttpsRequest.setReplyFunction([this](const bool errored, const std::string reply, const long httpStatusCode) {
			mOutputString = "Http status from " + mUrl + " is " + std::to_string(httpStatusCode) + "\n" + reply;

			setSpriteText("details", "Http status from " + mUrl + " is " + std::to_string(httpStatusCode));
			setStatus(!errored);
		});
		// WINVER
		mHttpsRequest.makeGetRequest(mUrl, false, false);
	}
	/**/
	else if (mTestType == TEST_TYPE_TOUCH) {
		int	 vally		= GetSystemMetrics(SM_DIGITIZER);
		auto maxTouches = 0;
		if (vally & NID_READY && vally & NID_MULTI_INPUT) {
			maxTouches = GetSystemMetrics(SM_MAXIMUMTOUCHES);
		}

		if (maxTouches < 1) {
			mOutputString = "No native touch system found.";
			setSpriteText("details", mOutputString);
			setStatus(false);
		} else {
			mOutputString = "Native touch found with " + std::to_string(maxTouches) + " touch points";
			setSpriteText("details", mOutputString);
			setStatus(true);
		}
	} else if (mTestType == TEST_TYPE_MEMORY_INFO) {
		double currentUsage	  = mEngine.getComputerInfo().getPhysicalMemoryUsedByProcess();
		double physicalMemory = mEngine.getComputerInfo().getTotalPhysicalMemory();
		double percentUsage	  = mEngine.getComputerInfo().getPercentUsageCPU();
		mOutputString		  = "Total memory: " + std::to_string(physicalMemory) +
						"MB\nCurrent Usage: " + std::to_string(currentUsage) +
						"MB\nProcessor Use: " + std::to_string(percentUsage) +
						"%\nNumber of Processors: " + std::to_string(mEngine.getComputerInfo().getNumberOfProcessors());
		setSpriteText("details", mOutputString);
		if (physicalMemory < 2048) { // 2gb of ram
			setStatus(false);
		} else {
			setStatus(true);
		}
	} else if (mTestType == TEST_TYPE_SCREEN_SETUP) {
		int virtualWidth  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		int numScreens	  = GetSystemMetrics(SM_CMONITORS);

		mOutputString = "Number of Screens: " + std::to_string(numScreens) +
						" Total Pixels: " + std::to_string(virtualWidth) + " x " + std::to_string(virtualHeight);
		setSpriteText("details", mOutputString);
		if (virtualWidth > 1500 && virtualHeight + 1000) {
			setStatus(true);
		} else {
			setStatus(false);
		}
	} else if (mTestType == TEST_TYPE_SOUND_CARD_GUIDS) {
		bool		  success = false;
		BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext);

		std::vector<SoundCardInfo*> soundCardList;
		// this is all you have to do. You don't even need to have a DirectSound device set up
		if ((DirectSoundEnumerate(DSEnumProc, (LPVOID)&soundCardList)) == DS_OK) {
			std::wstring outputty = L"";
			success				  = true;
			for (auto it = soundCardList.begin(); it != soundCardList.end(); ++it) {
				outputty.append((*it)->description);
				outputty.append(L"\n");
				outputty.append((*it)->wwstringGuid);
				outputty.append(L"\n");
				outputty.append(L"\n");
			}

			setSpriteText("details", outputty);
			mOutputString = ds::utf8_from_wstr(outputty);
		} else {
			success		  = false;
			mOutputString = "Couldn't find sound card info!";
			setSpriteText("details", mOutputString);
		}


		setStatus(success);
	} else if (mTestType == TEST_TYPE_VIDEO_CARD_INFO) {
		mComputerInfo.update();

		double		vidMemory	= mComputerInfo.getTotalVideoMemory();
		std::string vidDriver	= mComputerInfo.getVideoDriverVersion();
		std::string vidCard		= mComputerInfo.getVideoCardName();
		int			refreshRate = mComputerInfo.getVideoRefreshRate();

		mOutputString = vidCard + "\nDriver Version: " + vidDriver + "\nTotal Memory: " + std::to_string(vidMemory) +
						"MB\nRefresh Rate: " + std::to_string(refreshRate);
		setSpriteText("details", mOutputString);
		if (vidMemory > 256 && !vidCard.empty()) {
			setStatus(true);
		} else {
			setStatus(false);
		}
	} else if (mTestType == TEST_TYPE_IP) {

		ds::network::networkInfo Networki;

		auto addressy = Networki.getAddress();
		mOutputString = "IP Address: " + addressy;
		setSpriteText("details", mOutputString);
		if (addressy.empty()) {
			setStatus(false);
		} else {
			setStatus(true);
		}
	}

	layout();
}

void DiagnosticTest::setCompleteCallback(std::function<void()> completeCallback) {
	mCompleteCallback = completeCallback;
}

void DiagnosticTest::setLayoutCallback(std::function<void()> callback) {
	mLayoutCallback = callback;
}

const std::string DiagnosticTest::getOutput() {
	return mOutputString;
}

const bool DiagnosticTest::passed() {
	return mPassed;
}


void DiagnosticTest::layout() {
	runLayout();
	auto cl = getSprite("content_layout");
	if (cl) {
		setSize(cl->getWidth(), cl->getHeight());
	}

	if (mLayoutCallback) mLayoutCallback();
}

void DiagnosticTest::setStatus(const bool passed) {
	auto st = getSprite<ds::ui::Text>("status");

	mPassed = passed;
	if (mPassed) {
		if (st) {
			st->setText("Success");
			st->setColor(ci::Color(0.0f, 0.4f, 0.0f));
		}
	} else {
		if (st) {
			st->setText("Failed");
			st->setColor(ci::Color(0.4f, 0.0f, 0.0f));
		}
	}

	std::string theTest = "A";
	auto		tx		= getSprite<ds::ui::Text>("title");
	if (tx) theTest = tx->getTextAsString();

	DS_LOG_INFO(theTest << " test has status " << mPassed << " output: " << mOutputString);

	layout();

	if (mCompleteCallback) mCompleteCallback();
}

} // namespace waffles
