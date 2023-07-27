#ifndef UTIL_H
#define UTIL_H

#include <windows.h>
#include <wchar.h>

#define HEAP_ALLOC(size) (HeapAlloc(GetProcessHeap(), 0, size))
#define HEAP_ALLOCZ(size) (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size))
#define HEAP_REALLOC(memory, newsize) (HeapReAlloc(GetProcessHeap(), 0, memory, newsize))
#define HEAP_FREE(memory) (HeapFree(GetProcessHeap(), 0, memory))

#define ERR_MSGBOX(message) (MessageBoxW(NULL, message, L"Fehler!", MB_OK | MB_ICONERROR))

#define ERR_MSGEXIT(message) (MessageBoxW(NULL, message, L"Fehler!", MB_OK | MB_ICONERROR), exit(EXIT_FAILURE))

#define WSTR_EQUALS(str1, str2) (wcscmp(str1, str2) == 0)

#define RECT_WIDTH(rect) (rect.right - rect.left)
#define RECT_HEIGHT(rect) (rect.bottom - rect.top)

#define redraw_window(window_handle) (RedrawWindow(window_handle, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW))

// Functions
int messageboxf(_In_ UINT type, _In_ const wchar_t* const title, _In_ const wchar_t* const _format, ...);
RECT get_window_rect(_In_ HWND window_handle);
RECT get_client_rect(_In_ HWND window_handle);

#endif // UTIL_H