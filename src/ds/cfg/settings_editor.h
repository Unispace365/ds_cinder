#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR
#define DS_CFG_SETTINGS_EDITOR

#include "ds/app/event_client.h"
#include "ds/cfg/settings.h"
#include "ds/network/https_client.h"
#include "ds/ui/sprite/sprite.h"

#include <Poco/DateTime.h>
#include <Poco/Environment.h>

namespace ds::cfg {

/// View for displaying and editing settings
class SettingsEditor : public ui::Sprite {
  public:
	SettingsEditor(ui::SpriteEngine& e);

	void toggleSetting(const std::string& settingsName);
	void showSettings(const std::string& settingsName);
	void hideSettings();

	void drawPostLocalClient() override;

  private:
	friend class Engine;
	void drawMenu();

	void drawSettings();
	void drawContent();
	void drawAppStatus();
	void drawAppStatusInfo();
	void drawAppHostStatus();
	void drawSyncStatus();
	void drawSyncStatusInfo() const;
	void drawShortcuts();
	void drawLog();

	void drawSettingFile(Settings& eng, bool& isOpen);
	void drawSingleSetting(Settings::Setting& setting, Settings& allSettings, const std::string& search,
						   bool multiple = false);

	void        drawSaveButtons(Settings& toSave);
	static void saveChange(const std::string& path, Settings& toSave);

	EventClient							 mEventClient;
	Settings*							 mCurrentSettings;
	std::unordered_map<std::string, int> mSettingCounters;


	std::unordered_map<std::string, std::string> mSearchMap;
	std::unordered_map<std::string, std::string> mFilterMap;

	bool mOpen				  = false;
	bool mAppStatusOpen		  = false;
	bool mSyncStatusOpen	  = false;
	bool mAppHostStatusOpen	  = false;
	bool mEngineOpen		  = false;
	bool mAppSettingsOpen	  = false;
	bool mWafflesSettingsOpen = false;
	bool mStylesOpen		  = false;
	bool mFontsOpen			  = false;
	bool mTuioOpen			  = false;
	bool mContentOpen		  = false;
	bool mShortcutsOpen		  = false;
	bool mImGuiStyleOpen	  = false;
	bool mLogOpen			  = false;

	// App Status
	int			mSpriteCount = 0;
	std::string mTouchMode;
	float		mPhysicalMemory = 0.f;
	float		mVirtualMemory	= 0.f;
	int			mBytesReceived	= 0;
	int			mBytesSent		= 0;
	float		mFps			= 0.f;

	bool	  mSrcDestSaved = false;
	ci::Rectf mOrigSrc, mOrigDest;

	// AppHost
	net::HttpsRequest mHttpsRequest;
	bool			  mAppHostRunning = false;

	// Logs
	bool		mLogAutoScroll = true;
	std::string mLogBuffer;

	// Sync
	Poco::DateTime	  mLastSync;
	Poco::Environment mEnv;
	std::string		  mAppVersion;
	std::string		  mProductName;
	std::string		  mOsVersion;
	std::string		  mGlVendor;
	std::string		  mGlVersion;
};

} // namespace ds::cfg
#endif //
