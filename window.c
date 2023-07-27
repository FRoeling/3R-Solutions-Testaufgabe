#include <GL/glew.h>
#include <GL/gl.h>
#include <stdint.h>
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "resource.h"
#include "window.h"
#include "util.h"
#include "drawing.h"
#include "app_math.h"

// Global main window struct
Window main_window = { 0 };
Window* gMainWindow = &main_window;

// Global render window handle
HWND gRenderWindowHandle = NULL;
HDC gRenderDC = NULL;

static struct ContentPage
{
	HWND handle;
	int num_controls;
	UIControl* controls;

} main_content_page;

// Function prototypes
LRESULT CALLBACK main_window_procedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK content_page_procedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK render_window_procedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK textbox_procedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK content_page_child_enum_func(HWND ui_handle, LPARAM lparam);
UICtrlType get_ui_type_from_hwnd(_In_ HWND ui_handle);

// UI functions
void calculate_button_click(void);
BOOL textbox_on_change(_In_ int ctrl_id, _In_ HWND textbox_handle, _In_ WPARAM wparam);

// Functions
BOOL initialize_window(_In_ HINSTANCE hinstance)
{
	// Register window class
	WNDCLASSEXW wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.hInstance = hinstance;
	wc.lpszClassName = WINDOW_CLASSNAME;
	wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 240));
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.lpfnWndProc = main_window_procedure;
	
	if (RegisterClassExW(&wc) == 0)
		return FALSE;

	// Calculate size and position
	int screen_size = GetSystemMetrics(SM_CXSCREEN); // Screen width
	int width = (int)(((float)screen_size / 3.0f) * 2.0f); // 2/3 of screen width
	int x = (screen_size - width) / 2;
	screen_size = GetSystemMetrics(SM_CYSCREEN); // Screen height
	int height = (int)(((float)screen_size / 3.0f) * 2.0f); // 2/3 of screen height
	int y = (screen_size - height) / 2;

	gMainWindow->handle = CreateWindowExW(0, WINDOW_CLASSNAME, WINDOW_TITLE, WINDOW_STYLE, x, y, width, height, NULL, NULL, NULL, 0);

	if (gMainWindow->handle == NULL)
		return FALSE;

	SendMessageW(gMainWindow->handle, WM_INITIALIZED, 0, 0);

	return TRUE;
}

BOOL initialize_render_window(void)
{
	// Register window class
	WNDCLASSEXW wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.hInstance = GetModuleHandleW(NULL);
	wc.lpszClassName = RENDERWINDOW_CLASSNAME;
	wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 240));
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.lpfnWndProc = render_window_procedure;

	if (RegisterClassExW(&wc) == 0)
		return FALSE;

	// Calculate size and position
	int width = (int)(((float)gMainWindow->cur_width / 3.0f) * 2.0f); // 3/4 of the main window width
	int height = gMainWindow->cur_height;
	int x = gMainWindow->cur_width - width; // Position it on the right side
	int y = 0;

	gRenderWindowHandle = CreateWindowExW(WS_EX_STATICEDGE, RENDERWINDOW_CLASSNAME, L"", WS_VISIBLE | WS_CHILD, x, y, width, height, gMainWindow->handle, NULL, NULL, 0);

	if (gRenderWindowHandle == NULL)
		return FALSE;

	// Initialize OpenGL
	const PIXELFORMATDESCRIPTOR pixel_format_descriptor = { sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, PFD_MAIN_PLANE };

	gRenderDC = GetDC(gRenderWindowHandle);

	if (gRenderDC == NULL)
		return FALSE;

	int pixel_format = ChoosePixelFormat(gRenderDC, &pixel_format_descriptor);

	if (pixel_format == 0)
		return FALSE;

	if (SetPixelFormat(gRenderDC, pixel_format, &pixel_format_descriptor) == 0)
		return FALSE;

	HGLRC rendering_context = wglCreateContext(gRenderDC);

	if (rendering_context == NULL)
		return FALSE;

	if (wglMakeCurrent(gRenderDC, rendering_context) == 0)
		return FALSE;

	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK)
		return FALSE;

	glViewport(0, 0, width, height);

	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_MULTISAMPLE);

	return TRUE;
}

