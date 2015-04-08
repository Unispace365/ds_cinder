#include "ds/app/engine/engine_cfg.h"

#include <Poco/String.h>
#include "ds/app/environment.h"
#include "ds/debug/debug_defines.h"

static void read_nine_patch_cfg(const std::string& path, std::unordered_map<std::string, ds::cfg::NinePatch>& out);
static void read_text_cfg(const std::string& path, std::unordered_map<std::string, ds::cfg::Text>& out);

namespace ds {

namespace {
const std::string			EMPTY_SZ;
const std::string			ENGINE_SZ("engine");
}

/**
 * ds::EngineCfg
 */
EngineCfg::EngineCfg(const ds::cfg::Settings& engine_settings)
		: mEngineSettings(engine_settings) {
}

const ds::cfg::Settings& EngineCfg::getSettings(const std::string& name) const {
	if (name.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() on empty name"));
		return mEmptySettings;
	}
	if (mSettings.empty()) {
		if (name == ENGINE_SZ) return mEngineSettings;
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() on empty mSettings"));
		return mEmptySettings;
	}
	auto it = mSettings.find(name);
	if (it == mSettings.end()) {
		if (name == ENGINE_SZ) return mEngineSettings;
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getSettings() settings does not exist"));
		return mEmptySettings;
	}
	return it->second;
}

ds::cfg::Settings& EngineCfg::editSettings(const std::string& name) {
	if (name.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::editSettings() on empty name"));
		return mEditEmptySettings;
	}
	if (mSettings.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::editSettings() on empty mSettings"));
		return mEditEmptySettings;
	}
	auto it = mSettings.find(name);
	if (it == mSettings.end()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::editSettings() settings does not exist"));
		return mEditEmptySettings;
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

const ds::cfg::Text& EngineCfg::getText(const std::string& name) const {
	if (name.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getText() on empty name"));
		return mEmptyTextCfg;
	}
	if (mTextCfg.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getText() on empty mTextCfg (key=" + name + ")"));
		return mEmptyTextCfg;
	}
	auto it = mTextCfg.find(name);
	if (it == mTextCfg.end()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getText() cfg does not exist"));
		return mEmptyTextCfg;
	}
	return it->second;
}

void EngineCfg::setText(const std::string& name, const ds::cfg::Text& t) {
	try {
		mTextCfg[name] = t;
	} catch (std::exception const&) {
	}
}

bool EngineCfg::hasNinePatch(const std::string& name) const {
	if (name.empty()) return false;
	if (mNinePatchCfg.empty()) return false;
	auto it = mNinePatchCfg.find(name);
	if (it == mNinePatchCfg.end()) return  false;
	return true;
}

const ds::cfg::NinePatch& EngineCfg::getNinePatch(const std::string& name) const {
	if (name.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getNinePatch() on empty name"));
		return mEmptyNinePatchCfg;
	}
	if (mNinePatchCfg.empty()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getNinePatch() on empty mNinePatchCfg (key=" + name + ")"));
		return mEmptyNinePatchCfg;
	}
	auto it = mNinePatchCfg.find(name);
	if (it == mNinePatchCfg.end()) {
		DS_DBG_CODE(throw std::runtime_error("EngineCfg::getNinePatch() cfg does not exist"));
		return mEmptyNinePatchCfg;
	}
	return it->second;
}

bool EngineCfg::hasSettings(const std::string& name) const {
	return mSettings.find(name) != mSettings.cend();
}

void EngineCfg::loadSettings(const std::string& name, const std::string& filename) {
	ds::cfg::Settings&	settings = mSettings[name];
	ds::Environment::loadSettings(filename, settings);
}

void EngineCfg::appendSettings(const std::string& name, const std::string& filename) {
	ds::cfg::Settings&	settings = mSettings[name];
	settings.readFrom(filename, true);
}

void EngineCfg::loadText(const std::string& filename) {
	read_text_cfg(ds::Environment::getAppFolder(ds::Environment::SETTINGS(), filename), mTextCfg);
	read_text_cfg(ds::Environment::getLocalSettingsPath(filename), mTextCfg);
}

void EngineCfg::loadNinePatchCfg(const std::string& filename) {
	read_nine_patch_cfg(ds::Environment::getAppFolder(ds::Environment::SETTINGS(), filename), mNinePatchCfg);
	read_nine_patch_cfg(ds::Environment::getLocalSettingsPath(filename), mNinePatchCfg);
}

} // namespace ds

static bool split_key(const std::string& key, std::string& left, std::string& right) {
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

static ds::cfg::NinePatch::Type type_from(const std::string& v) {
	const std::string		s = Poco::toLower(v);
	if (s == "arc drop shadow") return ds::cfg::NinePatch::ARC_DROP_SHADOW;
	return ds::cfg::NinePatch::EMPTY; 
}

static void read_nine_patch_cfg(const std::string& path, std::unordered_map<std::string, ds::cfg::NinePatch>& out) {
	ds::cfg::Settings		s;
	s.readFrom(path, false);

	// Do the type first, because that determines whether an entry exists.
	s.forEachTextKey([&s, &out](const std::string& key) {
		std::string			left, right;
		if (split_key(key, left, right) && right == "type") {
			ds::cfg::NinePatch::Type type = type_from(s.getText(key, 0, ds::EMPTY_SZ));
			if (type != ds::cfg::NinePatch::EMPTY) {
				if (out.empty()) {
					out[left] = ds::cfg::NinePatch(type);
				} else {
					auto	found = out.find(left);
					if (found != out.end()) {
						found->second.mType = type;
					} else {
						out[left] = ds::cfg::NinePatch(type);
					}
				}
			}
		}
	});

	// Floats
	s.forEachFloatKey([&s, &out](const std::string& key) {
		std::string			left, right;
		if (split_key(key, left, right) && !out.empty()) {
			auto			found = out.find(left);
			if (found != out.end()) {
				found->second.mStore.setFloat(right, s.getFloat(key, 0, 0.0f));
			}
		}
	});

	// Color (color)
	s.forEachColorAKey([&s, &out](const std::string& key) {
		std::string			left, right;
		if (split_key(key, left, right) && !out.empty()) {
			auto			found = out.find(left);
			if (found != out.end()) {
				found->second.mStore.setColorA(right, s.getColorA(key, 0, ci::ColorA()));
			}
		}
	});

	// Strings (everything but type)
	s.forEachTextKey([&s, &out](const std::string& key) {
		std::string			left, right;
		if (split_key(key, left, right) && !out.empty() && right != "type") {
			auto			found = out.find(left);
			if (found != out.end()) {
				found->second.mStore.setString(right, s.getText(key, 0, ""));
			}
		}
	});
}

static void read_text_cfg(const std::string& path, std::unordered_map<std::string, ds::cfg::Text>& out) {
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

	// Text (alignment)
	s.forEachTextKey([&s, &out](const std::string& key) {
		std::string			left, right;
		if (split_key(key, left, right) && !out.empty()) {
			auto			found = out.find(left);
			if (found != out.end()) {
				if (right == "alignment") {
					found->second.mAlignment = ds::ui::Alignment::fromString(s.getText(key, 0, ""));
				}
			}
		}
	});

}