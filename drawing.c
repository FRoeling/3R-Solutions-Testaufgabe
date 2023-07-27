#include <GL/glew.h>
#include <GL/gl.h>
#include <windows.h>
#include <math.h>
#include <time.h>

#include "util.h"
#include "drawing.h"
#include "app_math.h"

extern HWND gRenderWindowHandle;
extern HDC gRenderDC;
extern CircleSegment gCircleSegment;

BOOL render_axis = TRUE;

// Has something already been calculated for drawing? If yes, draw that until something new, that is valid, has been calculated
BOOL drawn_yet = FALSE;

void draw(void)
{
	// Clear frame buffer
	glClearColor(1.0f, 1.0f, 0.94117647f, 0.0f); // Paper color
	glClear(GL_COLOR_BUFFER_BIT);

	// Get render window rect
	RECT render_rect = get_client_rect(gRenderWindowHandle);
	int render_width = RECT_WIDTH(render_rect);
	int render_height = RECT_HEIGHT(render_rect);

	// Setup for pixel coordinates (Origin is in the middle of the render window)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-render_width / 2, render_width / 2, -render_height / 2, render_height / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Drawing

	if (render_axis)
		draw_axis(1, render_width, render_height);

	if (!drawn_yet)
	{
		SwapBuffers(gRenderDC); // If nothing has been calculated yet, still draw the render axis (therefore swap buffers)
		return;
	}

	// Radius is always the same for rendering
	const int circle_radius = (((float)render_height / 2) - (((float)render_height / 2) * 0.2f)); // Render height - 20%

	draw_circle(0, 0, circle_radius, 0, 1.0f);

	// Draw the circle segment using the radius and angle from the gCircleSegment
	draw_legs(gCircleSegment.deg_angle, circle_radius, RGB(0, 255, 255), 3.0f);
	draw_arc(gCircleSegment.deg_angle, circle_radius, RGB(0, 0, 255), 3.0f);
	draw_secant(gCircleSegment.deg_angle, circle_radius, RGB(255, 0, 0), 3.0f);

	// Drawing

	SwapBuffers(gRenderDC);
}

void draw_line(_In_ int x1, _In_ int y1, _In_ int x2, _In_ int y2, _In_ COLORREF color, _In_ float thickness)
{
	glLineWidth(thickness);
	glBegin(GL_LINES);
	glColor3ub(GetRValue(color), GetGValue(color), GetBValue(color));
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
	glLineWidth(1);
}

void draw_point(_In_ int x, _In_ int y, _In_ COLORREF color, _In_ float thickness)
{
	glPointSize(thickness);
	glBegin(GL_POINTS);
	glColor3ub(GetRValue(color), GetGValue(color), GetBValue(color));
	glVertex2i(x, y);
	glEnd();
	glPointSize(1);
}

void draw_circle(_In_ int x, _In_ int y, _In_ int radius, _In_ COLORREF color, _In_ float thickness)
{
	const unsigned int circle_segments = 2048; // Seems like a lot, but it takes almost not time to run this function
	const float PItimesTwo = (float)PI * 2.0f;

	glLineWidth(thickness);
	glBegin(GL_LINE_LOOP);
	glColor3ub(GetRValue(color), GetGValue(color), GetBValue(color));
	for (unsigned int i = 0; i < circle_segments; i++) 
	{
		float angle = PItimesTwo * i / circle_segments; // Angle of the vertex from the center of the circle
		int px = (int)((float)radius * cosf(angle));
		int py = (int)((float)radius * sinf(angle));
		glVertex2i(x + px, y + py);
	}
	glEnd();
	glLineWidth(1);
}

void draw_axis(_In_ float thickness, _In_ int render_width, _In_ int render_height)
{
	glLineWidth(thickness);
	glBegin(GL_LINES);
	glColor3ub(255, 0, 0);
	glVertex2i(-render_width / 2, 0);
	glVertex2i(render_width / 2, 0);
	glColor3ub(0, 255, 0);
	glVertex2i(0, -render_height / 2);
	glVertex2i(0, render_height / 2);
	glEnd();
	glLineWidth(1);
}

void draw_legs(_In_ float angle, _In_ int radius, _In_ COLORREF color, _In_ float thickness)
{
	angle = degrees_to_radians(angle);

	int x2 = (int)(cosf(angle) * (float)radius);
	int y2 = (int)(sinf(angle) * (float)radius);

	draw_line(0, 0, x2, y2, color, thickness);
	draw_line(0, 0, radius, 0, color, thickness);
}

void draw_secant(_In_ float angle, _In_ int radius, _In_ COLORREF color, _In_ float thickness)
{
	angle = degrees_to_radians(angle);

	int x1 = radius;
	int y1 = 0;

	int x2 = (int)(cosf(angle) * (float)radius);
	int y2 = (int)(sinf(angle) * (float)radius);

	draw_line(x1, y1, x2, y2, color, thickness);
}

void draw_arc(_In_ float angle, _In_ int radius, _In_ COLORREF color, _In_ float thickness)
{
	float rad_angle = degrees_to_radians(angle);
	
	// Calculate the number of segments dynamically based on the angle
	int num_segments = (angle / 360) * 2048;

	glLineWidth(thickness);
	glBegin(GL_LINE_STRIP);
	glColor3ub(GetRValue(color), GetGValue(color), GetBValue(color));
	for (int i = 0; i < num_segments; i++)
	{
		float angleS = rad_angle * i / num_segments;
		int px = (int)((float)radius * cosf(angleS));
		int py = (int)((float)radius * sinf(angleS));
		glVertex2i(px, py);
	}
	glEnd();
	glLineWidth(1);
}