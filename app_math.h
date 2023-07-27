#ifndef APP_MATH_H
#define APP_MATH_H

#define CMB_AREA		0x01
#define CMB_SECANT		0x02
#define CMB_ARC			0x04
#define CMB_ANGLE		0x08
#define CMB_RADIUS		0x10

typedef struct CircleSegment
{
	float deg_angle;
	float radius;
	float secant;
	float area;
	float arc;

} CircleSegment;

typedef int BOOL; // No need to include windows.h for that

float radians_to_degrees(_In_ float radians);
float degrees_to_radians(_In_ float degrees);

BOOL calculate_everything(_In_opt_ float* area, _In_opt_ float* secant, _In_opt_ float* arc_length, _In_opt_ float* deg_angle, _In_opt_ float* radius);

#endif // APP_MATH_H

extern const double PI;
extern CircleSegment gCircleSegment;