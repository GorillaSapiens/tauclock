#ifndef _INCLUDE_ASTRO_H_
#define _INCLUDE_ASTRO_H_

#include <time.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct Aa {
   double A; // azimuth
   double a; // altitude
};

struct FD {
   double F; // % illuminated
   double D; // age of the moon
};

struct αδ {
   double α; // right ascension, hrs
   double δ; // declination, degrees
};

struct φλ {
   double φ; // latitude (N/S, S neg)
   double λ; // longitude (E/W, W neg)
};

#if 0
struct UTrs {
   double r; // rise time
   double s; // set time
   double cosH;
};
#endif

double time_t2julian(time_t t);

// Conversion of UT to Greenwich sidereal time (GST)
double ə12(double jd);

// Conversion of GST to UT
double ə13(double jd, double GST);

// Local sidereal time (LST)
double ə14(double GST, struct φλ φλ);

// Converting LST to GST
double ə15(double λ, double LST);

// Converting between right ascension and hour angle
double ə24(double jd, struct φλ φλ, struct αδ αδ);

// Equatorial to horizon coordinate conversion
struct Aa ə25(double jd, struct φλ φλ, struct αδ αδ);

// Ecliptic to equatorial coordinate conversion
struct αδ ə27(double jd, double λ, double β);

#if 0
// Rising and setting
struct UTrs ə33(double jd, struct φλ φλ, struct αδ αδ, double v);
#endif

// Calculating the position of the Sun
struct αδ ə46(double jd);

#if 0
// Twilight
struct UTrs ə50(double jd, struct φλ φλ, struct αδ αδ, double horizon);
#endif

// Calculating the coordinates of a planet
struct αδ ə54(double jd, int planet);

// Calculating the Moon's position
struct αδ ə65(double jd);

// The phases of the Moon
struct FD ə67(double jd);

#define ZRANGE(x, high)                                        \
   do {                                                        \
      if ((x) < 0.0) {                                         \
         (x) += (double)(1 - (int)((x) / (high))) * (high);    \
      }                                                        \
      if ((x) >= high) {                                       \
         (x) -= (double)((int)((x) / (high))) * (high);        \
      }                                                        \
   } while(0)

#ifdef  __cplusplus
}
#endif
#endif