BOOL load_content_page(_In_ int resource_id)
{
	main_content_page.handle = CreateDialogW(NULL, MAKEINTRESOURCE(resource_id), gMainWindow->handle, content_page_procedure);

	if (main_content_page.handle == NULL)
		return FALSE;

	// Count child controls of content page
	int child_count = 0;

	HWND child_handle = GetWindow(main_content_page.handle, GW_CHILD);
	while (child_handle != NULL)
	{
		child_count++;
		child_handle = GetWindow(child_handle, GW_HWNDNEXT);
	}

	// Allocate memory for the controls
	main_content_page.controls = HEAP_ALLOCZ(sizeof(UIControl) * child_count);

	if (main_content_page.controls == NULL)
		return FALSE;

	// Enum child windows
	EnumChildWindows(main_content_page.handle, content_page_child_enum_func, 0);

	SendMessageW(main_content_page.handle, WM_INITIALIZED, 0, 0);

	return TRUE;
}

void unload_content_page(void)
{
	// Free the allocated memory
	HEAP_FREE(main_content_page.controls);

	// Destroy window
	DestroyWindow(main_content_page.handle);

	// Reset struct
	ZeroMemory(&main_content_page, sizeof(main_content_page));
}

WNDPROC prev_textbox_proc = NULL;

BOOL CALLBACK content_page_child_enum_func(HWND ui_handle, LPARAM lparam)
{
	UIControl control = { 0 };
	control.handle = ui_handle;
	control.type = get_ui_type_from_hwnd(ui_handle);
	control.cmd_id = GetDlgCtrlID(ui_handle);

	// If it's a textbox, subclass it to handle user input (checking for valid input)
	if (control.type == UICT_TEXTBOX)
	{
		//SetWindowSubclass(control.handle, textbox_procedure, 0, 0);
		prev_textbox_proc = SetWindowLongPtr(ui_handle, GWLP_WNDPROC, (LONG_PTR)textbox_procedure);
		SendMessageW(ui_handle, EM_LIMITTEXT, 7, 0); // Limit each textbox to 7 characters max

		if (control.cmd_id == IDC_ANGLE_EDIT)
			SetFocus(ui_handle);
	}
	else if (control.type == UICT_CHECKBOX)
	{
		SendMessageW(ui_handle, BM_SETCHECK, BST_CHECKED, 0);
	}

	// Calculate normalized coordinates
	RECT ui_rect = get_window_rect(ui_handle);
	POINT ui_pos = { ui_rect.left, ui_rect.top };
	ScreenToClient(main_content_page.handle, &ui_pos);

	int16_t ui_x = (int16_t)ui_pos.x, ui_y = (int16_t)ui_pos.y,
		ui_width = (int16_t)(ui_rect.right - ui_rect.left),
		ui_height = (int16_t)(ui_rect.bottom - ui_rect.top);

	RECT parent_rect = get_window_rect(main_content_page.handle);

	int16_t parent_page_width = (int16_t)RECT_WIDTH(parent_rect);
	int16_t parent_page_height = (int16_t)RECT_HEIGHT(parent_rect);

	// Get normalized size and position
	control.normalx = (float)(2 * ui_x) / parent_page_width - 1;
	control.normaly = (float)(1 - (float)(2 * ui_y) / parent_page_height);
	control.normal_width = (float)(2 * ui_width) / parent_page_width;
	control.normal_height = (float)(2 * ui_height) / parent_page_height;

	main_content_page.controls[main_content_page.num_controls++] = control;
}

UICtrlType get_ui_type_from_hwnd(_In_ HWND ui_handle)
{
	wchar_t classname[32];
	GetClassNameW(ui_handle, classname, 32);

	if (WSTR_EQUALS(classname, L"BUTTON") || WSTR_EQUALS(classname, L"Button"))
	{
		DWORD button_type = GetWindowLongPtr(ui_handle, GWL_STYLE) & BS_TYPEMASK;

		if (button_type == BS_CHECKBOX || button_type == BS_AUTOCHECKBOX || button_type == BS_3STATE)
			return UICT_CHECKBOX;

		return UICT_BUTTON;
	}
	else if (WSTR_EQUALS(classname, L"EDIT") || WSTR_EQUALS(classname, L"Edit"))
	{
		return UICT_TEXTBOX;
	}

	return UICT_NONE;
}

