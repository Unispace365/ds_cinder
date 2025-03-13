#include "stdafx.h"

#include <cinder/Xml.h>

#include "ds/cfg/settings.h"
#include "ds/cfg/settings_variables.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/util/color_util.h"
#include "ds/util/file_meta_data.h"
#include "ds/util/string_util.h"

#include <Poco/File.h>
#include <Poco/String.h>


namespace ds::cfg {

const std::string& SETTING_TYPE_UNKNOWN		   = "unknown";
const std::string& SETTING_TYPE_BOOL		   = "bool";
const std::string& SETTING_TYPE_INT			   = "int";
const std::string& SETTING_TYPE_FLOAT		   = "float";
const std::string& SETTING_TYPE_DOUBLE		   = "double";
const std::string& SETTING_TYPE_STRING		   = "string";
const std::string& SETTING_TYPE_WSTRING		   = "wstring";
const std::string& SETTING_TYPE_COLOR		   = "color";
const std::string& SETTING_TYPE_COLORA		   = "colora";
const std::string& SETTING_TYPE_VEC2		   = "vec2";
const std::string& SETTING_TYPE_VEC3		   = "vec3";
const std::string& SETTING_TYPE_RECT		   = "rect";
const std::string& SETTING_TYPE_SECTION_HEADER = "section_header";
const std::string& SETTING_TYPE_TEXT_STYLE	   = "text_style";

const std::vector<std::string> WELL_KNOWN_ATTRIBUTES = {"name",		 "target",	  "value", "comment",	"default",
														"min_value", "max_value", "type",  "possibles", "multiplier"};

namespace {

	static const int SETTINGS_INCREMENT = 200;

	static std::vector<std::string> SETTING_TYPES;

	void initialize_types() {
		if (SETTING_TYPES.empty()) {
			SETTING_TYPES.emplace_back(SETTING_TYPE_UNKNOWN);
			SETTING_TYPES.emplace_back(SETTING_TYPE_BOOL);
			SETTING_TYPES.emplace_back(SETTING_TYPE_INT);
			SETTING_TYPES.emplace_back(SETTING_TYPE_FLOAT);
			SETTING_TYPES.emplace_back(SETTING_TYPE_DOUBLE);
			SETTING_TYPES.emplace_back(SETTING_TYPE_STRING);
			SETTING_TYPES.emplace_back(SETTING_TYPE_WSTRING);
			SETTING_TYPES.emplace_back(SETTING_TYPE_COLOR);
			SETTING_TYPES.emplace_back(SETTING_TYPE_COLORA);
			SETTING_TYPES.emplace_back(SETTING_TYPE_VEC2);
			SETTING_TYPES.emplace_back(SETTING_TYPE_VEC3);
			SETTING_TYPES.emplace_back(SETTING_TYPE_RECT);
			SETTING_TYPES.emplace_back(SETTING_TYPE_SECTION_HEADER);
			SETTING_TYPES.emplace_back(SETTING_TYPE_TEXT_STYLE);
		}
	}

