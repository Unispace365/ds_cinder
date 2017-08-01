#pragma once
#ifndef DS_CFG_SETTINGS_MANAGER_H_
#define DS_CFG_SETTINGS_MANAGER_H_

#include <map>
#include <vector>
#include <cinder/Color.h>
#include <cinder/Rect.h>
#include "ds/data/resource.h"

namespace cinder {
class XmlTree;
}

namespace ds {
class Engine;

namespace cfg {

/**
* \class ds::cfg::SettingsManager
* \brief Store generic settings info.
*/
class SettingsManager {
public:
	SettingsManager(ds::Engine& engine);
	
	// An actual setting with some metadata. Type conversion happens at read time
	struct Setting {
		Setting(){};

		bool							getBool() const;
		int								getInt() const;
		float							getFloat() const;
		double							getDouble() const;

		// TBD: figure out how to parse color with an engine reference
		//const ci::Color&				getColor() const;
		//const ci::ColorA&				getColorA() const;

		const std::string&				getString() const;
		const std::wstring				getWstring() const;

		const ci::vec2					getVec2() const;
		const ci::vec3					getVec3() const;
		const cinder::Rectf&			getRect() const;

		std::string						mRawValue;
		std::string						mName;
		std::string						mComment;
		std::string						mType;
		std::string						mDefault;
		std::string						mMinValue;
		std::string						mMaxValue;
		std::string						mSource;
	};


	// Read from an xml file 
	void								readFrom(const std::string& fullFilePath, const bool append = true);

	void								writeTo(const std::string&fullFilePath);

	bool								isChanged() const { return mChanged; }
	bool								empty() const;
	void							  	clear();

	bool								getBool(const std::string& name, const int index = 0);
	int									getInt(const std::string& name, const int index = 0);
	float								getFloat(const std::string& name, const int index = 0);
	double								getDouble(const std::string& name, const int index = 0);

	const ci::Color						getColor(const std::string& name, const int index = 0);
	const ci::ColorA					getColorA(const std::string& name, const int index = 0);

	const std::string&					getString(const std::string& name, const int index = 0);
	const std::wstring					getWstring(const std::string& name, const int index = 0);

	const ci::vec2&						getVec2(const std::string& name, const int index = 0);
	const ci::vec3&						getVec3(const std::string& name, const int index = 0);
	const cinder::Rectf&				getRect(const std::string& name, const int index = 0);

	const Setting&						getSetting(const std::string& name, const int index) const;
	Setting&							getSetting(const std::string& name, const int index);

	void								addSetting(const Setting& newSetting);

	// some debuggin'
	void								printAllSettings();

	bool								hasSetting(const std::string& name) const;

protected:
	ds::Engine&							mEngine;
	bool								mChanged;

	/// The first vector is all settings
	/// The pair is to match the name of the setting
	/// The inner vector is for a series of settings with the same name (to support the index calls in the getSetting() calls)
	std::vector<std::pair<std::string, std::vector<Setting>>>			mSettings;

	void								directReadFrom(const std::string& filename, const bool clear);

	/// Returns the index of the name of the setting in the vector of all settings (not the index of the same setting)
	/// Return -1 if the setting isn't there or there are no settings
	int									getSettingIndex(const std::string& name) const;


};

} // namespace cfg
} // namespace ds

#endif // DS_CFG_SETTINGS_H_
