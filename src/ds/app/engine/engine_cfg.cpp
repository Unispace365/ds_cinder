#include "ds/app/engine/engine_cfg.h"

#include <Poco/String.h>
#include "ds/app/environment.h"
#include "ds/debug/debug_defines.h"

static void read_text_cfg(const std::string& path, std::unordered_map<std::string, ds::cfg::Text>& out);

namespace ds {

namespace {
const ds::cfg::Settings		EMPTY_SETTINGS;
const ds::cfg::Text			EMPTY_TEXT_CFG;
const std::string			EMPTY_SZ;
}

/**
 * ds::EngineCfg
 */
EngineCfg::EngineCfg()
{
}

const ds::cfg::Settings& EngineCfg::getSettings(const std::string& name) const
{
	if (name.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() on empty name"));
		return EMPTY_SETTINGS;
	}
	if (mSettings.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() on empty mSettings"));
		return EMPTY_SETTINGS;
	}
	auto it = mSettings.find(name);
	if (it == mSettings.end()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() settings does not exist"));
		return EMPTY_SETTINGS;
	}
	return it->second;
}

const ds::cfg::Text& EngineCfg::getText(const std::string& name) const
{
	if (name.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getTextCfg() on empty name"));
		return EMPTY_TEXT_CFG;
	}
	if (mTextCfg.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getTextCfg() on empty mTextCfg"));
		return EMPTY_TEXT_CFG;
	}
	auto it = mTextCfg.find(name);
	if (it == mTextCfg.end()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getTextCfg() cfg does not exist"));
		return EMPTY_TEXT_CFG;
	}
	return it->second;
}

bool EngineCfg::hasText(const std::string& name) const {
	if (name.empty()) return false;
	if (mTextCfg.empty()) return false;
	auto it = mTextCfg.find(name);
	if (it == mTextCfg.end()) return  false;
	return true;
}

void EngineCfg::loadSettings(const std::string& name, const std::string& filename)
{
	ds::cfg::Settings&	settings = mSettings[name];
	ds::Environment::loadSettings(filename, settings);
}

void EngineCfg::loadText(const std::string& filename)
{
	read_text_cfg(ds::Environment::getAppFolder(ds::Environment::SETTINGS(), filename), mTextCfg);
	read_text_cfg(ds::Environment::getLocalSettingsPath(filename), mTextCfg);
}

} // namespace ds

static bool split_key(const std::string& key, std::string& left, std::string& right)
{
	if (key.empty()) return false;
	const size_t		pos = key.find_last_of(":");
	if (pos != key.npos) {
		left = key.substr(0, pos);
		right = key.substr(pos+1, key.length()-pos+1);
		Poco::toLowerInPlace(right);
		return !left.empty() && !right.empty();
	}
	return false;
}

static void read_text_cfg(const std::string& path, std::unordered_map<std::string, ds::cfg::Text>& out)
{
	ds::cfg::Settings		s;
	s.readFrom(path, false);

	// Do the name first, because that determines whether an entry exists.
	s.forEachTextKey([&s, &out](const std::string& key) {
		std::string			left, right;
		if (split_key(key, left, right) && right == "name") {
			std::string		v = s.getText(key, 0, ds::EMPTY_SZ);
			if (!v.empty()) {
				if (out.empty()) {
					out[left] = ds::cfg::Text(v, 10.0f, 1.0f, ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f));
				} else {
					auto	found = out.find(left);
					if (found != out.end()) {
						found->second.mFont = v;
					} else {
						out[left] = ds::cfg::Text(v, 10.0f, 1.0f, ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f));
					}
				}
			}
		}
	});

	// Floats (size, leading)
	s.forEachFloatKey([&s, &out](const std::string& key) {
		std::string			left, right;
		if (split_key(key, left, right) && !out.empty()) {
			auto			found = out.find(left);
			if (found != out.end()) {
				if (right == "size") found->second.mSize = s.getFloat(key, 0, found->second.mSize);
				else if (right == "leading") found->second.mLeading = s.getFloat(key, 0, found->second.mLeading);
			}
		}
	});

	// Color (color)
	s.forEachColorAKey([&s, &out](const std::string& key) {
		std::string			left, right;
		if (split_key(key, left, right) && !out.empty()) {
			auto			found = out.find(left);
			if (found != out.end()) {
				if (right == "color") found->second.mColor = s.getColorA(key, 0, found->second.mColor);
			}
		}
	});

}