static inline void page_on_command(_In_ unsigned int command_id, _In_ int event, _In_ HWND control_handle)
{
	if (event == BN_CLICKED)
	{
		if (command_id == IDC_DRAW_BUTTON)
		{
			calculate_button_click();
		}
		else if (command_id == IDC_DRAW_AXIS)
		{
			render_axis = (SendMessage(control_handle, BM_GETCHECK, 0, 0) == BST_CHECKED);
			
			draw();
		}
	}
}

// Callback procedure functions
LRESULT CALLBACK main_window_procedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		case WM_INITIALIZED:
		{
			// Load content from resource
			if (load_content_page(IDD_MAIN_PAGE) == FALSE)
				ERR_MSGEXIT(L"Fehler: Konnte den Seiteninhalt nicht aus den Ressourcen laden!");

			break;
		}
		case WM_CLOSE:
		{
			unload_content_page();
			DestroyWindow(window_handle);
			PostQuitMessage(0);
			break;
		}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lparam;
			int screen_width = GetSystemMetrics(SM_CXSCREEN);
			int screen_height = GetSystemMetrics(SM_CYSCREEN);

			lpMMI->ptMinTrackSize.x = (screen_width / 2) + 16;
			lpMMI->ptMinTrackSize.y = (screen_height / 2) + 39;
			break;
		}
		case WM_SIZE:
		{
			RECT client_rect = get_client_rect(window_handle);

			gMainWindow->cur_width = RECT_WIDTH(client_rect);
			gMainWindow->cur_height = RECT_HEIGHT(client_rect);

			// Position the content page
			if (main_content_page.handle != NULL)
			{
				SetWindowPos(main_content_page.handle, NULL, 0, 0, gMainWindow->cur_width, gMainWindow->cur_height, SWP_NOZORDER);
			}

			break;
		}
	}

	return DefWindowProcW(window_handle, message, wparam, lparam);
}

LRESULT CALLBACK content_page_procedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	static HBRUSH background_brush = NULL;

	switch (message)
	{
		case WM_INITDIALOG:
		{
			background_brush = CreateSolidBrush(RGB(255, 255, 255));
			break;
		}
		case WM_INITIALIZED:
		{
			// Position the content page on window
			SetWindowPos(window_handle, NULL, 0, 0, gMainWindow->cur_width, gMainWindow->cur_height, SWP_NOZORDER);

			if (initialize_render_window() == FALSE)
			{
				ERR_MSGEXIT(L"Fehler: Konnte den OpenGL Kontext nicht initialisieren!");
			}

			break;
		}
		case WM_SIZE:
		{
			int page_new_width = LOWORD(lparam);
			int page_new_height = HIWORD(lparam);

			// Resize and position the ui controls
			for (int i = 0; i < main_content_page.num_controls; i++)
			{
				UIControl cur_ctrl = main_content_page.controls[i];

				int ui_x = (int)((cur_ctrl.normalx + 1) * 0.5f * page_new_width);
				int ui_y = (int)((1 - cur_ctrl.normaly) * 0.5f * page_new_height);
				int ui_width = (int)(cur_ctrl.normal_width * 0.5f * page_new_width);
				int ui_height = (int)(cur_ctrl.normal_height * 0.5f * page_new_height);

				SetWindowPos(cur_ctrl.handle, NULL, ui_x, ui_y, ui_width, ui_height, SWP_NOZORDER | SWP_NOACTIVATE);

				redraw_window(cur_ctrl.handle);
			}

			// Reposition render window
			if (gRenderWindowHandle != NULL)
			{
				int width = (int)(((float)gMainWindow->cur_width / 3.0f) * 2.0f); // 3/4 of the main window width
				int height = gMainWindow->cur_height;
				int x = gMainWindow->cur_width - width; // Position it on the right side
				int y = 0;

				SetWindowPos(gRenderWindowHandle, NULL, x, y, width, height, SWP_NOZORDER);
			}

			break;
		}
		case WM_CTLCOLORDLG:
		{
			return (LRESULT)background_brush;
		}
		case WM_CTLCOLORBTN:
		{
			HDC button_dc = (HDC)wparam;
			HWND button_handle = (HWND)lparam;

			SetBkMode(button_dc, TRANSPARENT);
			RECT ui_rect = get_client_rect(button_handle);
			FillRect(button_dc, &ui_rect, background_brush);

			return (LRESULT)GetStockObject(NULL_BRUSH);
		}
		case WM_CTLCOLORSTATIC:
		{
			SetBkMode((HDC)wparam, TRANSPARENT);

			int textbox_id = GetDlgCtrlID((HWND)lparam);

			switch (textbox_id)
			{
				case IDC_DISPLAY_RADIUS:
					SetTextColor((HDC)wparam, RGB(0, 255, 255));
					break;
				case IDC_DISPLAY_ARC:
					SetTextColor((HDC)wparam, RGB(0, 0, 255));
					break;
				case IDC_DISPLAY_SECANT:
					SetTextColor((HDC)wparam, RGB(255, 0, 0));
					break;
				default:
					SetTextColor((HDC)wparam, 0);
					break;
			}

			return (LRESULT)background_brush;
		}
		case WM_DESTROY:
		{
			DeleteObject(background_brush);
			break;
		}
		case WM_COMMAND:
		{
			page_on_command((unsigned int)LOWORD(wparam), (int)HIWORD(wparam), (HWND)lparam);
			break;
		}
	}

	return DefWindowProcW(window_handle, message, wparam, lparam);
}