	static void merge_settings(std::vector<std::pair<std::string, std::vector<Settings::Setting>>>&		  dst,
							   const std::vector<std::pair<std::string, std::vector<Settings::Setting>>>& src) {

		int highestReadIndex = 0;

		for (auto& dit : dst) {
			for (const auto& vit : dit.second) {
				if (vit.mReadIndex > highestReadIndex) highestReadIndex = vit.mReadIndex;
			}
		}

		highestReadIndex += SETTINGS_INCREMENT;

		for (auto sit : src) {
			bool found = false;
			for (auto& dit : dst) {
				if (dit.first == sit.first) {

					int thisReadIndex = 0;
					for (int i = 0; i < sit.second.size(); i++) {
						if (i < dit.second.size()) {
							bool sourceEmpty  = dit.second[i].mSource.empty();
							bool valueChanged = dit.second[i].mRawValue != sit.second[i].mRawValue;
							bool isChange	  = sourceEmpty || valueChanged;

							if (isChange) {
								if (!sourceEmpty) {
									int readIndex			 = dit.second[i].mReadIndex;
									sit.second[i].mReadIndex = readIndex;
									if (readIndex > thisReadIndex) thisReadIndex = readIndex;
								}

								if (sit.second[i].mComment.empty() && !dit.second[i].mComment.empty()) {
									sit.second[i].mComment = dit.second[i].mComment;
								}
								if (sit.second[i].mType == "unknown") {
									sit.second[i].mType = dit.second[i].mType;
								}
								if (sit.second[i].mDefault.empty()) {
									sit.second[i].mDefault = dit.second[i].mDefault;
								}
								if (sit.second[i].mMinValue.empty()) {
									sit.second[i].mMinValue = dit.second[i].mMinValue;
								}
								if (sit.second[i].mMaxValue.empty()) {
									sit.second[i].mMaxValue = dit.second[i].mMaxValue;
								}
								if (sit.second[i].mPossibleValues.empty()) {
									sit.second[i].mPossibleValues = dit.second[i].mPossibleValues;
								}
								if (sit.second[i].mMultiplier.empty()) {
									sit.second[i].mMultiplier = dit.second[i].mMultiplier;
								}

								dit.second[i] = sit.second[i];
							}

						} else {
							dit.second.emplace_back(sit.second[i]);
							if (thisReadIndex > 0) {
								dit.second.back().mReadIndex = thisReadIndex;
								thisReadIndex += 1;
							}
						}
					}


					found = true;
					break;
				}
			}

			if (!found) {
				for (auto it : sit.second) {
					it.mReadIndex = highestReadIndex;
					highestReadIndex += SETTINGS_INCREMENT;
				}

				dst.emplace_back(sit);
			}
		}
	}

} // namespace

bool Settings::Setting::getBool() const {
	return parseBoolean(mRawValue);
}

int Settings::Setting::getInt() const {
	return string_to_int(mRawValue);
}

float Settings::Setting::getFloat() const {
	return string_to_float(mRawValue);
}

double Settings::Setting::getDouble() const {
	return string_to_double(mRawValue);
}

ci::Color Settings::Setting::getColor(const ui::SpriteEngine& eng) const {
	return parseColor(mRawValue, eng);
}

ci::ColorA Settings::Setting::getColorA(const ui::SpriteEngine& eng) const {
	return parseColor(mRawValue, eng);
}

const std::string& Settings::Setting::getString() const {
	return mRawValue;
}

std::wstring Settings::Setting::getWString() const {
	return wstr_from_utf8(mRawValue);
}

ci::vec2 Settings::Setting::getVec2() const {
	return {parseVector(mRawValue)};
}

ci::vec3 Settings::Setting::getVec3() const {
	return parseVector(mRawValue);
}

cinder::Rectf Settings::Setting::getRect() const {
	return parseRect(mRawValue);
}

std::vector<std::string> Settings::Setting::getPossibleValues() const {
	std::vector<std::string> possibles = split(mPossibleValues, ", ", true);
	return possibles;
}

const std::map<std::string, std::string>& Settings::Setting::getAttributes() const {
	return mAttributeMap;
}

std::map<std::string, std::string> Settings::Setting::getExtraAttributes() const {
	std::map<std::string, std::string> extras;
	for (const auto& attr : mAttributeMap) {
		if (std::find(WELL_KNOWN_ATTRIBUTES.begin(), WELL_KNOWN_ATTRIBUTES.end(), attr.first) ==
			std::end(WELL_KNOWN_ATTRIBUTES)) {
			extras[attr.first] = attr.second;
		}
	}
	return extras;
}

bool Settings::Setting::hasAttribute(const std::string& key) const {
	return mAttributeMap.find(key) != mAttributeMap.end();
}

const std::string& Settings::Setting::getAttribute(const std::string& key, const std::string& defaultValue) const {
	std::string value = defaultValue;
	return hasAttribute(key) ? mAttributeMap.at(key) : defaultValue;
}

void Settings::Setting::replaceSettingVariablesAndExpressions() {

	std::string value = SettingsVariables::replaceVariables(mRawValue);
	value			  = SettingsVariables::parseAllExpressions(value);
	if (!mMultiplier.empty()) {
		value = SettingsVariables::doMultiply(value, mMultiplier, mType);
	}

	if (value != mRawValue) {
		// save the originalRawValue
		mOriginalValue = mRawValue;
		mRawValue	   = value;
	} else if (!mOriginalValue.empty()) {
		value = SettingsVariables::replaceVariables(mOriginalValue);
		value = SettingsVariables::parseAllExpressions(value);
		if (!mMultiplier.empty()) {
			value = SettingsVariables::doMultiply(value, mMultiplier, mType);
		}
		mRawValue = value;
	}

	for (const auto& attr : mAttributeMap) {
		if (mOriginalAttributeMap.find(attr.first) == mOriginalAttributeMap.end()) {
			mOriginalAttributeMap[attr.first] = attr.second;
		}
		value = attr.second;
		value = SettingsVariables::replaceVariables(value);
		value = SettingsVariables::parseAllExpressions(value);
		if (!mMultiplier.empty()) {
			value = SettingsVariables::doMultiply(value, mMultiplier, mType);
		}
		mAttributeMap[attr.first] = value;
	}
}

Settings::Settings()
  : mReadIndex(1) {
	initialize_types();
}

void Settings::mergeSettings(const Settings& mergeIn) {
	merge_settings(mSettings, mergeIn.mSettings);
}

void Settings::applyMultiplier() {
	for (auto& dit : mSettings) {
		for (auto& sit : dit.second) {
			if (!sit.mMultiplier.empty()) {
				auto multiplierKey = sit.mMultiplier;

				// if the multiplier key starts with an !, the we inverse the multiplier
				if (multiplierKey[0] == '!') {
					multiplierKey	 = multiplierKey.substr(1);
					float multiplier = getFloat(multiplierKey);
					if (multiplier != 0.0f) {
						if (sit.mType == SETTING_TYPE_INT) {
							int value = string_to_int(sit.mRawValue);
							value /= multiplier;
							sit.mRawValue = std::to_string(value);
						} else if (sit.mType == SETTING_TYPE_FLOAT) {
							float value = string_to_float(sit.mRawValue);
							value /= multiplier;
							sit.mRawValue = std::to_string(value);
						} else if (sit.mType == SETTING_TYPE_DOUBLE) {
							double value = string_to_double(sit.mRawValue);
							value /= multiplier;
							sit.mRawValue = std::to_string(value);
						}
					}
				} else {
					float multiplier = getFloat(multiplierKey);
					if (multiplier != 0.0f) {
						if (sit.mType == SETTING_TYPE_INT) {
							int value = string_to_int(sit.mRawValue);
							value *= multiplier;
							sit.mRawValue = std::to_string(value);
						} else if (sit.mType == SETTING_TYPE_FLOAT) {
							float value = string_to_float(sit.mRawValue);
							value *= multiplier;
							sit.mRawValue = std::to_string(value);
						} else if (sit.mType == SETTING_TYPE_DOUBLE) {
							double value = string_to_double(sit.mRawValue);
							value *= multiplier;
							sit.mRawValue = std::to_string(value);
						}
					}
				}
			}
		}
	}
}

void Settings::readFrom(const std::string& fullFilePath, bool append) {
	if (!append) {
		directReadFrom(fullFilePath, true);
		return;
	}

	Settings s;
	s.directReadFrom(fullFilePath, false);

	merge_settings(mSettings, s.mSettings);
}

void Settings::readFrom(ci::XmlTree& tree, const std::string& fullFilePath, bool append,
						const ui::SpriteEngine* engine) {
	if (!append) {
		directReadFromXml(tree, fullFilePath, true, engine);
		return;
	}

	Settings s;
	s.directReadFromXml(tree, fullFilePath, false, engine);

	merge_settings(mSettings, s.mSettings);
}

void Settings::directReadFrom(const std::string& filename, bool clearAll) {
	if (clearAll) clear();

	if (filename.empty()) {
		return;
	}

	if (!safeFileExistsCheck(filename, false)) {
		return;
	}


	ci::XmlTree xml;
	try {
		xml = ci::XmlTree(ci::loadFile(filename));
	} catch (std::exception& e) {
		DS_LOG_WARNING("Exception loading settings from " << filename << " " << e.what());
		return;
	}

	directReadFromXml(xml, filename, clearAll);
}

void Settings::directReadFromXml(ci::XmlTree& tree, const std::string& referenceFilename, bool clearAll,
								 const ui::SpriteEngine* engine) {
	if (clearAll) clear();

	ci::XmlTree& xml = tree;

	auto xmlEnd = xml.end();
	for (auto it = xml.begin("settings/setting"); it != xmlEnd; ++it) {
		if (!it->hasAttribute("name")) {
			DS_LOG_WARNING("Missing a name attribute for a setting!");
			continue;
		}


		if (engine) {
			if (it->hasAttribute("target")) {
				auto target = it->getAttributeValue<std::string>("target");
				if (!engine->hasLayoutTarget(target)) {
					continue;
				}
			}
		}

		auto theName = it->getAttributeValue<std::string>("name");

		Setting theSetting;
		theSetting.mName	  = theName;
		theSetting.mReadIndex = mReadIndex;
		mReadIndex += SETTINGS_INCREMENT; // leave space for other settings to be inserted
		if (it->hasAttribute("value")) theSetting.mRawValue = it->getAttributeValue<std::string>("value");
		if (it->hasAttribute("comment")) theSetting.mComment = it->getAttributeValue<std::string>("comment");
		if (it->hasAttribute("default")) theSetting.mDefault = it->getAttributeValue<std::string>("default");
		if (it->hasAttribute("min_value")) theSetting.mMinValue = it->getAttributeValue<std::string>("min_value");
		if (it->hasAttribute("max_value")) theSetting.mMaxValue = it->getAttributeValue<std::string>("max_value");
		if (it->hasAttribute("type")) theSetting.mType = it->getAttributeValue<std::string>("type");
		if (it->hasAttribute("possibles")) theSetting.mPossibleValues = it->getAttributeValue<std::string>("possibles");
		if (it->hasAttribute("multiplier")) theSetting.mMultiplier = it->getAttributeValue<std::string>("multiplier");

		if (!validateType(theSetting.mType)) {
			DS_LOG_WARNING("Unknown setting type for " << theName << " type:" << theSetting.mType
													   << " source: " << referenceFilename);
		}


		for (auto& attr : it->getAttributes()) {
			if (std::find(WELL_KNOWN_ATTRIBUTES.begin(), WELL_KNOWN_ATTRIBUTES.end(), attr.getName()) ==
				std::end(WELL_KNOWN_ATTRIBUTES)) {
				theSetting.mHasExtraAttributes = true;
			}
			theSetting.mAttributeMap[attr.getName()] = attr.getValue();
		}

		theSetting.mSource = referenceFilename;

		auto settingIndex = getSettingIndex(theName);
		if (settingIndex > -1 && !mSettings.empty()) {
			mSettings[settingIndex].second.push_back(theSetting);
		} else {
			std::vector<Setting> newSettingVec;
			newSettingVec.push_back(theSetting);
			mSettings.emplace_back(std::pair(theName, newSettingVec));
		}
	}
}


namespace {
	// Locations ordered by which overrides which (high number = higher priority)
	// 'USER' is for settings changed by the user while the application is running
	enum SettingLocation { APP = 1, LOCAL = 2, APP_CFG = 3, LOCAL_CFG = 4, USER = 5 };

