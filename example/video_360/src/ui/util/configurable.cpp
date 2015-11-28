#include "configurable.h"

#include <unordered_map>

#include <ds/cfg/settings.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_cfg.h>

#include <boost/algorithm/string/replace.hpp>

namespace {
static struct ConfigPool
{
	bool has(ds::ui::SpriteEngine& engine, const std::string& key) const {
		return engine.getEngineCfg().hasSettings(key);
	}
	
	void load(ds::ui::SpriteEngine& engine, const std::string& key) {
		engine.loadSettings(key, key + ".xml");
	}

	const ds::cfg::Settings& get(ds::ui::SpriteEngine& engine, const std::string& key) const {
		return engine.getSettings(key);
	}
} CONFIG_POOL;
}

namespace ds {
namespace util {

Configurable::Configurable(ds::ui::SpriteEngine& engine, const std::string& name)
	: mConfigPoolEngine(engine)
{
	auto name_copy = name;
	if (name_copy.find("settings/") != std::string::npos)
	{
		boost::algorithm::replace_all(name_copy, "settings/", "");
	}
	if (name_copy.find(".xml") == std::string::npos)
	{
		boost::algorithm::replace_all(name_copy, ".xml", "");
	}

	mKey = name_copy;

	if (!CONFIG_POOL.has(mConfigPoolEngine, mKey))
	{
		CONFIG_POOL.load(mConfigPoolEngine, mKey);
	}
}

const ds::cfg::Settings& Configurable::getSettings() const
{
	return CONFIG_POOL.get(mConfigPoolEngine, mKey);
}

Configurable::~Configurable()
{
	/* no op */
}

}} //!dlpr::common