LRESULT CALLBACK render_window_procedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		case WM_SIZE:
		{
			glViewport(0, 0, LOWORD(lparam), HIWORD(lparam));
			draw();
			break;
		}
	}

	return DefWindowProcW(window_handle, message, wparam, lparam);
}

LRESULT CALLBACK textbox_procedure(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
		case WM_CHAR:
		{
			if (wparam == VK_RETURN) // Enter key pressed
			{
				calculate_button_click();
				break;
			}

			int ctrl_id = GetDlgCtrlID(window_handle);
			
			if (textbox_on_change(ctrl_id, window_handle, wparam) == FALSE)
				return 0;

			break;
		}
	}

	return CallWindowProcW(prev_textbox_proc, window_handle, message, wparam, lparam);
}

// Command functions
BOOL textbox_on_change(_In_ int ctrl_id, _In_ HWND textbox_handle, _In_ WPARAM wparam)
{
	wchar_t character = (wchar_t)wparam;

	// If the character is neither a number, nor a dot nor a control character (return, enter, etc.) return
	if (!iswdigit(character) && character != L'.' && character > L' ')
		return FALSE;

	int text_length = GetWindowTextLengthW(textbox_handle);

	// Check if there's already a dot in the text and return if so
	wchar_t buffer[8] = { 0 };
	GetWindowTextW(textbox_handle, buffer, 7);

	if (character == L'.' && wcschr(buffer, L'.'))
		return FALSE;

	if (text_length == 3 && character != L'.' && character > L' ' && !wcschr(buffer, L'.'))
		return FALSE;

	return TRUE;
}

