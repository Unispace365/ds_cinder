#include "stdafx.h"

#include "settings_editor.h"

#include <cinder/CinderImGui.h>
#include <cinder/Clipboard.h>
#include <cinder/app/Platform.h>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include <Poco/DateTimeFormatter.h>
#include <Poco/File.h>
#include <Poco/Net/DNS.h>

#include <pango/pango-fontmap.h>

#include <ds/app/blob_reader.h>
#include <ds/app/blob_registry.h>
#include <ds/app/engine/engine_data.h>
#include <ds/cfg/settings.h>
#include <ds/cfg/settings_variables.h>
#include <ds/content/content_events.h>
#include <ds/debug/computer_info.h>
#include <ds/debug/logger.h>
#include <ds/math/math_defs.h>
#include <ds/util/color_util.h>

// Select the platform specific implementation OS verion / app version / product name
// Currently windows only with a null "stub" for future additions
#include "impl/impl.h"

namespace ds::cfg {

SettingsEditor::SettingsEditor(ds::ui::SpriteEngine& e)
  : ds::ui::Sprite(e)
  , mEventClient(e)
  , mHttpsRequest(e) {

	hide();
	setTransparent(false);

	mHttpsRequest.setVerboseOutput(false);
	mHttpsRequest.setReplyFunction([this](const bool errored, const std::string& reply, long httpCode) {
		if (errored && httpCode == 0) {
			mAppHostRunning = false;
		} else {
			mAppHostRunning = true;
		}
	});
	mHttpsRequest.makeGetRequest("localhost:7800/api/status");

	// Get the current application version
	auto info	 = impl::ComputerInfo();
	mAppVersion	 = info.getAppVersionString();
	mProductName = info.getAppProductName();
	mOsVersion	 = info.getOsVersion();
	mGlVendor	 = info.getOpenGlVendor();
	mGlVersion	 = info.getOpenglVersion();

	// Set mLastSync years in the past (to signify no sync yet)
	mLastSync.assign(2000, 1, 1);

	// Update the last sync time every time we get a dsnode message or CMS loading complete
	mEventClient.listenToEvents<ds::CmsDataLoadCompleteEvent>([this](const auto& e) { mLastSync = Poco::DateTime(); });
	mEventClient.listenToEvents<ds::DsNodeMessageReceivedEvent>(
		[this](const auto& e) { mLastSync = Poco::DateTime(); });
}


void SettingsEditor::drawPostLocalClient() {
	if (mOpen && visible()) {
		drawMenu();
		drawSettings();

		if (mContentOpen) drawContent();
		if (mAppStatusOpen) drawAppStatus();
		if (mAppHostStatusOpen) drawAppHostStatus();
		if (mSyncStatusOpen) drawSyncStatus();
		if (mShortcutsOpen) drawShortcuts();
		if (mLogOpen) drawLog();


		if (mImguiStyleOpen) {
			ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("ImGui", &mImguiStyleOpen, ImGuiWindowFlags_NoFocusOnAppearing)) {
				ImGui::ShowStyleSelector("Style select");
				ImGui::ShowStyleEditor();
			}
			ImGui::End();
		}
	}
}

