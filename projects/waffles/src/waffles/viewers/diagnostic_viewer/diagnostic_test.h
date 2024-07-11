#pragma once

#include <ds/debug/computer_info.h>
#include <ds/network/https_client.h>
#include <ds/ui/layout/smart_layout.h>

namespace waffles {

/**
 * \class ds::DiagnosticTest
 *			A single diagnostic test
 */
class DiagnosticTest : public ds::ui::SmartLayout {
  public:
	static const int TEST_TYPE_URL				= 0;
	static const int TEST_TYPE_TOUCH			= 1;
	static const int TEST_TYPE_SCREEN_SETUP		= 2;
	static const int TEST_TYPE_GSTREAMER		= 3;
	static const int TEST_TYPE_WRITE_ACCESS		= 4;
	static const int TEST_TYPE_SOUND_CARD_GUIDS = 5;
	static const int TEST_TYPE_MEMORY_INFO		= 6;
	static const int TEST_TYPE_VIDEO_CARD_INFO	= 7;
	static const int TEST_TYPE_IP				= 8;

	DiagnosticTest(ds::ui::SpriteEngine& g, const float widdy, const int type, const std::string& url = "");

	void runTest();
	void setCompleteCallback(std::function<void()> completeCallback);
	void setLayoutCallback(std::function<void()> layoutCallback);

	const std::string getOutput();
	const bool		  passed();

  private:
	void				  layout();
	void				  setStatus(const bool passed);
	int					  mTestType;
	bool				  mPassed;
	std::string			  mOutputString;
	std::string			  mUrl;
	std::function<void()> mCompleteCallback;
	std::function<void()> mLayoutCallback;

	ds::net::HttpsRequest mHttpsRequest;
	ds::ComputerInfo	  mComputerInfo;
};

} // namespace waffles
