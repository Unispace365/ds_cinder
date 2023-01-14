#include "stdafx.h"

#include "settings_editor.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

#include <cinder/CinderImGui.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <pango/pango-fontmap.h>

#include <ds/app/blob_reader.h>
#include <ds/app/blob_registry.h>
#include <ds/cfg/editor_components/edit_view.h>
#include <ds/cfg/editor_components/editor_item.h>
#include <ds/cfg/settings.h>
#include <ds/cfg/settings_variables.h>
#include <ds/math/math_defs.h>
#include <ds/util/color_util.h>


namespace ds::cfg {

SettingsEditor::SettingsEditor(ds::ui::SpriteEngine& e)
  : ds::ui::Sprite(e) {

	hide();
	setTransparent(false);
}


void SettingsEditor::drawPostLocalClient() {
	if (mSettingsOpen && visible()) {
		drawSettings();
	}
}

void SettingsEditor::drawSettings() {
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoFocusOnAppearing)) {

		ImGui::BeginTabBar("Settings Tabs");
		drawSettingFile(mEngine.getEngineCfg().getSettings("engine"));
		drawSettingFile(mEngine.getEngineCfg().getSettings("app_settings"));

		if (!mEngine.getEngineCfg().getSettings("styles").empty()) {
			drawSettingFile(mEngine.getEngineCfg().getSettings("styles"));
		} else {
			drawSettingFile(mEngine.getEngineCfg().getSettings("text"));
			drawSettingFile(mEngine.getEngineCfg().getSettings("colors"));
		}

		drawSettingFile(mEngine.getEngineCfg().getSettings("fonts"));

		if (!mEngine.getEngineCfg().getSettings("tuio_inputs").empty()) {
			drawSettingFile(mEngine.getEngineCfg().getSettings("tuio_inputs"));
		}

		ImGui::EndTabBar();
	}
	ImGui::End();
}

void SettingsEditor::drawSettingFile(ds::cfg::Settings& eng) {
	if (ImGui::BeginTabItem(eng.getName().data())) {
		drawSaveButtons(eng);
		ImGui::NewLine();

		eng.forEachSetting([this, &eng](ds::cfg::Settings::Setting& setting) { drawSingleSetting(setting, eng); });

		ImGui::EndTabItem();
	}
}

void SettingsEditor::drawSingleSetting(ds::cfg::Settings::Setting& setting, ds::cfg::Settings& allSettings) {
	const auto& name = setting.mName;

	auto standardText = [&] {
		std::string buf = setting.mRawValue;
		if (ImGui::InputText(name.data(), &buf)) {
			setting.mRawValue = buf;
		}
	};

	if (!setting.mOriginalValue.empty()) {
		ImGui::BeginDisabled(true);
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
			standardText();
		} else if (setting.mType == ds::cfg::SETTING_TYPE_WSTRING) {
			standardText();
		} else if (setting.mType == ds::cfg::SETTING_TYPE_BOOL) {
			bool checked = setting.getBool();
			if (ImGui::Checkbox(name.data(), &checked)) {
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
			// ImGui::Text(name.data());
			ci::Rectf value	  = setting.getRect();
			ci::vec4  rectish = ci::vec4(value.getUpperLeft(), value.getLowerRight() - value.getUpperLeft());

			if (ImGui::DragFloat4(name.data(), &rectish)) {
				setting.mRawValue =
					ds::unparseRect(ci::Rectf(rectish.x, rectish.y, rectish.x + rectish.z, rectish.y + rectish.w));
			}
		} else if (setting.mType == ds::cfg::SETTING_TYPE_UNKNOWN) {
			standardText();
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
			// standardText();
		}
	}

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay)) {
		std::string tooltip;

		bool needsNewline = false;
		if (!setting.mComment.empty()) {
			tooltip += "Description\n\t" + setting.mComment;
			needsNewline = true;
		}

		if(setting.mSource.empty()){
			if(needsNewline) tooltip += "\n\n";

			tooltip += "*Source\n\tEdited";
			needsNewline = true;
		}else{
			if(needsNewline) tooltip += "\n\n";

			tooltip += "Source\n\t" + setting.mSource;
			needsNewline = true;

		}

		if(!tooltip.empty()) ImGui::SetTooltip(tooltip.data());
	}


	if (!setting.mOriginalValue.empty()) {
		ImGui::EndDisabled();
	}

	// Update the ACTUAL setting, since for some reason the one we have is from a different list (eyeroll)
	auto& theSetting = allSettings.getSetting(name, 0);
	if (theSetting.mRawValue != setting.mRawValue) {
		theSetting.mRawValue = setting.mRawValue;
		theSetting.mSource.clear();
	}
}

void SettingsEditor::drawSaveButtons(ds::cfg::Settings& toSave) {
	ImGui::Separator();
	if (ImGui::Button("Save to %APP%/settings/")) {
		saveChange(ds::Environment::expand("%APP%/settings/"), toSave);
	}

	if (ImGui::Button("Save to %LOCAL%/%PP%/")) {
		saveChange(ds::Environment::expand("%LOCAL%/%PP%/"), toSave);
	}

	if (ImGui::Button("Save to %APP%/settings/%CFG_FOLDER%/")) {
		saveChange(ds::Environment::expand("%APP%/settings/%CFG_FOLDER%/"), toSave);
	}

	if (ImGui::Button("Save to %LOCAL%/settings/%PP%/%CFG_FOLDER%/")) {
		saveChange(ds::Environment::expand("%LOCAL%/settings/%PP%/%CFG_FOLDER%/"), toSave);
	}
}

void SettingsEditor::saveChange(const std::string& path, ds::cfg::Settings& toSave) {

	auto savePath = std::string(path) + toSave.getName() + ".xml";
	toSave.writeTo(savePath);
}

void SettingsEditor::showSettings(const std::string theSettingsName) {

	show();
	mSettingsOpen = true;
}

void SettingsEditor::hideSettings() {
	hide();
	mSettingsOpen = false;
}

} // namespace ds::cfg