void SettingsEditor::drawMenu() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Quick Info")) {
			if (ImGui::Button("Exit Debug")) {
				auto& eng = ((ds::Engine&)(mEngine));
				eng.hideSettingsEditor();
			}
			if (ImGui::Button("Open All")) {
				mAppStatusOpen	   = true;
				mSyncStatusOpen	   = true;
				mAppHostStatusOpen = true;
				mLogOpen		   = true;
				mEngineOpen		   = true;
				mAppSettingsOpen   = true;
				mStylesOpen		   = true;
				mFontsOpen		   = true;
				mTuioOpen		   = true;
				mContentOpen	   = true;
				mShortcutsOpen	   = true;
				mImguiStyleOpen	   = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Close All")) {
				mAppStatusOpen	   = false;
				mSyncStatusOpen	   = false;
				mAppHostStatusOpen = false;
				mLogOpen		   = false;
				mEngineOpen		   = false;
				mAppSettingsOpen   = false;
				mStylesOpen		   = false;
				mFontsOpen		   = false;
				mTuioOpen		   = false;
				mContentOpen	   = false;
				mShortcutsOpen	   = false;
				mImguiStyleOpen	   = false;
			}

			drawAppStatusInfo();
			ImGui::NewLine();

			drawSyncStatusInfo();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Status")) {
			if (ImGui::MenuItem("Open All", nullptr)) {
				mAppStatusOpen	   = true;
				mSyncStatusOpen	   = true;
				mAppHostStatusOpen = true;
				mLogOpen		   = true;
			}
			if (ImGui::MenuItem("App status", nullptr, mAppStatusOpen)) {
				mAppStatusOpen = !mAppStatusOpen;
			}
			if (ImGui::MenuItem("Sync status", nullptr, mSyncStatusOpen)) {
				mSyncStatusOpen = !mSyncStatusOpen;
			}
			if (ImGui::MenuItem("AppHost status", nullptr, mAppHostStatusOpen)) {
				mAppHostStatusOpen = !mAppHostStatusOpen;
			}
			if (ImGui::MenuItem("Logs", nullptr, mLogOpen)) {
				mLogOpen = !mLogOpen;
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Settings")) {
			if (ImGui::MenuItem("Open All")) {
				mEngineOpen		 = true;
				mAppSettingsOpen = true;
				mStylesOpen		 = true;
				mFontsOpen		 = true;
				mTuioOpen		 = true;
			}

			if (ImGui::MenuItem("Engine", nullptr, mEngineOpen)) {
				mEngineOpen = !mEngineOpen;
			}
			if (ImGui::MenuItem("App Settings", nullptr, mAppSettingsOpen)) {
				mAppSettingsOpen = !mAppSettingsOpen;
			}
			if (ImGui::MenuItem("Styles", nullptr, mStylesOpen)) {
				mStylesOpen = !mStylesOpen;
			}
			if (ImGui::MenuItem("Fonts", nullptr, mFontsOpen)) {
				mFontsOpen = !mFontsOpen;
			}
			if (ImGui::MenuItem("Tuio", nullptr, mTuioOpen)) {
				mTuioOpen = !mTuioOpen;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Content")) {
			if (ImGui::MenuItem("Content Viewer", nullptr, mContentOpen)) {
				mContentOpen = !mContentOpen;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("App Shortcuts", nullptr, mShortcutsOpen)) {
				mShortcutsOpen = !mShortcutsOpen;
			}

			if (ImGui::MenuItem("ImGui Style", nullptr, mImguiStyleOpen)) {
				mImguiStyleOpen = !mImguiStyleOpen;
			}

			ImGui::EndMenu();
		}


		ImGui::EndMainMenuBar();
	}
}

void SettingsEditor::drawSettings() {
	if (mEngineOpen) drawSettingFile(mEngine.getEngineCfg().getSettings("engine"), mEngineOpen);
	if (mAppSettingsOpen) drawSettingFile(mEngine.getEngineCfg().getSettings("app_settings"), mAppSettingsOpen);

	if (mStylesOpen) {
		if (!mEngine.getEngineCfg().getSettings("styles").empty()) {
			drawSettingFile(mEngine.getEngineCfg().getSettings("styles"), mStylesOpen);
		} else {
			drawSettingFile(mEngine.getEngineCfg().getSettings("text"), mStylesOpen);
			drawSettingFile(mEngine.getEngineCfg().getSettings("colors"), mStylesOpen);
		}
	}

	if (mFontsOpen) drawSettingFile(mEngine.getEngineCfg().getSettings("fonts"), mFontsOpen);

	if (!mEngine.getEngineCfg().getSettings("tuio_inputs").empty() && mTuioOpen) {
		drawSettingFile(mEngine.getEngineCfg().getSettings("tuio_inputs"), mTuioOpen);
	}
}

