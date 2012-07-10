#include "ds/app/engine.h"

#include "ds/debug/console.h"
#include "ds/debug/debug_defines.h"

namespace {
#ifdef _DEBUG
ds::Console		GLOBAL_CONSOLE;
#endif
}

namespace ds {

/**
 * \class ds::Engine
 */
Engine::Engine()
{
	DS_DBG_CODE(GLOBAL_CONSOLE.create());
}

Engine::~Engine()
{
	DS_DBG_CODE(GLOBAL_CONSOLE.destroy());
}

void Engine::update()
{
#if defined DS_PLATFORM_SERVER || defined DS_PLATFORM_SERVERCLIENT
	mWorkManager.update();
#endif
}

} // namespace ds
