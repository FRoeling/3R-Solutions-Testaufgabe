#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>

#define WINDOW_TITLE L"Kreissektor berechnen"
#define WINDOW_STYLE (WS_VISIBLE | WS_OVERLAPPEDWINDOW)
#define WINDOW_CLASSNAME L"MainWindowClass"
#define RENDERWINDOW_CLASSNAME L"RenderWindowClass"

#define WM_INITIALIZED (WM_USER + 1)

typedef struct Window
{
	HWND handle;
	unsigned short cur_width, cur_height; // Client width and height

} Window;

typedef enum UIControlType
{
	UICT_NONE,
	UICT_BUTTON,
	UICT_TEXTBOX,
	UICT_CHECKBOX

} UICtrlType;

typedef struct UIControl
{
	HWND handle;
	UICtrlType type;
	unsigned int cmd_id;

	float normalx, normaly, normal_width, normal_height;

} UIControl;

// Initializes the main application window, returns FALSE on failure
BOOL initialize_window(_In_ HINSTANCE hinstance);

// Initializes the rendering window, returns FALSE on failure
BOOL initialize_render_window(void);

// Returns the type of ui control
UICtrlType get_ui_type_from_hwnd(_In_ HWND ui_handle);

// Loads a dialog window (content page) from the resources
BOOL load_content_page(_In_ int resource_id);

// Unloads the current content page
void unload_content_page(void);

#endif // WINDOW_H