void calculate_button_click(void)
{
	float angle = 0, *p_angle;
	float radius = 0, *p_radius;
	float secant = 0, *p_secant;
	float area = 0, *p_area;
	float arc = 0, *p_arc;

	// Check user input
	wchar_t textbox_content[8] = { 0 };
	GetWindowTextW(GetDlgItem(main_content_page.handle, IDC_ANGLE_EDIT), textbox_content, 8);

	if (WSTR_EQUALS(textbox_content, L"") || WSTR_EQUALS(textbox_content, L"."))
	{
		p_angle = NULL;
	}
	else
	{
		swscanf_s(textbox_content, L"%f", &angle);

		if (angle > 360.000000f || angle < 0.000001f)
		{
			messageboxf(MB_OK | MB_ICONWARNING, L"Ungültige Eingabe!", L"Der eingegebene Winkel: %.3f ist ungültig (0 - 360 Grad)", angle);
			return;
		}

		p_angle = &angle;
	}

	GetWindowTextW(GetDlgItem(main_content_page.handle, IDC_RADIUS_EDIT), textbox_content, 8);

	if (WSTR_EQUALS(textbox_content, L"") || WSTR_EQUALS(textbox_content, L"."))
	{
		p_radius = NULL;
	}
	else
	{
		swscanf_s(textbox_content, L"%f", &radius);

		p_radius = &radius;
	}

	GetWindowTextW(GetDlgItem(main_content_page.handle, IDC_SECANT_EDIT), textbox_content, 8);

	if (WSTR_EQUALS(textbox_content, L"") || WSTR_EQUALS(textbox_content, L"."))
	{
		p_secant = NULL;
	}
	else
	{
		swscanf_s(textbox_content, L"%f", &secant);
		p_secant = &secant;
	}

	GetWindowTextW(GetDlgItem(main_content_page.handle, IDC_AREA_EDIT), textbox_content, 8);

	if (WSTR_EQUALS(textbox_content, L"") || WSTR_EQUALS(textbox_content, L"."))
	{
		p_area = NULL;
	}
	else
	{
		swscanf_s(textbox_content, L"%f", &area);
		p_area = &area;
	}

	GetWindowTextW(GetDlgItem(main_content_page.handle, IDC_ARC_EDIT), textbox_content, 8);

	if (WSTR_EQUALS(textbox_content, L"") || WSTR_EQUALS(textbox_content, L"."))
	{
		p_arc = NULL;
	}
	else
	{
		swscanf_s(textbox_content, L"%f", &arc);
		p_arc = &arc;
	}

	// Make the display textboxes visible
	ShowWindow(GetDlgItem(main_content_page.handle, IDC_DISPLAY_ANGLE), SW_SHOW);
	ShowWindow(GetDlgItem(main_content_page.handle, IDC_DISPLAY_RADIUS), SW_SHOW);
	ShowWindow(GetDlgItem(main_content_page.handle, IDC_DISPLAY_SECANT), SW_SHOW);
	ShowWindow(GetDlgItem(main_content_page.handle, IDC_DISPLAY_AREA), SW_SHOW);
	ShowWindow(GetDlgItem(main_content_page.handle, IDC_DISPLAY_ARC), SW_SHOW);
	ShowWindow(GetDlgItem(main_content_page.handle, IDC_CALCULATED_LABEL), SW_SHOW);

	// Calculate and draw
	if (calculate_everything(p_area, p_secant, p_arc, p_angle, p_radius) == FALSE)
		return;

	drawn_yet = TRUE;
	draw();

	// Put the results in the display textboxes
#define BUFFERSIZE 16
	wchar_t buffer[BUFFERSIZE] = { 0 };

	// Angle display textbox
	swprintf_s(buffer, BUFFERSIZE, L"%.3f°", gCircleSegment.deg_angle);
	SetWindowTextW(GetDlgItem(main_content_page.handle, IDC_DISPLAY_ANGLE), buffer);

	// Radius display textbox
	swprintf_s(buffer, BUFFERSIZE, L"%.3f", gCircleSegment.radius);
	SetWindowTextW(GetDlgItem(main_content_page.handle, IDC_DISPLAY_RADIUS), buffer);

	// Secant textbox
	swprintf_s(buffer, BUFFERSIZE, L"%.3f", gCircleSegment.secant);
	SetWindowTextW(GetDlgItem(main_content_page.handle, IDC_DISPLAY_SECANT), buffer);

	// Area textbox
	swprintf_s(buffer, BUFFERSIZE, L"%.3f", gCircleSegment.area);
	SetWindowTextW(GetDlgItem(main_content_page.handle, IDC_DISPLAY_AREA), buffer);

	// Arc textbox
	swprintf_s(buffer, BUFFERSIZE, L"%.3f", gCircleSegment.arc);
	SetWindowTextW(GetDlgItem(main_content_page.handle, IDC_DISPLAY_ARC), buffer);
}