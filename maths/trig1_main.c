#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include "trig1.h"

/// @brief Generate lookup table for sin1()
void generate_tables() {
   int i;
   printf("static const uint16_t sin90[8192] = {\n   ");
   for (i = 0; i < 8192; i++) {
      int16_t n =
         (int16_t)
         round(32768.0 * sin((double) i * M_PI / (2.0 * 8192.0)));

      printf("0x%04x", (uint16_t) n);

      if (i != 8191) {
         printf(",");
         if (!((i+1)%8)) {
            printf("\n   ");
         }
         else {
            printf(" ");
         }
      }
      else {
         printf("\n};\n");
      }
   }
}

/// @brief test comparing the real floating point sine with the sin1 function.
void run() {
   double angle;
   double total_error = 0.0;
   printf("%6s, %8s, %8s, %6s\n", "angle", "sin", "sin1", "error");
   for (angle = 0; angle < 360; angle += 0.1) {
      double lookup_sine = sin1(angle * 32768.0 / 360.0) / 32768.0;
      double real_sine = sin(angle * 2 * M_PI / 360.0);
      double sine_error = real_sine - lookup_sine;
      printf("%6.1f, %+8.5f, %+8.5f, %+8.6f\n", angle, real_sine, lookup_sine,
             sine_error);
      total_error += (sine_error * sine_error);
   }
   printf("error^2=%f\n", total_error);
}

/// @brief a silly useless function
/// @param argc The number of args
/// @param argv argument values
int main(int argc, char **argv) {
   if (argc < 2) {
      fprintf(stderr, "Usage %s -g          : Create lookup tables\n", argv[0]);
      fprintf(stderr, "      %s <int angle> : Test a number (0 to 32767)\n",
              argv[0]);
      fprintf(stderr, "      %s -r          : Run iterated numbers test\n",
              argv[0]);
      return 1;
   }
   else if (!strcmp(argv[1], "-g")) {
      generate_tables();
   }
   else if (!strcmp(argv[1], "-r")) {
      run();
   }
   else if (!isnan(atof(argv[1]))) {
      int16_t l = (int16_t)atof(argv[1]);
      printf("s=%d\n", (int)sin1(l));
      printf("c=%d\n", (int)cos1(l));
   }
   return 0;
}