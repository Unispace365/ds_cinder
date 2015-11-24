#ifndef DELOITTE_PRESENTATION_SRC_CONFIGURABLE_H_
#define DELOITTE_PRESENTATION_SRC_CONFIGURABLE_H_

#include <memory>
#include <string>
#include "ds\cfg\settings.h"
#include "ds\ui\sprite\sprite_engine.h"

//#include "common/fwd.h"

namespace dlpr {
namespace util {

class Configurable
{
protected:
	// pass either: settings/name.xml, name.xml, or name
	Configurable(ds::ui::SpriteEngine& engine, const std::string& name);
	virtual ~Configurable();

	const ds::cfg::Settings&	getSettings() const;

private:
	ds::ui::SpriteEngine&		mConfigPoolEngine;
	std::string					mKey;
	Configurable()				= delete;
};

}} //!dlpr::common

#endif //!DELOITTE_PRESENTATION_SRC_CONFIGURABLE_H_