namespace {
	void drawTree(ds::model::ContentModelRef model) {
		auto name = model.getPropertyString("name");
		if (name.empty()) name = model.getPropertyString("app_key");
		if (name.empty()) name = model.getName();
		if (name.empty()) name = model.getLabel();

		if (ImGui::TreeNode((name).data())) {
			if (!model.empty() && !model.getProperties().empty()) {
				ImGui::BeginTable("Properties", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
				ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();
				for (auto&& prop : model.getProperties()) {
					// Key
					ImGui::TableNextColumn();
					ImGui::Text(prop.first.data());

					// Value
					ImGui::TableNextColumn();
					if (prop.second.getResource().empty()) {
						ImGui::Text(prop.second.getString().data());
						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false)) {
							ci::Clipboard::setString(prop.second.getString());
						}
					} else {
						ImGui::Text(prop.second.getResource().getAbsoluteFilePath().data());
						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false)) {
							ci::Clipboard::setString(prop.second.getResource().getAbsoluteFilePath());
						}
					}

					// Copy value to clipboard on click
					// ImGui::LabelText(prop.first.data(), prop.second.getString().data());
				}
				ImGui::EndTable();

				ImGui::NewLine();
			}

			for (auto&& content : model.getChildren()) {
				drawTree(content);
			}
			ImGui::TreePop();
		}
	}
} // namespace
void SettingsEditor::drawContent() {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Content", &mContentOpen, ImGuiWindowFlags_NoFocusOnAppearing)) {
		auto& allContent = mEngine.mContent;
		drawTree(allContent);
	}
	ImGui::End();
}

void SettingsEditor::drawAppStatus() {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Status", &mAppStatusOpen, ImGuiWindowFlags_NoFocusOnAppearing)) {
		drawAppStatusInfo();
	}
	ImGui::End();
}

void SettingsEditor::drawAppStatusInfo() {
	auto& eng = ((ds::Engine&)(mEngine));

	// No need for these to change every single frame
	if (ci::app::getElapsedFrames() % 8 == 0) {
		mSpriteCount	= int(eng.mSprites.size());
		mTouchMode		= ds::ui::TouchMode::toString(eng.mTouchMode);
		mPhysicalMemory = mEngine.getComputerInfo().getPhysicalMemoryUsedByProcess();
		mVirtualMemory	= mEngine.getComputerInfo().getVirtualMemoryUsedByProcess();

		if (mEngine.getMode() != ds::ui::SpriteEngine::STANDALONE_MODE) {
			mBytesReceived = mEngine.getBytesRecieved();
			mBytesSent	   = mEngine.getBytesSent();
		}

		mFps = eng.getAverageFps();
	}

	if (!mProductName.empty()) {
		ImGui::Text("%s Info", mProductName.data());
	} else {
		ImGui::Text("App Info");
	}

	ImGui::Text("\tVersion: %s", mAppVersion.data());
	ImGui::Text("\tSprites: %i", int(mSpriteCount));
	ImGui::Text("\tFPS: %f", mFps);
	ImGui::Text("\tTouch Mode: %s", mTouchMode.data());
	ImGui::Text("\tPhysical Memory: %f", mPhysicalMemory);
	ImGui::Text("\tVirtual Memory: %f", mVirtualMemory);
	if (mEngine.getMode() != ds::ui::SpriteEngine::STANDALONE_MODE) {
		ImGui::Text("\tBytes Received: %i", mBytesReceived);
		ImGui::Text("\tBytes Sent: %i", mBytesSent);
	}
	ImGui::Separator();
	ImGui::Text("Computer Info");
	ImGui::Text("\tOS: %s", mOsVersion.data());
	ImGui::Text("\tArcitecture: %s", mEnv.osArchitecture().data());
	ImGui::Text("\tCores: %u", mEnv.processorCount());
	ImGui::Text("\tGraphics");
	ImGui::Text("\t\tVendor: %s", mGlVendor.data());
	ImGui::Text("\t\tVersion: %s", mGlVersion.data());

	ImGui::Text("Network");
	auto host = Poco::Net::DNS::thisHost();
	ImGui::Text("\tHostName: %s", host.name().data());
	for (auto&& alias : host.aliases()) {
		ImGui::Text("\tAlias: %s", alias.data());
	}
	for (auto&& addr : host.addresses()) {
		ImGui::Text("\taddr: %s", addr.toString().data());
	}

	ImGui::Text("Displays");
	if (auto plat = ci::app::Platform::get()) {
		int i = 1;
		for (auto&& disp : plat->getDisplays()) {
			ImGui::Text("\tDisplay %i : %i x %i", i, disp->getWidth(), disp->getHeight());
			++i;
		}
	}
}

