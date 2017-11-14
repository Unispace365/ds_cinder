#pragma once
#ifndef DS_DEBUG_CONSOLE_H_
#define DS_DEBUG_CONSOLE_H_

#include <cinder/app/msw/PlatformMsw.h>

namespace ds {

class Console
{
public:
	Console()
		: mConsoleCreated(false)
	{}

	~Console() {
		destroy();
	}

	void create() {
		if(mConsoleCreated)	return;

		AllocConsole();
		mConsoleCreated = true;
		ci::app::PlatformMsw::get()->directConsoleToCout(true);

		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);

		std::cout.clear();
		std::cerr.clear();
		std::cin.clear();
	}
	void destroy() {
		if(!mConsoleCreated)
			return;

		fclose(stdin);
		fclose(stdout);
		fclose(stderr);

		if(!FreeConsole()) {
			DS_LOG_WARNING("failed to close console window");
		}
		mConsoleCreated = false;
	};

private:
	bool mConsoleCreated;
};

}

#endif // DS_DEBUG_CONSOLE_H_