#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR
#define DS_CFG_SETTINGS_EDITOR

#include <Poco/DateTime.h>

#include <ds/cfg/settings.h>
#include <ds/app/event_client.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/network/https_client.h>

namespace ds::cfg {

/// View for displaying and editing settings
/// Note that the ui is specified in c++ and not an external layout file
/// This is designed to work for any app without any sidecar files
/// So it's best not to use this as an example of best practices (unless you're also making a portable ui)
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
	void drawSingleSetting(ds::cfg::Settings::Setting& setting, ds::cfg::Settings& allSettings, const std::string& search);

	void drawSaveButtons(ds::cfg::Settings& toSave);
	void saveChange(const std::string& path, ds::cfg::Settings& toSave);

	ds::EventClient mEventClient;
	Settings* mCurrentSettings;


	std::map<std::string, std::string> mSearchMap;
	std::map<std::string, std::string> mFilterMap;

	bool mOpen = false;
	bool mAppStatusOpen = true;
	bool mSyncStatusOpen = true;

	bool mAppHostStatusOpen = true;
	ds::net::HttpsRequest mHttpsRequest;
	bool mAppHostRunning = false;

	bool mEngineOpen = true;
	bool mAppSettingsOpen = true;
	bool mStylesOpen = true;
	bool mFontsOpen = true;
	bool mTuioOpen = true;

	bool mContentOpen = true;

	bool mShortcutsOpen = true;

	bool mLogOpen = true;
	bool mLogAutoScroll = true;
	std::string mLogBuffer;

	Poco::DateTime mLastSync;

	// EditView*				 mEditView;
};

} // namespace ds::cfg
#endif //
