#ifdef TEST
#include <stdio.h>
#endif

#include "trig1.h"
#include "astro.h"

#ifndef INFINITY
#define INFINITY (__builtin_inff ())
#endif

double time_t2julian(time_t t) {
   // https://stackoverflow.com/questions/466321/convert-unix-timestamp-to-julian
   //
   // The Unix epoch (zero-point) is January 1, 1970 GMT.
   // That corresponds to the Julian day of 2440587.5

   return (double)t / 86400.0 + 2440587.5;
}

#define ZRANGE(x, high)                                        \
   do {                                                        \
      if ((x) < 0.0) {                                         \
         (x) += (double)(1 - (int)((x) / (high))) * (high);    \
      }                                                        \
      if ((x) >= high) {                                       \
         (x) -= (double)((int)((x) / (high))) * (high);        \
      }                                                        \
   } while(0)

// Conversion of GST to UT
double ə13(double jd, double GST) {
   double S = jd - 2451545.0;
   double T = S / 36525.0;
   double T0 = 6.697374558 + (2400.051336 *T) + (0.000025862 * T * T);
   ZRANGE(T0, 24.0);

   GST -= T0;
   ZRANGE(GST, 24.0);

   double UT = GST * 0.9972695663;

   return UT;
}

// Converting LST to GST
double ə15(double λ, double LST) {
   LST -= λ / 15.0;
   ZRANGE(LST, 24.0);
   return LST;
}

// Ecliptic to equatorial coordinate conversion
struct αδ ə27(double jd, double λ, double β) {
   // tested, works
   double T = (jd - 2451545.0) / 36525.0;
   double DE = (46.815 * T + 0.0006 * T * T - 0.00181*T*T*T) / 3600.0;
   double ε = 23.439292 - DE;

   double sinδ = sin_deg(β)*cos_deg(ε) + cos_deg(β)*sin_deg(ε)*sin_deg(λ);

   struct αδ αδ;

   αδ.δ = asin_deg(sinδ);

   αδ.α =
      atan2_deg(
         (sin_deg(λ)*cos_deg(ε) - tan_deg(β)*sin_deg(ε)),
         cos_deg(λ));

   αδ.α /= 15.0; // convert to hours

   return αδ;
}

// Rising and setting
struct UTrs ə33(double jd, struct φλ φλ, struct αδ αδ, double v) {
printf("jd=%f\n", jd);
printf("φλ=%f,%f\n", φλ.φ, φλ.λ);
printf("αδ=%f,%f\n", αδ.α, αδ.δ);
printf("v=%f\n", v);

   double cosH =
      -(
         (sin_deg(v) + sin_deg(φλ.φ) * sin_deg(αδ.δ)) /
         (cos_deg(φλ.φ) * cos_deg(αδ.δ))
       );
printf("cosH=%f\n", cosH);

   struct UTrs UTrs;

   if (cosH < -1.0) {
      // circumpolar
      UTrs.r = +INFINITY;
      UTrs.s = +INFINITY;
   }
   else if (cosH > 1.0) {
      // never rises
      UTrs.r = -INFINITY;
      UTrs.s = -INFINITY;
   }
   else {
      double H = acos_deg(cosH) / 15.0;
printf("H=%f\n", H);

      double LSTr = αδ.α - H;
      ZRANGE(LSTr, 24.0);
printf("LSTr=%f\n", LSTr);
      double LSTs = αδ.α + H;
      ZRANGE(LSTs, 24.0);
printf("LSTs=%f\n", LSTs);

      double GSTr = ə15(φλ.λ, LSTr);
      ZRANGE(GSTr, 24.0);
printf("GSTr=%f\n", GSTr);
      double GSTs = ə15(φλ.λ, LSTs);
      ZRANGE(GSTs, 24.0);
printf("GSTs=%f\n", GSTs);

      UTrs.r = ə13(jd, GSTr);
      UTrs.s = ə13(jd, GSTs);
   }
   return UTrs;
}

// Calculating the position of the Sun
struct αδ ə46(double jd) {
   // tested, works

   // 2010 January 0.0 (JD = 2 455 196.5)
   static const double epoch = 2455196.5;

   // εg (ecliptic longitude at epoch 2010.0) = 279.557208 degrees
   static const double εg = 279.557208;

   // ϖg (ecliptic longitude of perigee at epoch 2010.0) = 283.112438 degrees
   static const double ϖg = 283.112438;

   // e (eccentricity of orbit at epoch 2010.0) = 0.016705
   static const double e  = 0.016705;

   // // r0 (semi-major axis) = 1.495985×10^8 km
   // static const double r0 = 1.495985e8;

   // // θ0 (angular diameter at r = r0) = 0.533128 degrees
   // static const double θ0 = 0.533128;

   // ==================

   double D = jd - epoch;

   double N = (360.0 * D / 365.242191);
   ZRANGE(N, 360.0);

   double M = N + εg - ϖg;
   ZRANGE(M, 360.0);

   double Ec = (360.0 / M_PI) * e * sin_deg(M);

   double λ = N + Ec + εg;
   ZRANGE(λ, 360.0);

   return ə27(jd, λ, 0.0);
}

#ifdef TEST

#include <stdio.h>
#include <math.h>
#include <assert.h>

int close(double a, double b, double delta) {
   printf("close %f %f %f\n", a, b, delta);
   return fabs(a-b) < delta;
}

int main(int argc, char **argv) {
   double tmp;
   struct αδ αδ;
   struct UTrs UTrs;

   // double ə13(double jd, double GST) {
   tmp = ə13(2444351.5, 4.668119);
   assert(close(tmp, 14.614353, 0.001));

   // double ə15(double λ, double LST) {
   tmp = ə15(-64.0, 0.401453);
   assert(close(tmp, 4.668119, 0.001));

   // struct αδ ə27(double jd, double λ, double β) {
   αδ = ə27(2455018.5, 139.686111, 4.875278);
   assert(close(αδ.α, 9.581478, 0.001));
   assert(close(αδ.δ, 19.535003, 0.02));

   // struct UTrs ə33(double jd, struct φλ φλ, struct αδ αδ, double v) {
   UTrs = ə33(2455432.5,
               (struct φλ) { 30.0, 64.0 },
               (struct αδ) { 23.655558, 21.700000 },
               0.5666666666);
   assert(close(UTrs.r,  14.271670, 0.0015));
   assert(close(UTrs.s,  4.166990, 0.0015));

   // struct αδ ə46(double jd) {
   αδ = ə46(2455196.5 - 2349);
   assert(close(αδ.α, 8.39277777, 0.001));
   assert(close(αδ.δ, 19.35277777, 0.01));
}

#endif
