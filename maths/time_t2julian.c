#include "time_t2julian.h"

// https://stackoverflow.com/questions/466321/convert-unix-timestamp-to-julian
//
// The Unix epoch (zero-point) is January 1, 1970 GMT.
// That corresponds to the Julian day of 2440587.5

double time_t2julian(time_t t) {
   return (double)t / 86400.0 + 2440587.5;
}

#ifdef STANDALONE
#include <stdio.h>

int main (int argc, char **argv) {
   printf("jd=%f\n", time_t2julian(time(NULL)));
}
#endif
