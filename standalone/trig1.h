#ifndef _TRIG1_H_
#define _TRIG1_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef M_PI
static const double M_PI = 3.14159265358979323846;
static const double M_PI_2 = 1.57079632679489661923;
static const double M_PI_4 = 0.785398163397448309616;
#endif

/// @brief Integer trigonometric sine fucntion
///⏎
/// here angle is:
///   -32768 for -2*PI
///   0 for 0
///   +32768 (which is really 0) for 2*PI
///
/// the return value is
///   -32768 for -1.0
///   0 for 0.0
///   +32768 for 1.0
///
/// @param angle expressed as int16_t
/// @returns sin() in range -32767 to +32767
int64_t sin1(int16_t angle);

/// @brief Floating point trigonometric sine function
///
/// @param angle in degrees
/// @returns sine of the angle
double sin_deg(double angle);

/// @brief Integer trigonometric cosine fucntion
///⏎
/// here angle is:
///   -32768 for -2*PI
///   0 for 0
///   +32768 (which is really 0) for 2*PI
///
/// the return value is
///   -32767 for -1.0
///   0 for 0.0
///   +32767 for 1.0
///
/// @param angle expressed as int16_t
/// @returns sin() in range -32767 to +32767
int64_t cos1(int16_t angle);

/// @brief Floating point trigonometric cosine function
///
/// @param angle in degrees
/// @returns cosine of the angle
double cos_deg(double angle);

double tan_deg(double angle);
double asin_deg(double sine);
double acos_deg(double cosine);
double atan_deg(double tangent);
double atan2_deg(double y, double x);

#ifdef  __cplusplus
}
#endif
#endif