void SettingsEditor::drawAppHostStatus() {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("AppHost", &mAppHostStatusOpen, ImGuiWindowFlags_NoFocusOnAppearing)) {
		if (mAppHostRunning) {
			ImGui::Text("AppHost running");
			if (ImGui::Button("Start")) {
				mHttpsRequest.makeGetRequest("http://localhost:7800/api/start");
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				mHttpsRequest.makeGetRequest("http://localhost:7800/api/stop");
			}

			if (ImGui::Button("Ping")) {
				mHttpsRequest.makeGetRequest("http://localhost:7800/api/ping");
			}

			if (ImGui::Button("Kiosk")) {
				mHttpsRequest.makeGetRequest("http://localhost:7800/api/kiosk");
			}
			ImGui::SameLine();
			if (ImGui::Button("Un-Kiosk")) {
				mHttpsRequest.makeGetRequest("http://localhost:7800/api/unkiosk");
			}

			if (ImGui::Button("Reload Config")) {
				mHttpsRequest.makeGetRequest("http://localhost:7800/api/reconfigure");
			}
			if (ImGui::Button("Exit AppHost")) {
				mHttpsRequest.makeGetRequest("http://localhost:7800/api/exit");
			}
			if (ImGui::Button("Reboot Machine")) {
				mHttpsRequest.makeGetRequest("http://localhost:7800/api/reboot");
			}

		} else {
			ImGui::Text("AppHost not running");
		}
	}
	ImGui::End();
}

void SettingsEditor::drawSyncStatus() {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Sync Status", &mSyncStatusOpen, ImGuiWindowFlags_NoFocusOnAppearing)) {
		drawSyncStatusInfo();
	}
	ImGui::End();
}

void SettingsEditor::drawSyncStatusInfo() {
	if (mLastSync.year() < Poco::DateTime().year()) {
		ImGui::Text("Sync incomplete or not running");
	} else {
		auto diff = Poco::DateTime() - mLastSync;

		if (diff.minutes() < 1) {
			ImGui::Text("Last Sync was %i seconds ago", diff.seconds());
		} else if (diff.hours() < 1) {
			ImGui::Text("Last Sync was %i minutes ago", diff.minutes());
		} else {
			ImGui::Text("Last Sync was %i hours ago", diff.hours());
		}
	}
}

void SettingsEditor::drawShortcuts() {

	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Shortcuts", &mShortcutsOpen, ImGuiWindowFlags_NoFocusOnAppearing)) {
		auto  appy	 = dynamic_cast<ds::App*>(ds::App::get());
		auto& keyMgr = appy->getKeyManager();

		ImGui::BeginTable("App Shortcuts", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
		ImGui::TableSetupColumn("Ctrl", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Alt", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Shift", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();
		for (auto&& key : keyMgr.getKeyRegistry()) {
			ImGui::TableNextColumn();
			if (key.mCtrlDown) {
				ImGui::Text("*");
			} else {
				ImGui::Text(" ");
			}

			ImGui::TableNextColumn();
			if (key.mAltDown) {
				ImGui::Text("*");
			} else {
				ImGui::Text(" ");
			}

			ImGui::TableNextColumn();
			if (key.mShiftDown) {
				ImGui::Text("*");
			} else {
				ImGui::Text(" ");
			}

			auto keyLabel = keyMgr.keyCodeToString(key.mKeyCode);
			ImGui::TableNextColumn();
			ImGui::Text(keyLabel.data());

			auto keyName = key.mName;
			ds::replace(keyName, "%", "%%"); // Escape '%' so it doesn't get interpreted as an escape sequence
			ImGui::TableNextColumn();
			ImGui::Text(keyName.data());
		}
		ImGui::EndTable();
	}
	ImGui::End();
}

void SettingsEditor::drawLog() {
	if (mLogBuffer.empty() || ci::app::getElapsedFrames() % 16 == 0) {
		mLogBuffer = ci::loadString(ci::loadFile(ds::getLogger().getLogFile()));
	}

	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Log", &mLogOpen, ImGuiWindowFlags_NoFocusOnAppearing)) {
		ImGui::Checkbox("Auto Scroll", &mLogAutoScroll);

		auto logSize = ImGui::GetWindowSize();
		logSize.y -= ImGui::GetTextLineHeightWithSpacing() * 4.f;
		ImGui::BeginChildFrame(69, logSize);
		ImGui::TextUnformatted(mLogBuffer.data());
		if (mLogAutoScroll) ImGui::SetScrollY(ImGui::GetScrollMaxY());
		ImGui::EndChildFrame();
	}
	ImGui::End();
}

