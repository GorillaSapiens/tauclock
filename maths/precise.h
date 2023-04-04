#ifndef _INCLUDE_PRECISE_H_
#define _INCLUDE_PRECISE_H_

#include <math.h>

double sin_deg(double angle);
double cos_deg(double angle);
double tan_deg(double angle);
double asin_deg(double sine);
double acos_deg(double cosine);
double atan_deg(double tangent);
double atan2_deg(double y, double x);

// disallow usage of stack radiant based functions
#define FNORD KNARF
#define sin(x) FNORD
#define cos(x) FNORD
#define tan(x) FNORD

#endif // _INCLUDE_PRECISE_H_

// vim: expandtab:noai:ts=3:sw=3
