#ifndef DRAWING_H
#define DRAWING_H

#include <windows.h>

void draw(void);

void draw_line(_In_ int x1, _In_ int y1, _In_ int x2, _In_ int y2, _In_ COLORREF color, _In_ float thickness);
void draw_point(_In_ int x, _In_ int y, _In_ COLORREF color, _In_ float thickness);
void draw_circle(_In_ int x, _In_ int y, _In_ int radius, _In_ COLORREF color, _In_ float thickness);
void draw_axis(_In_ float thickness, _In_ int render_width, _In_ int render_height);
void draw_legs(_In_ float angle, _In_ int radius, _In_ COLORREF color, _In_ float thickness);
void draw_secant(_In_ float angle, _In_ int radius, _In_ COLORREF color, _In_ float thickness);
void draw_arc(_In_ float angle, _In_ int radius, _In_ COLORREF color, _In_ float thickness);

#endif // DRAWING_H

extern BOOL render_axis;
extern BOOL drawn_yet;