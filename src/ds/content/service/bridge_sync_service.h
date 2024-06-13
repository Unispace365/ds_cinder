#pragma once
#ifndef CONTENT_BRIDGE_SYNC_SERVICE
#define CONTENT_BRIDGE_SYNC_SERVICE

#include <Poco/Pipe.h>
#include <Poco/Process.h>

#include <ds/app/auto_update.h>
#include <ds/app/event.h>
#include <ds/app/event_client.h>
#include <ds/network/https_client.h>

namespace ds::content {


struct BridgeSyncSettings {
	// std::string name		 = "";
	std::string syncPath	   = "";
	std::string server		   = "";
	std::string authServer	   = "";
	std::string clientId	   = "";
	std::string clientSecret   = "";
	std::string directory	   = "";
	std::string interval	   = "";
	std::string additionalArgs = "";
	bool		verbose		   = false;
};

/**
 * \class ds::content::BridgeSyncService
 *					Runs a content sync application (downsync) in a sub process
 *
 */
class BridgeSyncService : public ds::AutoUpdate {
  public:
	BridgeSyncService(ds::ui::SpriteEngine&);
	~BridgeSyncService();

	void initialize(const BridgeSyncSettings& settings);
	void toggleOutput() { mShowOutput = !mShowOutput; }
	void showOutput(bool show = true) { mShowOutput = show; }
	// Inherited via AutoUpdate
	virtual void update(const ds::UpdateParams&) override;

  private:
	bool					mExit	 = false;
	bool					mStarted = false;
	HANDLE					mJobObj;
	std::string				mPath = "";
	Poco::Pipe				mOutPipe;
	Poco::Pipe				mErrPipe;
	std::string				mFirstLine = "";
	std::deque<std::string> mStdoutBuffer;
	std::thread				mThreadObj;
	ds::ui::SpriteEngine&	mEngine;
	Poco::Process::PID		mProcessId = 0;
	std::mutex				mMutex;
	size_t					mShowCount	= 1000;
	bool					mTestScroll = true;
	bool					mShowOutput = false;
};

} // namespace ds::content


#endif
