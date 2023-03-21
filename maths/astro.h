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

struct αδ sec27(double jd, double λ, double β);
struct αδ sec46(double jd);

#ifdef  __cplusplus
}
#endif
#endif
