#pragma once
#ifndef CONTENT_BRIDGE_SYNC_SERVICE
#define CONTENT_BRIDGE_SYNC_SERVICE

#include <Poco/Pipe.h>
#include <Poco/Process.h>

#include <ds/app/auto_update.h>
#include <ds/network/https_client.h>

namespace ds::content {


struct BridgeSyncSettings {
	std::string syncPath;
	std::string server;
	std::string authServer;
	std::string clientId;
	std::string clientSecret;
	std::string directory;
	std::string interval;
	std::string additionalArgs;
	bool		verbose = false;
};

/**
 * \class ds::content::BridgeSyncService
 *					Runs a content sync application (downsync) in a sub process
 *
 */
class BridgeSyncService : public ds::AutoUpdate {
  public:
	explicit BridgeSyncService(ds::ui::SpriteEngine& engine);
	~BridgeSyncService() override;

	BridgeSyncService(const BridgeSyncService&)			   = delete;
	BridgeSyncService(BridgeSyncService&&)				   = delete;
	BridgeSyncService& operator=(const BridgeSyncService&) = delete;
	BridgeSyncService& operator=(BridgeSyncService&&)	   = delete;

	void start(const BridgeSyncSettings& settings);
	void stop();

	void initialize(const BridgeSyncSettings& settings) { start(settings); }
	void toggleOutput() { mShowOutput = !mShowOutput; }
	void showOutput(bool show = true) { mShowOutput = show; }

	// Inherited via AutoUpdate
	void update(const ds::UpdateParams&) override;

  private:
	class Loop : public Poco::Runnable {
	  public:
		Loop() = default;

		void setSettings(const BridgeSyncSettings& settings) { mSettings = settings; }

		void run() override;

		// Signal the background thread that it should abort.
		void abort() { mExit = true; }

		void clearBuffer();
		bool readBuffer(std::deque<std::string>& buffer);

	  private:
		std::deque<std::string> mStdoutBuffer;
		BridgeSyncSettings		mSettings;
		Poco::Mutex				mMutex; // Controls access to mStdoutBuffer
		Poco::Pipe				mOutPipe;
		Poco::Pipe				mErrPipe;
		std::atomic_bool		mExit = false;
	};

	std::deque<std::string> mStdoutBuffer;		 //
	Poco::Thread			mThread;			 //
	Loop					mLoop;				 //
	size_t					mShowCount	= 1000;	 //
	bool					mShowOutput = false; //
};

} // namespace ds::content


#endif
