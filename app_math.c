#include <math.h>
#include "app_math.h"
#include "drawing.h"
#include "util.h"

const double PI = 3.14159265358979323846;

CircleSegment gCircleSegment = { 0 };

float radians_to_degrees(_In_ float radians)
{
	return radians * (float)(180 / PI);
}

float degrees_to_radians(_In_ float degrees)
{
	return degrees * (float)(PI / 180);
}

static float calculate_area(_In_ float radius, _In_ float rad_angle)
{
	return ((radius * radius) * rad_angle) / 2;
}

static float calculate_secant(_In_ float radius, _In_ float rad_angle)
{
	return 2 * radius * sinf(rad_angle / 2);
}

static float calculate_arc_length(_In_ float radius, _In_ float rad_angle)
{
	return radius * rad_angle;
}

BOOL calculate_everything(_In_opt_ float* area, _In_opt_ float* secant, _In_opt_ float* arc_length, _In_opt_ float* deg_angle, _In_opt_ float* radius)
{
	unsigned int combi = 0; // Combination of values the user entered

	// Check for NULL to decide the combination
	combi = ((area == NULL) ? 0 : CMB_AREA) | ((secant == NULL) ? 0 : CMB_SECANT) | ((arc_length == NULL) ? 0 : CMB_ARC) | ((deg_angle == NULL) ? 0 : CMB_ANGLE) | ((radius == NULL) ? 0 : CMB_RADIUS);

	if (combi == 0) // No choices made
		return FALSE;

	if ((combi & (combi - 1)) == FALSE) // Check that at least two values are given
		return FALSE;

	float r = 0; // Radius
	float a = 0; // Angle (α)
	float arc = 0; // Arc length
	float A = 0; // Area
	float s = 0; // Secant

	if ((combi & CMB_ARC) && (combi & CMB_RADIUS))
	{
		// User entered arc length and radius
		r = *radius;
		arc = *arc_length;
		a = arc / r;
		A = calculate_area(r, a);
		s = calculate_secant(r, a);
	}
	else if ((combi & CMB_ARC) && (combi & CMB_ANGLE))
	{
		// User entered arc length and angle
		a = degrees_to_radians(*deg_angle);
		arc = *arc_length;
		r = arc / a;
		A = calculate_area(r, a);
		s = calculate_secant(r, a);
	}
	else if ((combi & CMB_AREA) && (combi & CMB_ANGLE))
	{
		// User entered area and angle
		A = *area;
		a = degrees_to_radians(*deg_angle);
		r = sqrtf((2 * A) / a);
		arc = calculate_arc_length(r, a);
		s = calculate_secant(r, a);
	}
	else if ((combi & CMB_SECANT) && (combi & CMB_ANGLE))
	{
		// User entered secant and angle
		s = *secant;
		a = degrees_to_radians(*deg_angle);
		r = s / (2 * sinf(a / 2));
		arc = calculate_arc_length(r, a);
		A = calculate_area(r, a);
	}
	else if ((combi & CMB_ANGLE) && (combi & CMB_RADIUS))
	{
		// User entered angle and radius
		r = *radius;
		a = degrees_to_radians(*deg_angle);
		arc = calculate_arc_length(r, a);
		A = calculate_area(r, a);
		s = calculate_secant(r, a);
	}
	else if ((combi & CMB_SECANT) && (combi & CMB_RADIUS))
	{
		// User entered secant and radius
		r = *radius;
		s = *secant;
		a = 2 * asinf(s / (2 * r));
		arc = calculate_arc_length(r, a);
		A = calculate_area(r, a);
	}
	else if ((combi & CMB_AREA) && (combi & CMB_RADIUS))
	{
		// User entered area and radius
		r = *radius;
		A = *area;
		a = (2 * A) / (r * r);
		arc = calculate_arc_length(r, a);
		s = calculate_secant(r, a);
	}
	else if ((combi & CMB_AREA) && (combi & CMB_ARC))
	{
		// User entered area and arc length
		arc = *arc_length;
		A = *area;
		r = (2 * A) / arc;
		a = (2 * A) / (r * r);
		s = calculate_secant(r, a);
	}
	else // Impossible combinations
	{
		messageboxf(MB_OK | MB_ICONERROR, L"Fehler!", L"Fehler: Mit den eingegebenen Werten ist es nicht möglich die anderen Werte auszurechnen!");
		return FALSE;
	}

	float angle_degrees = radians_to_degrees(a);

	// Error checking
	if (angle_degrees > 360.000f)
	{
		messageboxf(MB_OK | MB_ICONERROR, L"Fehler!", L"Fehler: Die eingegebenen Werte sind ungültig, der berechnete Winkel (%.3f) beträgt mehr als 360 Grad!", angle_degrees);
		return FALSE;
	}

	if (arc > (2 * (float)PI * r))
	{
		messageboxf(MB_OK | MB_ICONERROR, L"Fehler!", L"Fehler: Die eingegebenen Werte sind ungültig, die berechnete Bogenlänge (%.3f) ist größer als der Kreisumfang!", arc);
		return FALSE;
	}

	gCircleSegment.deg_angle = angle_degrees;
	gCircleSegment.radius = r;
	gCircleSegment.secant = s;
	gCircleSegment.area = A;
	gCircleSegment.arc = arc;

	return TRUE;
}