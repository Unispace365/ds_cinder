#pragma once
#ifndef DS_DEBUG_CONSOLE_H_
#define DS_DEBUG_CONSOLE_H_

#include <cinder/app/msw/PlatformMsw.h>

namespace ds {

class Console {
  public:
	Console()
	  : mConsoleCreated(false) {}

	~Console() { destroy(); }

	void create() {
		if (mConsoleCreated) return;

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
		if (!mConsoleCreated) return;

		// NH: For some reason, the app was crashing on restart  if you were to show
		// the console and then restart.  The crash was occuring when initializing
		// the logger, which writes to std::cout.  I suspect it has something to do
		// with trying to write to file descriptor that has been destroyed.
		// Someone on StackOverflow says we can keep the file open,  but just direct
		// it to a null device with freopen().  This seems to have fixed the crashing
		// issue...
		// https://stackoverflow.com/a/4973065
		// fclose(stdin);
		// fclose(stdout);
		// fclose(stderr);
		freopen("nul", "r", stdin);
		freopen("nul", "w", stdout);
		freopen("nul", "w", stderr);

		if (!FreeConsole()) {
			DS_LOG_WARNING("failed to close console window");
		}
		mConsoleCreated = false;
	};

  private:
	bool mConsoleCreated;
};

} // namespace ds

#endif // DS_DEBUG_CONSOLE_H_