void SettingsEditor::drawSettingFile(ds::cfg::Settings& eng, bool& isOpen) {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(eng.getName().data(), &isOpen, ImGuiWindowFlags_NoFocusOnAppearing)) {

		drawSaveButtons(eng);

		auto& searchText = mSearchMap[eng.getName()];
		auto& filterText = mFilterMap[eng.getName()];

		ImGui::NewLine();

		if (ImGui::BeginCombo("Filter", filterText.data())) {
			for (auto&& val :
				 {"", "string", "bool", "float", "double", "vec2", "vec3", "vec4", "rect", "color", "unknown"}) {
				if (ImGui::Selectable(val, val == filterText)) {
					filterText = val;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::InputText((eng.getName() + " Search").data(), &searchText);
		if (ImGui::Button("Clear Search + Filters")) {
			searchText.clear();
			filterText.clear();
		}

		ImGui::NewLine();

		if (filterText.empty()) {
			mSettingCounters.clear();
			eng.forEachSetting([this, &eng, &searchText](ds::cfg::Settings::Setting& setting) {
				auto count = eng.countSetting(setting.mName);
				if (count > 1) {
					auto findy = mSettingCounters.find(setting.mName);
					if (findy == mSettingCounters.end()) {
						mSettingCounters[setting.mName] = 0;
					}
				}
				drawSingleSetting(setting, eng, searchText, count > 1);
			});
		} else {
			mSettingCounters.clear();
			eng.forEachSetting(
				[this, &eng, &searchText](ds::cfg::Settings::Setting& setting) {
					auto count = eng.countSetting(setting.mName);
					if (count > 1) {
						auto findy = mSettingCounters.find(setting.mName);
						if (findy == mSettingCounters.end()) {
							mSettingCounters[setting.mName] = 0;
						}
					}

					drawSingleSetting(setting, eng, searchText, count > 1);
				},
				filterText);
		}
	}
	ImGui::End();
}

void SettingsEditor::drawSingleSetting(ds::cfg::Settings::Setting& setting, ds::cfg::Settings& allSettings,
									   const std::string& search, bool multiple) {
	const auto& name	 = setting.mName;
	std::string drawName = name;
	int			index	 = 0;
	if (multiple) {
		index = mSettingCounters[name];
		drawName += ":" + std::to_string(index);
		mSettingCounters[name] += 1;
	}

	bool noSearch = search.empty();
	bool showMe =
		noSearch || (name.find(search) != std::string::npos || setting.mRawValue.find(search) != std::string::npos);
	if (!showMe) {
		return;
	}

	if (!setting.mPossibleValues.empty()) {
		if (ImGui::BeginCombo(drawName.data(), setting.mRawValue.data())) {

			for (auto val : ds::split(setting.mPossibleValues, ",")) {
				if (ImGui::Selectable(val.data(), val == setting.mRawValue)) {
					setting.mRawValue = val;
				}
			}

			ImGui::EndCombo();
		}
	} else {
		if (setting.mType == ds::cfg::SETTING_TYPE_SECTION_HEADER) {
			ImGui::NewLine();
			ImGui::Separator();
			ImGui::Text(drawName.data());
			ImGui::Separator();
		} else if (setting.mType == ds::cfg::SETTING_TYPE_STRING) {
			ImGui::InputText(drawName.data(), &setting.mRawValue);
		} else if (setting.mType == ds::cfg::SETTING_TYPE_WSTRING) {
			ImGui::InputText(drawName.data(), &setting.mRawValue);
		} else if (setting.mType == ds::cfg::SETTING_TYPE_BOOL) {
			bool checked = setting.getBool();
			ImGui::Checkbox(drawName.data(), &checked);
			if (setting.getBool() != checked) {
				setting.mRawValue = ds::unparseBoolean(checked);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_INT) {
			int value = setting.getInt();
			if (ImGui::InputInt(drawName.data(), &value)) {
				setting.mRawValue = ds::value_to_string(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_FLOAT) {
			float value = setting.getFloat();
			if (ImGui::InputFloat(drawName.data(), &value)) {
				setting.mRawValue = ds::value_to_string(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_DOUBLE) {
			double value = setting.getDouble();
			if (ImGui::InputDouble(drawName.data(), &value)) {
				setting.mRawValue = ds::value_to_string(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_VEC2) {
			ci::vec2 value = setting.getVec2();
			if (ImGui::DragFloat2(drawName.data(), &value)) {
				setting.mRawValue = ds::unparseVector(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_VEC3) {
			ci::vec3 value = setting.getVec3();
			if (ImGui::DragFloat3(drawName.data(), &value)) {
				setting.mRawValue = ds::unparseVector(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_RECT) {
			ci::Rectf value	  = setting.getRect();
			ci::vec4  rectish = ci::vec4(value.getUpperLeft(), value.getLowerRight() - value.getUpperLeft());

			if (ImGui::DragFloat4(drawName.data(), &rectish)) {
				setting.mRawValue =
					ds::unparseRect(ci::Rectf(rectish.x, rectish.y, rectish.x + rectish.z, rectish.y + rectish.w));
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_UNKNOWN) {
			ImGui::InputText(drawName.data(), &setting.mRawValue);
		} else if (setting.mType == ds::cfg::SETTING_TYPE_COLOR) {
			ci::ColorA color	 = ds::parseColor(setting.mRawValue, mEngine);
			ci::ColorA colorOrig = color;
			if (ImGui::ColorEdit4(drawName.data(), &color, ImGuiColorEditFlags_NoInputs)) {
				DS_LOG_INFO("COLOR CHANGED");
				DS_LOG_INFO("OG:  " << colorOrig << " - " << ds::unparseColor(colorOrig));
				DS_LOG_INFO("NEW: " << color << " - " << ds::unparseColor(color));
				setting.mRawValue = ds::unparseColor(color, mEngine);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_TEXT_STYLE) {
			bool			  changed = false;
			ds::ui::TextStyle style	  = ds::ui::TextStyle::textStyleFromSetting(mEngine, setting.mRawValue);
			if (ImGui::BeginCombo((std::string(drawName) + " Font").data(), style.mFont.data())) {
				mEngine.getEngineCfg().getSettings("fonts").forEachSetting(
					[&changed, &style](ds::cfg::Settings::Setting& setting) {
						if (ImGui::Selectable(setting.mName.data())) {
							style.mFont = setting.mName;
							changed		= true;
						}
					});
				ImGui::EndCombo();
			}
			if (ImGui::InputDouble((std::string(drawName) + " Size").data(), &style.mSize)) {
				changed = true;
			}
			if (ImGui::InputDouble((std::string(drawName) + " Leading").data(), &style.mLeading)) {
				changed = true;
			}
			if (ImGui::InputDouble((std::string(drawName) + " Letter Spacing").data(), &style.mLetterSpacing)) {
				changed = true;
			}
			if (ImGui::BeginCombo((std::string(drawName) + " Alignment").data(),
								  ds::ui::Alignment::toString(style.mAlignment).data())) {
				if (ImGui::Selectable("Left", style.mAlignment == ds::ui::Alignment::kLeft)) {
					style.mAlignment = ds::ui::Alignment::kLeft;
					changed			 = true;
				}

				if (ImGui::Selectable("Right", style.mAlignment == ds::ui::Alignment::kRight)) {
					style.mAlignment = ds::ui::Alignment::kRight;
					changed			 = true;
				}

				if (ImGui::Selectable("Center", style.mAlignment == ds::ui::Alignment::kCenter)) {
					style.mAlignment = ds::ui::Alignment::kCenter;
					changed			 = true;
				}

				ImGui::EndCombo();
			}
			if (ImGui::BeginCombo((std::string(drawName) + " Color").data(), style.mColorName.data())) {
				mEngine.getSettings("styles").forEachSetting(
					[this, &changed, &style](ds::cfg::Settings::Setting& setting) {
						if (ImGui::Selectable(setting.mName.data(), setting.mName == style.mColorName)) {
							style.mColorName = setting.mName;
							style.mColor	 = mEngine.getColors().getColorFromName(setting.mName);
							changed			 = true;
						}
					},
					"color");
				ImGui::EndCombo();
			}
			ImGui::NewLine();
			if (changed) {
				DS_LOG_INFO("Text StyleSetting Changed!");
				DS_LOG_INFO(style.settingFromTextStyle(mEngine, style));
				setting.mRawValue = style.settingFromTextStyle(mEngine, style);
			}
		}
	}

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNormal |
							 ImGuiHoveredFlags_NoSharedDelay)) {
		std::string tooltip;

		bool needsNewline = false;
		if (!setting.mComment.empty()) {
			tooltip += "Description\n\t" + setting.mComment;
			needsNewline = true;
		}

		if (setting.mSource.empty()) {
			if (needsNewline) tooltip += "\n\n";

			tooltip += "*Source\n\tEdited";
			needsNewline = true;
		} else {
			if (needsNewline) tooltip += "\n\n";

			tooltip += "Source\n\t" + setting.mSource;
			needsNewline = true;
		}

		if (!tooltip.empty()) ImGui::SetTooltip(tooltip.data());
	}


	// Update the ACTUAL setting, since for some reason the one we have is from a different list (eyeroll)
	auto& theSetting = allSettings.getSetting(name, index);
	if (theSetting.mRawValue != setting.mRawValue) {
		theSetting.mRawValue = setting.mRawValue;
		theSetting.mSource.clear();
	}
}

void SettingsEditor::drawSaveButtons(ds::cfg::Settings& toSave) {
	std::string appDir		= ds::Environment::expand("%APP%/settings/");
	std::string localDir	= ds::Environment::expand("%LOCAL%/%PP%/");
	std::string appCfgDir	= ds::Environment::expand("%APP%/settings/%CFG_FOLDER%/");
	std::string localCfgDir = ds::Environment::expand("%LOCAL%/settings/%PP%/%CFG_FOLDER%/");

	ImGui::Separator();
	if (ImGui::Button(appDir.data())) {
		saveChange(appDir, toSave);
	}

	if (ImGui::Button(localDir.data())) {
		saveChange(localDir, toSave);
	}

	if (ImGui::Button(appCfgDir.data())) {
		saveChange(appCfgDir, toSave);
	}

	if (ImGui::Button(localCfgDir.data())) {
		saveChange(localCfgDir, toSave);
	}
}

void SettingsEditor::saveChange(const std::string& path, ds::cfg::Settings& toSave) {

	auto savePath = std::string(path) + toSave.getName() + ".xml";
	toSave.writeTo(savePath);
}

void SettingsEditor::showSettings(const std::string theSettingsName) {
	if (!mOpen) {
		show();
		mOpen = true;
	}

	if (theSettingsName == "stats") {
		mAppStatusOpen = true;
	}
}

void SettingsEditor::hideSettings() {
	if (mOpen) {
		if (mSrcDestSaved) {
			auto& eng		   = ((ds::Engine&)(mEngine));
			eng.mData.mSrcRect = mOrigSrc;
			eng.markCameraDirty();
		}

		hide();
		mOpen = false;
	}
}

} // namespace ds::cfg