	SettingLocation getLocationType(const std::string& filename) {
		if (filename.empty()) return USER;

		const auto appDir			= Environment::expand("%APP%/settings");
		const auto localDir			= Environment::expand("%LOCAL%/settings/%PP%");
		const auto appOverrideDir	= Environment::expand("%APP%/settings/%CFG_FOLDER%");
		const auto localOverrideDir = Environment::expand("%LOCAL%/settings/%PP%/%CFG_FOLDER%");

		if (filename.find(localOverrideDir) != std::string::npos) return LOCAL_CFG;
		if (filename.find(appOverrideDir) != std::string::npos) return APP_CFG;
		if (filename.find(localDir) != std::string::npos) return LOCAL;
		if (filename.find(appDir) != std::string::npos) return APP;

		return USER;
	}
} // namespace

void Settings::writeTo(const std::string& fullFilePath) {
	auto targetLocation = getLocationType(fullFilePath);

	ci::XmlTree xml = ci::XmlTree::createDoc();
	ci::XmlTree rootNode;
	rootNode.setTag("settings");
	auto& sortedSettings = getReadSortedSettings();
	for (const auto& sit : sortedSettings) {
		// Determine if this setting is actually an override or not
		auto sourceLocation = getLocationType(sit.mSource);
		// Never skip section headers, but skip non-overrides
		bool skip = !(sit.mType == SETTING_TYPE_SECTION_HEADER) && (sourceLocation < targetLocation);
		if (skip) {
			continue;
		} else {
			DS_LOG_INFO("Writing setting" << sit.mName << " as it overrides a prior value");
		}

		ci::XmlTree settingNode;
		settingNode.setTag("setting");
		settingNode.setAttribute("name", sit.mName);
		settingNode.setAttribute("value", sit.mOriginalValue.empty() ? sit.mRawValue : sit.mOriginalValue);
		if (!sit.mType.empty()) settingNode.setAttribute("type", sit.mType);
		if (!sit.mComment.empty()) settingNode.setAttribute("comment", sit.mComment);
		if (!sit.mDefault.empty()) settingNode.setAttribute("default", sit.mDefault);
		if (!sit.mMinValue.empty()) settingNode.setAttribute("min_value", sit.mMinValue);
		if (!sit.mMaxValue.empty()) settingNode.setAttribute("max_value", sit.mMaxValue);
		if (!sit.mPossibleValues.empty()) settingNode.setAttribute("possibles", sit.mPossibleValues);
		for (auto& attr : sit.mOriginalAttributeMap) {
			if (std::find(WELL_KNOWN_ATTRIBUTES.begin(), WELL_KNOWN_ATTRIBUTES.end(), attr.first) ==
				WELL_KNOWN_ATTRIBUTES.end()) {}
			settingNode.setAttribute(attr.first, attr.second);
		}
		rootNode.push_back(settingNode);
	}


	xml.push_back(rootNode);
	xml.write(ci::writeFile(fullFilePath), true);
}

bool Settings::empty() const {
	return mSettings.empty();
}

void Settings::clear() {
	mSettings.clear();
}


bool Settings::getBool(const std::string& name, int index) {
	return getSetting(name, index).getBool();
}

bool Settings::getBool(const std::string& name, int index, bool defaultValue) {
	/// std::to_string was converting bool to int and returning 1 or 0
	std::string defaultString = "false";
	if (defaultValue) defaultString = "true";
	return getSetting(name, index, defaultString).getBool();
}

int Settings::getInt(const std::string& name, int index) {
	return getSetting(name, index).getInt();
}

int Settings::getInt(const std::string& name, int index, int defaultValue) {
	return getSetting(name, index, std::to_string(defaultValue)).getInt();
}

float Settings::getFloat(const std::string& name, int index) {
	return getSetting(name, index).getFloat();
}

float Settings::getFloat(const std::string& name, int index, float defaultValue) {
	return getSetting(name, index, std::to_string(defaultValue)).getFloat();
}

double Settings::getDouble(const std::string& name, int index) {
	return getSetting(name, index).getDouble();
}

double Settings::getDouble(const std::string& name, const int index, double defaultValue) {
	return getSetting(name, index, std::to_string(defaultValue)).getDouble();
}

ci::Color Settings::getColor(const ui::SpriteEngine& engine, const std::string& name, int index) {
	return getSetting(name, index).getColor(engine);
}

ci::Color Settings::getColor(const ui::SpriteEngine& engine, const std::string& name, int index,
							 const ci::Color& defaultValue) {
	return getSetting(name, index, unparseColor(defaultValue)).getColor(engine);
}

ci::ColorA Settings::getColorA(const ui::SpriteEngine& engine, const std::string& name, int index) {
	return getSetting(name, index).getColorA(engine);
}

ci::ColorA Settings::getColorA(const ui::SpriteEngine& engine, const std::string& name, int index,
							   const ci::ColorA& defaultValue) {
	return getSetting(name, index, unparseColor(defaultValue)).getColorA(engine);
}

const std::string& Settings::getString(const std::string& name, int index) {
	return getSetting(name, index).getString();
}

const std::string& Settings::getString(const std::string& name, int index, const std::string& defaultValue) {
	return getSetting(name, index, defaultValue).getString();
}

std::wstring Settings::getWString(const std::string& name, int index) {
	return getSetting(name, index).getWString();
}

std::wstring Settings::getWString(const std::string& name, int index, const std::wstring& defaultValue) {
	return getSetting(name, index, utf8_from_wstr(defaultValue)).getWString();
}

ci::vec2 Settings::getVec2(const std::string& name, int index) {
	return getSetting(name, index).getVec2();
}

ci::vec2 Settings::getVec2(const std::string& name, int index, const ci::vec2& defaultValue) {
	return getSetting(name, index, unparseVector(defaultValue)).getVec2();
}

ci::vec3 Settings::getVec3(const std::string& name, int index) {
	return getSetting(name, index).getVec3();
}

ci::vec3 Settings::getVec3(const std::string& name, int index, const ci::vec3& defaultValue) {
	return getSetting(name, index, unparseVector(defaultValue)).getVec3();
}

ci::Rectf Settings::getRect(const std::string& name, int index) {
	return getSetting(name, index).getRect();
}

ci::Rectf Settings::getRect(const std::string& name, int index, const ci::Rectf& defaultValue) {
	return getSetting(name, index, unparseRect(defaultValue)).getRect();
}

const std::map<std::string, std::string>& Settings::getAttributes(const std::string& name, int index) {
	return getSetting(name, index).mAttributeMap;
}

std::string Settings::getAttribute(const std::string& name, int index, const std::string& key,
								   const std::string& defaultValue) {
	return getSetting(name, index).getAttribute(key, defaultValue);
}

bool Settings::validateType(const std::string& inputType) {
	return std::find(SETTING_TYPES.begin(), SETTING_TYPES.end(), inputType) != SETTING_TYPES.end();
}

bool sortByReadIndex(const Settings::Setting& a, const Settings::Setting& b) {
	if (a.mReadIndex < b.mReadIndex) {
		return true;
	} else if (a.mReadIndex == b.mReadIndex) {
		return a.mName < b.mName;
	}

	return false;
}

std::vector<Settings::Setting>& Settings::getReadSortedSettings() {
	mSortedSettings.clear();

	for (const auto& it : mSettings) {
		auto theVec = it.second;
		for (const auto& sit : theVec) {
			mSortedSettings.push_back(sit);
		}
	}

	std::sort(mSortedSettings.begin(), mSortedSettings.end(), sortByReadIndex);

	return mSortedSettings;
}

bool Settings::hasSetting(const std::string& name) const {
	for (const auto& it : mSettings) {
		if (it.first == name) return true;
	}

	return false;
}

size_t Settings::countSetting(const std::string& name) const {
	for (const auto& it : mSettings) {
		if (it.first == name) return it.second.size();
	}

	return 0;
}

int Settings::getSettingIndex(const std::string& name) const {
	for (int i = 0; i < mSettings.size(); i++) {
		if (mSettings[i].first == name) return i;
	}

	return -1;
}

void Settings::forEachSetting(const std::function<void(Setting&)>& func, const std::string& typeFilter /*= ""*/) {
	auto& theThingies = getReadSortedSettings();
	for (auto&& sit : theThingies) {
		if (!typeFilter.empty() && typeFilter != sit.mType) continue;
		func(sit);
	}
}

Settings::Setting& Settings::getSetting(const std::string& name, int index) {
	return getSetting(name, index, "");
}

Settings::Setting& Settings::getSetting(const std::string& name, int index, const std::string& defaultRawValue) {
	auto settingIndex = getSettingIndex(name);

	if (settingIndex > -1 && index > -1 && !mSettings[settingIndex].second.empty() &&
		index < mSettings[settingIndex].second.size()) {
		return mSettings[settingIndex].second[index];
	}

	// create a new blank setting and return that
	std::vector<Setting> settings;
	settings.emplace_back();
	settings.back().mName	   = name;
	settings.back().mRawValue  = defaultRawValue;
	settings.back().mReadIndex = mReadIndex;
	mReadIndex += SETTINGS_INCREMENT;
	mSettings.emplace_back(std::pair(name, settings));

	return mSettings.back().second.back();
}

Settings::Setting& Settings::getSetting(const std::string& name, int index, const std::string& settingType,
										const std::string& commentValue, const std::string& defaultRawValue /*= ""*/,
										const std::string& minValue /*= ""*/, const std::string& maxValue /*= ""*/,
										const std::string& possibleValues /*= ""*/) {
	auto& theSetting		   = getSetting(name, index, defaultRawValue);
	theSetting.mType		   = settingType;
	theSetting.mComment		   = commentValue;
	theSetting.mDefault		   = defaultRawValue;
	theSetting.mMinValue	   = minValue;
	theSetting.mMaxValue	   = maxValue;
	theSetting.mPossibleValues = possibleValues;
	return theSetting;
}

void Settings::addSetting(const Setting& newSetting) {
	auto settingIndex = getSettingIndex(newSetting.mName);

	if (settingIndex > -1 && !mSettings.empty()) {
		mSettings[settingIndex].second.push_back(newSetting);
	} else {
		std::vector<Setting> theSettings;
		theSettings.push_back(newSetting);
		auto hintInsert = mSettings.end();
		mSettings.insert(hintInsert, std::pair(newSetting.mName, theSettings));
	}
}

void Settings::printAllSettings() {
	std::cout << "Settings: " << std::endl;
	auto& theVec = getReadSortedSettings();
	for (const auto& sit : theVec) {
		std::cout << std::endl << "\t" << sit.mName << ": \t" << sit.mRawValue << std::endl;
		std::cout << "\t\t source: \t" << sit.mSource << std::endl;

		if (!sit.mComment.empty()) std::cout << "\t\t comment: \t" << sit.mComment << std::endl;
		if (!sit.mType.empty()) std::cout << "\t\t type: \t\t" << sit.mType << std::endl;
		if (!sit.mDefault.empty()) std::cout << "\t\t default: \t" << sit.mDefault << std::endl;
		if (!sit.mMinValue.empty()) std::cout << "\t\t min: \t\t" << sit.mMinValue << std::endl;
		if (!sit.mMaxValue.empty()) std::cout << "\t\t max: \t\t" << sit.mMaxValue << std::endl;
		if (!sit.mPossibleValues.empty()) std::cout << "\t\t possibles: \t\t" << sit.mPossibleValues << std::endl;
	}
}

void Settings::replaceSettingVariablesAndExpressions() {
	for (auto& it : mSettings) {
		auto& theVec = it.second;
		for (auto& theSetting : theVec) {
			theSetting.replaceSettingVariablesAndExpressions();
		}
	}
}

} // namespace ds::cfg
