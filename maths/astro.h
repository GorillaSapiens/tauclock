#ifndef _INCLUDE_ASTRO_H_
#define _INCLUDE_ASTRO_H_

#include <time.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct αδ {
   double α;
   double δ;
};

double time_t2julian(time_t t);

// Conversion of GST to UT
double ə13(double jd, double GST);

// Converting LST to GST
double ə15(double λ, double LST);

// Ecliptic to equatorial coordinate conversion
struct αδ ə27(double jd, double λ, double β);

// Rising and setting
struct αδ ə33(double jd, double λ, double β);

// Calculating the position of the Sun
struct αδ ə46(double jd);

#ifdef  __cplusplus
}
#endif
#endif
