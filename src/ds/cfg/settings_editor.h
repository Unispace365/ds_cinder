#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR
#define DS_CFG_SETTINGS_EDITOR

#include <Poco/DateTime.h>

#include <ds/app/event_client.h>
#include <ds/cfg/settings.h>
#include <ds/network/https_client.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds::cfg {

/// View for displaying and editing settings
class SettingsEditor : public ds::ui::Sprite {
  public:
	SettingsEditor(ds::ui::SpriteEngine& e);

	void showSettings(const std::string settingsName);
	void hideSettings();

	virtual void drawPostLocalClient() override;

  private:
	friend class ds::Engine;
	void drawMenu();

	void drawSettings();
	void drawContent();
	void drawAppStatus();
	void drawAppHostStatus();
	void drawSyncStatus();
	void drawShortcuts();
	void drawLog();

	void drawSettingFile(ds::cfg::Settings& eng);
	void drawSingleSetting(ds::cfg::Settings::Setting& setting, ds::cfg::Settings& allSettings,
						   const std::string& search);

	void drawSaveButtons(ds::cfg::Settings& toSave);
	void saveChange(const std::string& path, ds::cfg::Settings& toSave);

	ds::EventClient mEventClient;
	Settings*		mCurrentSettings;


	std::map<std::string, std::string> mSearchMap;
	std::map<std::string, std::string> mFilterMap;

	bool mOpen				= false;
	bool mAppStatusOpen		= false;
	bool mSyncStatusOpen	= false;
	bool mAppHostStatusOpen = false;
	bool mEngineOpen		= false;
	bool mAppSettingsOpen	= false;
	bool mStylesOpen		= false;
	bool mFontsOpen			= false;
	bool mTuioOpen			= false;
	bool mContentOpen		= false;
	bool mShortcutsOpen		= false;
	bool mImguiStyleOpen	= false;
	bool mLogOpen			= false;

	// AppHost
	ds::net::HttpsRequest mHttpsRequest;
	bool				  mAppHostRunning = false;

	// Logs
	bool		mLogAutoScroll = true;
	std::string mLogBuffer;

	// Sync
	Poco::DateTime mLastSync;
};

} // namespace ds::cfg
#endif //
