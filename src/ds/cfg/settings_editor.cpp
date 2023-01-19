#include "stdafx.h"

#include "settings_editor.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

#include <cinder/CinderImGui.h>
#include <cinder/Clipboard.h>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include <Poco/DateTimeFormatter.h>

#include <pango/pango-fontmap.h>

#include <ds/app/blob_reader.h>
#include <ds/app/blob_registry.h>
#include <ds/app/engine/engine_data.h>
#include <ds/cfg/editor_components/edit_view.h>
#include <ds/cfg/editor_components/editor_item.h>
#include <ds/cfg/settings.h>
#include <ds/cfg/settings_variables.h>
#include <ds/content/content_events.h>
#include <ds/debug/computer_info.h>
#include <ds/debug/logger.h>
#include <ds/math/math_defs.h>
#include <ds/util/color_util.h>

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

	// Set mLastSync years in the past
	mLastSync.assign(2000, 1, 1);

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
	}
}

void SettingsEditor::drawMenu() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Status")) {
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
			ImGui::EndMenu();
		}


		ImGui::EndMainMenuBar();
	}
}

void SettingsEditor::drawSettings() {

	if (mEngineOpen) drawSettingFile(mEngine.getEngineCfg().getSettings("engine"));
	if (mAppSettingsOpen) drawSettingFile(mEngine.getEngineCfg().getSettings("app_settings"));

	if (mStylesOpen) {
		if (!mEngine.getEngineCfg().getSettings("styles").empty()) {
			drawSettingFile(mEngine.getEngineCfg().getSettings("styles"));
		} else {
			drawSettingFile(mEngine.getEngineCfg().getSettings("text"));
			drawSettingFile(mEngine.getEngineCfg().getSettings("colors"));
		}
	}

	if (mFontsOpen) drawSettingFile(mEngine.getEngineCfg().getSettings("fonts"));

	if (!mEngine.getEngineCfg().getSettings("tuio_inputs").empty() && mTuioOpen) {
		drawSettingFile(mEngine.getEngineCfg().getSettings("tuio_inputs"));
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
	if (ImGui::Begin("Content", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
		auto& allContent = mEngine.mContent;
		drawTree(allContent);
	}
	ImGui::End();
}

void SettingsEditor::drawAppStatus() {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Status", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
		auto& eng = ((ds::Engine&)(mEngine));
		auto  sz  = eng.mSprites.size();
		ImGui::LabelText("Sprite Count", "%i", int(sz));

		ImGui::LabelText("Touch Mode", ds::ui::TouchMode::toString(eng.mTouchMode).data());

		ImGui::LabelText("Physical Memory", "%f", mEngine.getComputerInfo().getPhysicalMemoryUsedByProcess());
		ImGui::LabelText("Virtual Memory", "%f", mEngine.getComputerInfo().getVirtualMemoryUsedByProcess());

		if (mEngine.getMode() != ds::ui::SpriteEngine::STANDALONE_MODE) {
			ImGui::LabelText("Bytes Received", "%i", mEngine.getBytesRecieved());
			ImGui::LabelText("Bytes Sent", "%i", mEngine.getBytesSent());
		}

		float fpsy = eng.getAverageFps();
		ImGui::LabelText("FPS", "%f", fpsy);
		// DsNode / Downsync status
		// CPU Usage
		// GPU Usage
	}
	ImGui::End();
}

void SettingsEditor::drawAppHostStatus() {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("AppHost", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
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
	if (ImGui::Begin("Sync Status", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
		if (mLastSync.year() < Poco::DateTime().year()) {
			ImGui::Text("Sync incomplete or not running");
		} else {
			auto diff = Poco::DateTime() - mLastSync;

			if (diff.minutes() < 1) {
				ImGui::LabelText("Last Sync", "%i seconds ago", diff.seconds());
			} else if (diff.hours() < 1) {
				ImGui::LabelText("Last Sync", "%i minutes ago", diff.minutes());
			} else {
				ImGui::LabelText("Last Sync", "%i hours ago", diff.hours());
			}
		}
	}
	ImGui::End();
}

void SettingsEditor::drawShortcuts() {

	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Shortcuts", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
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
	if (ImGui::Begin("Log", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {
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

void SettingsEditor::drawSettingFile(ds::cfg::Settings& eng) {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(eng.getName().data(), NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {

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
			eng.forEachSetting([this, &eng, &searchText](ds::cfg::Settings::Setting& setting) {
				drawSingleSetting(setting, eng, searchText);
			});
		} else {
			eng.forEachSetting(
				[this, &eng, &searchText](ds::cfg::Settings::Setting& setting) {
					drawSingleSetting(setting, eng, searchText);
				},
				filterText);
		}
	}
	ImGui::End();
}

void SettingsEditor::drawSingleSetting(ds::cfg::Settings::Setting& setting, ds::cfg::Settings& allSettings,
									   const std::string& search) {
	const auto& name = setting.mName;

	bool noSearch = search.empty();
	bool showMe =
		noSearch || (name.find(search) != std::string::npos || setting.mRawValue.find(search) != std::string::npos);
	if (!showMe) {
		return;
	}

	if (!setting.mPossibleValues.empty()) {
		if (ImGui::BeginCombo(name.data(), setting.mRawValue.data())) {

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
			ImGui::Text(name.data());
			ImGui::Separator();
		} else if (setting.mType == ds::cfg::SETTING_TYPE_STRING) {
			ImGui::InputText(name.data(), &setting.mRawValue);
		} else if (setting.mType == ds::cfg::SETTING_TYPE_WSTRING) {
			ImGui::InputText(name.data(), &setting.mRawValue);
		} else if (setting.mType == ds::cfg::SETTING_TYPE_BOOL) {
			bool checked = setting.getBool();
			ImGui::Checkbox(name.data(), &checked);
			if (setting.getBool() != checked) {
				setting.mRawValue = ds::unparseBoolean(checked);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_INT) {
			int value = setting.getInt();
			if (ImGui::InputInt(name.data(), &value)) {
				setting.mRawValue = ds::value_to_string(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_FLOAT) {
			float value = setting.getFloat();
			if (ImGui::InputFloat(name.data(), &value)) {
				setting.mRawValue = ds::value_to_string(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_DOUBLE) {
			double value = setting.getDouble();
			if (ImGui::InputDouble(name.data(), &value)) {
				setting.mRawValue = ds::value_to_string(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_VEC2) {
			ci::vec2 value = setting.getVec2();
			if (ImGui::DragFloat2(name.data(), &value)) {
				setting.mRawValue = ds::unparseVector(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_VEC3) {
			ci::vec3 value = setting.getVec3();
			if (ImGui::DragFloat3(name.data(), &value)) {
				setting.mRawValue = ds::unparseVector(value);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_RECT) {
			ci::Rectf value	  = setting.getRect();
			ci::vec4  rectish = ci::vec4(value.getUpperLeft(), value.getLowerRight() - value.getUpperLeft());

			if (ImGui::DragFloat4(name.data(), &rectish)) {
				setting.mRawValue =
					ds::unparseRect(ci::Rectf(rectish.x, rectish.y, rectish.x + rectish.z, rectish.y + rectish.w));
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_UNKNOWN) {
			ImGui::InputText(name.data(), &setting.mRawValue);
		} else if (setting.mType == ds::cfg::SETTING_TYPE_COLOR) {
			ci::ColorA color	 = ds::parseColor(setting.mRawValue, mEngine);
			ci::ColorA colorOrig = color;
			if (ImGui::ColorEdit4(name.data(), &color, ImGuiColorEditFlags_NoInputs)) {
				DS_LOG_INFO("COLOR CHANGED");
				DS_LOG_INFO("OG:  " << colorOrig << " - " << ds::unparseColor(colorOrig));
				DS_LOG_INFO("NEW: " << color << " - " << ds::unparseColor(color));
				setting.mRawValue = ds::unparseColor(color, mEngine);
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_TEXT_STYLE) {
			bool			  changed = false;
			ds::ui::TextStyle style	  = ds::ui::TextStyle::textStyleFromSetting(mEngine, setting.mRawValue);
			if (ImGui::BeginCombo((std::string(name) + " Font").data(), style.mFont.data())) {
				mEngine.getEngineCfg().getSettings("fonts").forEachSetting(
					[&changed, &style](ds::cfg::Settings::Setting& setting) {
						if (ImGui::Selectable(setting.mName.data())) {
							style.mFont = setting.mName;
							changed		= true;
						}
					});
				ImGui::EndCombo();
			}
			if (ImGui::InputDouble((std::string(name) + " Size").data(), &style.mSize)) {
				changed = true;
			}
			if (ImGui::InputDouble((std::string(name) + " Leading").data(), &style.mLeading)) {
				changed = true;
			}
			if (ImGui::InputDouble((std::string(name) + " Letter Spacing").data(), &style.mLetterSpacing)) {
				changed = true;
			}
			if (ImGui::BeginCombo((std::string(name) + " Alignment").data(),
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
			if (ImGui::BeginCombo((std::string(name) + " Color").data(), style.mColorName.data())) {
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
	auto& theSetting = allSettings.getSetting(name, 0);
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
	if(!mOpen){
		show();
		mOpen = true;
	}
}

void SettingsEditor::hideSettings() {
	if(mOpen){
		hide();
		mOpen = false;
	}
}

} // namespace ds::cfg
