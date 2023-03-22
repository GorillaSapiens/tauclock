#ifndef _INCLUDE_ASTRO_H_
#define _INCLUDE_ASTRO_H_

#include <time.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct αδ {
   double α; // right ascension, hrs
   double δ; // declination, degrees
};

struct φλ {
   double φ; // latitude (N/S, S neg)
   double λ; // longitude (E/W, W neg)
};

struct UTrs {
   double r; // rise time
   double s; // set time
   double cosH;
};

double time_t2julian(time_t t);

// Conversion of GST to UT
double ə13(double jd, double GST);

// Converting LST to GST
double ə15(double λ, double LST);

// Ecliptic to equatorial coordinate conversion
struct αδ ə27(double jd, double λ, double β);

// Rising and setting
struct UTrs ə33(double jd, struct φλ φλ, struct αδ αδ, double v);

// Calculating the position of the Sun
struct αδ ə46(double jd);

// Twilight
struct UTrs ə50(double jd, struct φλ φλ, struct αδ αδ, double horizon);

// Calculating the coordinates of a planet
struct αδ ə54(double jd, int planet);

// Calculating the Moon's position
struct αδ ə65(double jd);

#ifdef  __cplusplus
}
#endif
#endif
