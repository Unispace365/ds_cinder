#pragma once
#ifndef DS_DEBUG_CONSOLE_H_
#define DS_DEBUG_CONSOLE_H_

#include <Windows.h>
#include <io.h>
#include <fcntl.h>

namespace ds {

class Console
{
public:
	Console()
		: mConsoleCreated(false)
	{
	}

	~Console()
	{
		destroy();
	}

	void create()
	{
		if ( mConsoleCreated )
			return;

    // The app writes to the output before any console has been allocated, which puts
    // it in an error state, so that needs to be cleared out before you'll see any output.
    std::cout.clear();
    std::cin.clear();

		AllocConsole();

		HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
		int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
		FILE* hf_out = _fdopen(hCrt, "w");
		setvbuf(hf_out, NULL, _IONBF, 1);
		*stdout = *hf_out;

		HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
		hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
		FILE* hf_in = _fdopen(hCrt, "r");
		setvbuf(hf_in, NULL, _IONBF, 128);
		*stdin = *hf_in;

		mConsoleCreated = true;
	}

	void destroy()
	{
		if ( !mConsoleCreated )
			return;

		FreeConsole();
		mConsoleCreated = false;
	}

private:
	bool mConsoleCreated;
};

}

#endif // DS_DEBUG_CONSOLE_H_