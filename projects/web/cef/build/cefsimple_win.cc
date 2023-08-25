#include <windows.h>
#include <include/cef_app.h>
int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ){
	CefMainArgs main_args; return CefExecuteProcess(main_args, NULL, NULL);
}