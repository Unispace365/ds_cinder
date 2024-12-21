#pragma once

#include <cinder/Json.h>

#include <Poco/DateTime.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>

#include <ds/app/event_client.h>
#include <ds/content/platform.h>
#include <ds/network/helper/delayed_node_watcher.h>
#include <ds/network/https_client.h>
#include <ds/network/node_watcher.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds::content {

/**
 * \class BridgeClient
 * \brief A service that runs in the background and communicates with the BridgeSync application.
 */

class BridgeClient : public Poco::Runnable {
	inline static char SEPARATOR = '/';

  public:
	BridgeClient(ui::SpriteEngine& engine)
	  : mEngine(engine)
	  , mSocket(Poco::Net::SocketAddress{"127.0.0.1", 0 /* Let OS find an unused port. */}, false, false) {}

	~BridgeClient() override = default;

	BridgeClient(const BridgeClient&)			 = delete;
	BridgeClient(BridgeClient&&)				 = delete;
	BridgeClient& operator=(const BridgeClient&) = delete;
	BridgeClient& operator=(BridgeClient&&)		 = delete;

	void run() override;

	void abort() { mAbort = true; }
	bool isAborted() const { return mAbort; }

  private:
	// Send a message to the server.
	bool sendToServer(const std::vector<std::string>& parts);
	// Send a message to the server.
	bool sendToServer(const std::string& msg);

	ui::SpriteEngine&		  mEngine;		 //
	mutable Poco::Mutex		  mMutex;		 //
	Poco::Net::DatagramSocket mSocket;		 //
	Poco::Net::SocketAddress  mServer;		 //
	std::atomic_bool		  mAbort{false}; //
};

/**
 * \class BridgeService
 * \brief A service that runs in the background and refreshes the database content.
 */

class BridgeService {
  public:
	explicit BridgeService(ds::ui::SpriteEngine& engine);
	~BridgeService();

	void start();
	void stop();

	//
	void refreshDatabase(bool force = false);
	//
	void refreshEvents(bool force = false);

	bool isRunning() const { return mThread.isRunning(); }

	void setValidator(const std::function<bool(const ds::model::ContentModelRef&)> validator) {
		mLoop.setValidator(validator);
	};

  private:
	class Loop final : public Poco::Runnable {
	  public:
		explicit Loop(ds::ui::SpriteEngine& engine);

		void run() override;

		//
		void refreshDatabase(bool force = false);
		//
		void refreshEvents(bool force = false);

		// Signal the background thread that it should abort.
		void abort();

		void setValidator(const std::function<bool(const ds::model::ContentModelRef&)> validator) {
			mValidator = validator;
		};

	  private:
		///
		bool eventIsNow(ds::model::ContentModelRef& event, Poco::DateTime& ldt) const;

		///
		bool loadContent();
		///
		void validateContent();
		///
		void updatePlatformEvents() const;

		ci::app::AppBase* mApp =
			nullptr; // Pointer to main application, allowing us to execute code on the main thread.

		ds::ui::SpriteEngine&										mEngine;	   // Reference to the sprite engine.
		Poco::Mutex													mContentMutex; // Controls access to content.
		ds::model::ContentModelRef									mContent;	   //
		ds::model::ContentModelRef									mPlatforms;	   //
		ds::model::ContentModelRef									mEvents;	   //
		ds::model::ContentModelRef									mRecords;	   //
		ds::model::ContentModelRef									mTags;		   // all the tags
		std::unordered_map<std::string, ds::model::ContentModelRef> mRecordMap;	   // all the records
		std::unordered_map<std::string, ds::model::ContentModelRef>
					mValidMap;		  // all the valid records (as determined by the validator)
		Poco::Mutex mMutex;			  // Controls access to abort, force and refresh flags.
		bool		mAbort;			  // If true, will abort the background thread.
		bool		mForce;			  // If true, will force a refresh of the content.
		bool		mRefreshDatabase; // If true, will refresh the database content.
		bool		mRefreshEvents;	  // If true, will refresh (only) the events.
		const long	mRefreshRateMs;	  // in milliseconds
		int			mResourceId											  = 1; //
		int			mLoadContentCount									  = 0;
		std::function<bool(const ds::model::ContentModelRef&)> mValidator = nullptr;
	};

	ds::EventClient				  mEventClient;			   //
	ds::ui::SpriteEngine&		  mEngine;				   //
	ds::time::Callback			  mRefreshTimer;		   //
	Poco::Thread				  mThread;				   //
	Loop						  mLoop;				   //
	Poco::SharedPtr<BridgeClient> mBridgeConnection;	   //
	Poco::Thread				  mBridgeConnectionThread; //
};


} // namespace ds::content
