#include <windows.h>
#include <commctrl.h>

#include "window.h"
#include "util.h"

inline void setup(_In_ HINSTANCE hinstance);
inline void cleanup(void);

int WINAPI WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hprevinstance, _In_ LPCSTR cmdline, _In_ int showcmd)
{
	UNREFERENCED_PARAMETER(hprevinstance);
	UNREFERENCED_PARAMETER(cmdline);
	UNREFERENCED_PARAMETER(showcmd);

	setup(hinstance);

	MSG message = { 0 };

	while (GetMessageW(&message, NULL, 0, 0) > 0)
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	cleanup();

	return 0;
}

inline void setup(_In_ HINSTANCE hinstance)
{
	// Init common controls
	INITCOMMONCONTROLSEX icex = { 0 };
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES;
	InitCommonControlsEx(&icex);

	// Initialize the main window
	if (initialize_window(hinstance) == FALSE)
	{
		ERR_MSGEXIT(L"Fehler: Konnte das Fenster nicht initialisieren!");
	}
}

inline void cleanup(void)
{
	UnregisterClassW(WINDOW_CLASSNAME, NULL);
	UnregisterClassW(RENDERWINDOW_CLASSNAME, NULL);
}