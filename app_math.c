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

#define MAX_ITERATIONS 1000
#define EPSILON 1e-6

float f_secant_arc(float x, float s, float b)
{
	return s * 1 / (2 * sinf(b / (2 * x))) - x;
}

float f_prime_secant_arc(float x, float s, float b)
{
	float term1 = -s * (1 / (2 * sinf(b / (2 * x)))) * (1 / (2 * cosf(b / (2 * x))));
	float term2 = 1;
	return term1 + term2;
}

float newton_raphson_secant_arc(float x0, float s, float b)
{
	float x = x0;
	int iterations = 0;

	while (iterations < MAX_ITERATIONS)
	{
		float fx = f_secant_arc(x, s, b);
		float f_prime_x = f_prime_secant_arc(x, s, b);
		float x1 = x - fx / f_prime_x;

		if (fabsf(x1 - x) < EPSILON)
		{
			return x1;
		}

		x = x1;
		iterations++;
	}

	return x; // Return the approximation if maximum iterations are reached
}

float find_initial_guess_secant_arc(float s, float b, float step_size)
{
	float x = step_size;
	float x_prev = 0;
	float fx, fx_prev;

	do {
		x_prev = x;
		fx_prev = f_secant_arc(x_prev, s, b);
		x += step_size;
		fx = f_secant_arc(x, s, b);
	} while (fx * fx_prev > 0 && x < 1000); // Search for opposite signs or until x gets too large

	return (x + x_prev) / 2; // Return the midpoint of the last interval
}

float f_secant_area(float x, float s, float A)
{
	return s * (1 / (2 * sinf(A / (4 * x * x)))) - x;
}

float f_prime_secant_area(float x, float s, float A)
{
	float term1 = -s * (1 / (2 * sinf(A / (4 * x * x)))) * (1 / (2 * cosf(A / (4 * x * x)))) * (A / 2 * x);
	float term2 = 1;
	return term1 + term2;
}

float newton_raphson_secant_area(float x0, float s, float A)
{
	float x = x0;
	int iterations = 0;

	while (iterations < MAX_ITERATIONS)
	{
		float fx = f_secant_area(x, s, A);
		float f_prime_x = f_prime_secant_area(x, s, A);
		float x1 = x - fx / f_prime_x;

		if (fabsf(x1 - x) < EPSILON)
		{
			return x1;
		}

		x = x1;
		iterations++;
	}

	return x; // Return the approximation if maximum iterations are reached
}

float find_initial_guess_secant_area(float s, float A, float step_size)
{
	float x = step_size;
	float x_prev = 0;
	float fx, fx_prev;

	do {
		x_prev = x;
		fx_prev = f_secant_area(x_prev, s, A);
		x += step_size;
		fx = f_secant_area(x, s, A);
	} while (fx * fx_prev > 0 && x < 1000); // Search for opposite signs or until x gets too large

	return (x + x_prev) / 2; // Return the midpoint of the last interval
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
	else if ((combi & CMB_SECANT) && (combi & CMB_ARC))
	{
		s = *secant;
		arc = *arc_length;
		float x0 = find_initial_guess_secant_arc(s, arc, 0.1f);
		r = newton_raphson_secant_arc(x0, s, arc);
		a = arc / r;
		A = calculate_area(r, a);
	}
	else if ((combi & CMB_AREA) && (combi & CMB_SECANT))
	{
		s = *secant;
		A = *area;
		float x0 = find_initial_guess_secant_area(s, A, 0.1f);
		r = newton_raphson_secant_area(x0, s, A);
		a = A / (0.5f * r * r);
		arc = calculate_arc_length(r, a);
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