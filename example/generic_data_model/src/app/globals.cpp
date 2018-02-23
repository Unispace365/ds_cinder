#include "stdafx.h"

#include "globals.h"

#include <Poco/String.h>

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>

#include "query/data_wrangler.h"

namespace downstream {

/**
 * \class downstream::Globals
 */
Globals::Globals(ds::ui::SpriteEngine& e, DataWrangler& dataWrangler)
	: mEngine(e)
	, mDataWrangler(dataWrangler)
{
}




} // !namespace downstream

