#include "trig1.h"
#include "astro.h"

#ifndef INFINITY
#define INFINITY (__builtin_inff ())
#endif

static const
struct Elements {
   double Tp;     // (tropical years)
   double ε;      // (degrees)
   double ϖ;      // (degrees)
   double e;
   double a;      // (AU)
   double i;      // (degrees)
   double Ω;      // (degrees)
   double θ0;     // (arcsec)
   double V0;
} planets[] = {
/* Mercury */ {  0.24085,    75.5671,    77.612,    0.205627,  0.387098, 7.0051,    48.449,      6.74, -0.42 },
/* Venus*/    {  0.615207,  272.30044,  131.54,     0.006812,  0.723329, 3.3947,    76.769,     16.92, -4.40 },
/* Earth*/    {  0.999996,   99.556772, 103.2055,   0.016671,  0.999985 },
/* Mars*/     {  1.880765,  109.09646,  336.217,    0.093348,  1.523689, 1.8497,    49.632,      9.36, -1.52 },
/* Jupiter*/  {  11.857911, 337.917132,  14.6633,   0.048907,  5.20278,  1.3035,   100.595,    196.74, -9.40 },
/* Saturn*/   {  29.310579, 172.398316,  89.567,    0.053853,  9.51134,  2.4873,   113.752,    165.60, -8.88 },
/* Uranus*/   {  84.039492, 356.135400, 172.884833, 0.046321, 19.21814,  0.773059,  73.926961,  65.80, -7.19 },
/* Neptune*/  { 165.84539,  326.895127,  23.07,     0.010483, 30.1985,   1.7673,   131.879,     62.20, -6.87 }
};

double time_t2julian(time_t t) {
   // https://stackoverflow.com/questions/466321/convert-unix-timestamp-to-julian
   //
   // The Unix epoch (zero-point) is January 1, 1970 GMT.
   // That corresponds to the Julian day of 2440587.5

   return (double)t / 86400.0 + 2440587.5;
}

// Conversion of UT to Greenwich sidereal time (GST)
double ə12(double jd) {
   double jd0 = (int)jd;
   double frac = jd - (double)((int)jd);
   if (frac >= 0.5) {
      jd0 += .5;
   }
   else {
      jd0 -= .5;
   }
   double UT = (jd - jd0) * 24.0;

   double S = jd0 - 2451545.0;
   double T = S / 36525.0;
   double T0 = 6.697374558 + (2400.051336 * T) + (0.000025862 * T * T);
   ZRANGE(T0, 24.0);

   double GST = T0 + UT * 1.002737909;
   ZRANGE(GST, 24.0);

   return GST;
}

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

// Local sidereal time (LST)
double ə14(double GST, struct φλ φλ) {
   double LST = GST + φλ.λ / 15.0;
   ZRANGE(LST, 24.0);
   return LST;
}

// Converting LST to GST
double ə15(double λ, double LST) {
   LST -= λ / 15.0;
   ZRANGE(LST, 24.0);
   return LST;
}

// Converting between right ascension and hour angle
double ə24(double jd, struct φλ φλ, struct αδ αδ) {
   double GST = ə12(jd);
   double LST = ə14(GST, φλ);
   double H = LST - αδ.α;
   ZRANGE(H, 24.0);
   return H;
}

// Equatorial to horizon coordinate conversion
struct Aa ə25(double jd, struct φλ φλ, struct αδ αδ) {
   double H = ə24(jd, φλ, αδ);
   H *= 15.0;
   double sina =
      sin_deg(αδ.δ) * sin_deg(φλ.φ) +
      cos_deg(αδ.δ) * cos_deg(φλ.φ) * cos_deg(H);
   double a = asin_deg(sina);
   double cosA =
      (sin_deg(αδ.δ) - sin_deg(φλ.φ) * sin_deg(a)) /
      (cos_deg(φλ.φ) * cos_deg(a));
   double A = acos_deg(cosA);
   double sinH = sin_deg(H);
   if (sinH >= 0.0) {
      A = 360.0 - A;
   }
   return (struct Aa) { A, a };
}

// Ecliptic to equatorial coordinate conversion
struct αδ ə27(double jd, double λ, double β) {
   double T = (jd - 2451545.0) / 36525.0;
   double DE = (46.815 * T + 0.0006 * T * T - 0.00181*T*T*T) / 3600.0;
   double ε = 23.439292 - DE;

   double sinδ = sin_deg(β)*cos_deg(ε) + cos_deg(β)*sin_deg(ε)*sin_deg(λ);

   struct αδ αδ;

   αδ.δ = asin_deg(sinδ);

   αδ.α =
      atan2_deg(
         (sin_deg(λ) * cos_deg(ε) - tan_deg(β) * sin_deg(ε)),
         cos_deg(λ));

   αδ.α /= 15.0; // convert to hours
   ZRANGE(αδ.α, 24.0);

   return αδ;
}

#if 0
// Rising and setting
struct UTrs ə33(double jd, struct φλ φλ, struct αδ αδ, double v) {
   struct UTrs UTrs;

   UTrs.cosH =
      -(
         (sin_deg(v) + sin_deg(φλ.φ) * sin_deg(αδ.δ)) /
         (cos_deg(φλ.φ) * cos_deg(αδ.δ))
       );

   if (UTrs.cosH < -1.0) {
      // circumpolar
      UTrs.r = +INFINITY;
      UTrs.s = +INFINITY;
   }
   else if (UTrs.cosH > 1.0) {
      // never rises
      UTrs.r = -INFINITY;
      UTrs.s = -INFINITY;
   }
   else {
      double H = acos_deg(UTrs.cosH) / 15.0;

      double LSTr = αδ.α - H;
      ZRANGE(LSTr, 24.0);
      double LSTs = αδ.α + H;
      ZRANGE(LSTs, 24.0);

      double GSTr = ə15(φλ.λ, LSTr);
      ZRANGE(GSTr, 24.0);
      double GSTs = ə15(φλ.λ, LSTs);
      ZRANGE(GSTs, 24.0);

      UTrs.r = ə13(jd, GSTr);
      UTrs.s = ə13(jd, GSTs);
   }
   return UTrs;
}
#endif

// Calculating the position of the Sun
struct αδ ə46(double jd) {
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

#if 0
// Twilight
struct UTrs ə50(double jd, struct φλ φλ, struct αδ αδ, double horizon) {
   struct UTrs UTrs;

   UTrs.cosH =
      (
         (cos_deg(horizon) - sin_deg(φλ.φ) * sin_deg(αδ.δ) - sin_deg(0.566666666)) /
         (cos_deg(φλ.φ) * cos_deg(αδ.δ))
       );

   if (UTrs.cosH < -1.0) {
      // circumpolar
      UTrs.r = +INFINITY;
      UTrs.s = +INFINITY;
   }
   else if (UTrs.cosH > 1.0) {
      // never rises
      UTrs.r = -INFINITY;
      UTrs.s = -INFINITY;
   }
   else {
      double H = acos_deg(UTrs.cosH) / 15.0;

      double LSTr = αδ.α - H;
      ZRANGE(LSTr, 24.0);
      double LSTs = αδ.α + H;
      ZRANGE(LSTs, 24.0);

      double GSTr = ə15(φλ.λ, LSTr);
      ZRANGE(GSTr, 24.0);
      double GSTs = ə15(φλ.λ, LSTs);
      ZRANGE(GSTs, 24.0);

      UTrs.r = ə13(jd, GSTr);
      UTrs.s = ə13(jd, GSTs);
   }
   return UTrs;
}
#endif

static void ə54_helper(double jd, const struct Elements *elem,
      double *l, double *r) {

   // 2010 January 0.0 (JD = 2 455 196.5)
   static const double epoch = 2455196.5;

   double M = (360.0 / 365.242191) *
      ((jd - epoch) / elem->Tp) +
      elem->ε -
      elem->ϖ;
   ZRANGE(M, 360.0);

   double ν = M + (360.0 / M_PI) * elem->e * sin_deg(M);
   ZRANGE(ν, 360.0);

   *l = ν + elem->ϖ;
   ZRANGE(*l, 360.0);

   *r = ((elem->a * (1 - elem->e * elem->e)) /
         (1 + elem->e * cos_deg(ν)));
}

// Calculating the coordinates of a planet
struct αδ ə54(double jd, int planet) {

   const struct Elements *elem = planets + planet;
   const struct Elements *earth = planets + 2;

   double l, r;
   double L, R;

   ə54_helper(jd, elem, &l, &r);
   ə54_helper(jd, earth, &L, &R);

   double ψ = asin_deg(sin_deg(l - elem->Ω) * sin_deg(elem->i));

   double lp = atan2_deg (
                  sin_deg(l - elem->Ω) * cos_deg(elem->i),
                  cos_deg(l - elem->Ω)) + elem->Ω;

   double rp = r*cos_deg(ψ);

   double λ;

   if (planet < 2) { // inner planet
      λ = atan2_deg(
         rp * sin_deg(L - lp),
         R - rp * cos_deg(L - lp)) + 180.0 + L;
   }
   else { // outer planet
      λ = atan_deg(
         (R * sin_deg(lp - L)) / 
         (rp - R * cos_deg(lp - L))) + lp;
   }

   double β = atan_deg(
               (rp * tan_deg(ψ) * sin_deg(λ - lp)) /
               (R * sin_deg(lp - L)));

   struct αδ αδ = ə27(jd, λ, β);
   return αδ;
}

// Calculating the Moon's position
struct αδ ə65(double jd) {
   // much of this copied from ə46

   static const double epoch = 2455196.5;
   static const double εg_sun = 279.557208;
   static const double ϖg_sun = 283.112438;
   static const double e_sun  = 0.016705;

   double D = jd - epoch;

   double N_sun = (360.0 * D / 365.242191);
   ZRANGE(N_sun, 360.0);
   double M_sun = N_sun + εg_sun - ϖg_sun;
   ZRANGE(M_sun, 360.0);
   double Ec_sun = (360.0 / M_PI) * e_sun * sin_deg(M_sun);
   double λ_sun = N_sun + Ec_sun + εg_sun;
   ZRANGE(λ_sun, 360.0);

   static const double l0 = 91.929336;
   static const double P0 = 130.143076;
   static const double N0 = 291.682547;
   static const double i = 5.145396;

   double l = 13.1763966 * D + l0;
   ZRANGE(l, 360.0);

   double M_moon = l - 0.1114041 * D - P0;
   ZRANGE(M_moon, 360.0);

   double N = N0 - 0.0529539 * D;
   ZRANGE(N, 360.0);

   double C = l - λ_sun;
   double Ev = 1.2739 * sin_deg(2.0 * C - M_moon);
   double Ae = 0.1858 * sin_deg(M_sun);
   double A3 = 0.37 * sin_deg(M_sun);

   double Mp_moon = M_moon + Ev - Ae - A3;
   double Ec = 6.2886 * sin_deg(Mp_moon);
   double A4 = 0.214 * sin_deg(2.0 * Mp_moon);
   double lp = l + Ev + Ec - Ae + A4;

   double V = 0.6583 * sin_deg(2.0 * (lp - λ_sun));
   double lpp = lp + V;

   double Np = N - 0.16 * sin_deg(M_sun);
   double λ_moon = atan2_deg(
                        sin_deg(lpp - Np) * cos_deg(i),
                        cos_deg(lpp - Np)
                     ) + Np;
   double β_moon = asin_deg(sin_deg(lpp-Np)*sin_deg(i));

   return ə27(jd, λ_moon, β_moon);
}

// The phases of the Moon
struct FD ə67(double jd) {
   // much of this copied from ə65

   static const double epoch = 2455196.5;
   static const double εg_sun = 279.557208;
   static const double ϖg_sun = 283.112438;
   static const double e_sun  = 0.016705;

   double D = jd - epoch;

   double N_sun = (360.0 * D / 365.242191);
   ZRANGE(N_sun, 360.0);
   double M_sun = N_sun + εg_sun - ϖg_sun;
   ZRANGE(M_sun, 360.0);
   double Ec_sun = (360.0 / M_PI) * e_sun * sin_deg(M_sun);
   double λ_sun = N_sun + Ec_sun + εg_sun;
   ZRANGE(λ_sun, 360.0);

   static const double l0 = 91.929336;
   static const double P0 = 130.143076;
   static const double N0 = 291.682547;
   // static const double i = 5.145396;

   double l = 13.1763966 * D + l0;
   ZRANGE(l, 360.0);

   double M_moon = l - 0.1114041 * D - P0;
   ZRANGE(M_moon, 360.0);

   double N = N0 - 0.0529539 * D;
   ZRANGE(N, 360.0);

   double C = l - λ_sun;
   double Ev = 1.2739 * sin_deg(2.0 * C - M_moon);
   double Ae = 0.1858 * sin_deg(M_sun);
   double A3 = 0.37 * sin_deg(M_sun);

   double Mp_moon = M_moon + Ev - Ae - A3;
   double Ec = 6.2886 * sin_deg(Mp_moon);
   double A4 = 0.214 * sin_deg(2.0 * Mp_moon);
   double lp = l + Ev + Ec - Ae + A4;

   double V = 0.6583 * sin_deg(2.0 * (lp - λ_sun));
   double lpp = lp + V;

//   double Np = N - 0.16 * sin_deg(M_sun);
//   double λ_moon = atan2_deg(
//                        sin_deg(lpp - Np) * cos_deg(i),
//                        cos_deg(lpp - Np)
//                     ) + Np;
//   double β_moon = asin_deg(sin_deg(lpp-Np)*sin_deg(i));

   // NB, this new D is **NOT** the D from above!
   struct FD FD;

   FD.D = lpp - λ_sun;
   ZRANGE(FD.D, 360.0);

   FD.F = 0.5 * (1 - cos_deg(FD.D));

   return FD;
}

#ifdef TEST

#include <stdio.h>
#include <math.h>
#include <assert.h>

#ifndef VERBOSE
#define printf(format, ...)
#endif

int close(double a, double b, double delta) {
   printf("close %f %f %f\n", a, b, delta);
   return fabs(a-b) < delta;
}

int old_main(int argc, char **argv) {
   // double ə12(double jd)
   {
      printf("test ə12\n");
      double tmp = ə12(2444351.5 + 14.614353 / 24.0);
      assert(close(tmp, 4.668119444, 0.001));
      printf("pass\n");
   }

   // double ə13(double jd, double GST)
   {
      printf("test ə13\n");
      double tmp = ə13(2444351.5, 4.668119);
      assert(close(tmp, 14.614353, 0.001));
      printf("pass\n");
   }

   // double ə14(double GST, struct φλ φλ)
   {
      printf("test ə14\n");
      double tmp = ə14(4.668119, (struct φλ) { 0.0, -64.0 });
      assert(close(tmp, 0.401453, 0.001));
      printf("pass\n");
   }

   // double ə15(double λ, double LST)
   {
      printf("test ə15\n");
      double tmp = ə15(-64.0, 0.401453);
      assert(close(tmp, 4.668119, 0.001));
      printf("pass\n");
   }

   // double ə24(double jd, struct φλ φλ, struct αδ αδ)
   {
      printf("test ə24\n");
      double tmp = ə24(2444351.5 + +  18.614353 / 24.0,
            (struct φλ) { 0.0, -64.0 },
            (struct αδ) { 18.539167, 0.0 });
      assert(close(tmp, 9.873237, 0.001));
      printf("pass\n");
   }

   // struct Aa ə25(double jd, struct φλ φλ, struct αδ αδ)
   {
      printf("test ə25\n");
      struct Aa Aa = ə25(2451621.5,
                         (struct φλ){52.0, 0.0},
                         (struct αδ){5.862222 , 23.219444});
      assert(close(Aa.A, 283.271027, 0.01));
      assert(close(Aa.a, 19.334345, 0.01));
      printf("pass\n");
   }

   // struct αδ ə27(double jd, double λ, double β)
   {
      printf("test ə27\n");
      struct αδ αδ = ə27(2455018.5, 139.686111, 4.875278);
      assert(close(αδ.α, 9.581478, 0.001));
      assert(close(αδ.δ, 19.535003, 0.02));
      printf("pass\n");
   }

#if 0
   // struct UTrs ə33(double jd, struct φλ φλ, struct αδ αδ, double v)
   {
      printf("test ə33\n");
      struct UTrs UTrs = ə33(2455432.5,
                             (struct φλ) { 30.0, 64.0 },
                             (struct αδ) { 23.655558, 21.700000 },
                             0.5666666666);
      assert(close(UTrs.r, 14.271670, 0.0015));
      assert(close(UTrs.s, 4.166990, 0.0015));
      printf("pass\n");
   }
#endif

   // struct αδ ə46(double jd)
   {
      printf("test ə46\n");
      struct αδ αδ = ə46(2455196.5 - 2349);
      assert(close(αδ.α, 8.39277777, 0.001));
      assert(close(αδ.δ, 19.35277777, 0.01));
      printf("pass\n");
   }

#if 0
   // struct UTrs ə50(double jd, struct φλ φλ, struct αδ αδ, double horizon)
   {
      printf("test ə50\n");
      struct αδ αδ = ə46(2444124.0);
      struct UTrs UTrs = ə50(2444124.0,
                             (struct φλ) { 52.0, 0.0 },
                             αδ,
                             108.0);
      printf("δ=%f\n", αδ.δ);
      assert(close(UTrs.r, 3.209, 0.01));
      assert(close(UTrs.s, 20.662838, 0.01));
      printf("pass\n");
   }
#endif

   // struct αδ ə54(double jd, int planet)
   {
      {
         printf("test ə54 (inner)\n");
         struct αδ αδ = ə54(2452965.5, 4);
         assert(close(αδ.α, 11.18722222, 0.002));
         assert(close(αδ.δ, 6.356944444, 0.02));
         printf("pass\n");
      }
      {
         printf("test ə54 (outer)\n");
         struct αδ αδ = ə54(2452965.5, 0);
         assert(close(αδ.α, 16.82, 0.002));
         assert(close(αδ.δ, -24.5025, 0.02));
         printf("pass\n");
      }
   }

   // struct αδ ə65(double jd)
   {
      printf("test ə65\n");
      struct αδ αδ = ə65(2452883.50000);
      assert(close(αδ.α, 14.2166667, 0.02));
      assert(close(αδ.δ, -11.52722222, 0.02));
      printf("pass\n");
   }

   // struct FD ə67(double jd)
   {
      printf("test ə67\n");
      struct FD FD = ə67(2452883.5);
      assert(close(FD.D, 56.622971, 0.02));
      assert(close(FD.F, 0.225, 0.02));
      printf("pass\n");
   }

   return 0;
}

int main(int argc, char **argv) {
   for (int i = 0; i < 10 * 24 * 60 * 60; i++) {
      old_main(argc, argv);
   }
}
#endif

#ifdef ONEDAY
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct Aa Aa[8][24*60];
const char *names[8] = {
   "solar",
   "lunar",
   "mercury",
   "venus",
   "mars",
   "jupiter",
   "saturn",
   "aries"
};

int main(int argc, char **argv) {
   time_t now = time(NULL);
   now /= 60;
   now *= 60;

   struct φλ φλ = { atof(argv[1]), atof(argv[2]) };

   int max[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

   for (int i = 0; i < 24*60; i++) {
      time_t when = now - (12*60*60) + i*60;
      double jd = time_t2julian(when);

      Aa[0][i] = ə25(jd, φλ, ə46(jd)); // sun
      Aa[1][i] = ə25(jd, φλ, ə65(jd)); // moon
      Aa[2][i] = ə25(jd, φλ, ə54(jd, 0)); // mercury
      Aa[3][i] = ə25(jd, φλ, ə54(jd, 1)); // venus
      Aa[4][i] = ə25(jd, φλ, ə54(jd, 3)); // mars
      Aa[5][i] = ə25(jd, φλ, ə54(jd, 4)); // jupiter
      Aa[6][i] = ə25(jd, φλ, ə54(jd, 5)); // saturn
      Aa[7][i] = ə25(jd, φλ, (struct αδ) { 0.0, 0.0 }); // aries

#if 0
      printf("==\n");
      printf("%f %f %s\n", Aa[0][i].A, Aa[0][i].a, names[0]);
      printf("%f %f %s\n", Aa[1][i].A, Aa[1][i].a, names[1]);
      printf("%f %f %s\n", Aa[2][i].A, Aa[2][i].a, names[2]);
      printf("%f %f %s\n", Aa[3][i].A, Aa[3][i].a, names[3]);
      printf("%f %f %s\n", Aa[4][i].A, Aa[4][i].a, names[4]);
      printf("%f %f %s\n", Aa[5][i].A, Aa[5][i].a, names[5]);
      printf("%f %f %s\n", Aa[6][i].A, Aa[6][i].a, names[6]);
      printf("%f %f %s\n", Aa[7][i].A, Aa[7][i].a, names[7]);
#endif

      for (int j = 0; j < 8; j++) {
         if (max[j] == -1 || Aa[j][i].a > Aa[j][max[j]].a) {
            max[j] = i;
         }
      }
   }

#define HORIZON        0.00
#define ASTRONOMICAL -18.56
#define NAUTICAL     -12.56
#define CIVIL         -6.56
#define SOLAR         -1.00
#define LUNAR         -1.00

   for (int i = 0; i < 24*60; i++) {
      time_t when = now - (12*60*60) + i*60;
      if (i == 0) {
         if (Aa[0][0].a > ASTRONOMICAL) {
            printf("astro up\t: %s", ctime(&when));
         }
         else {
            printf("astro down\t: %s", ctime(&when));
         }
         if (Aa[0][0].a > NAUTICAL) {
            printf("nautical up\t: %s", ctime(&when));
         }
         else {
            printf("nautical down\t: %s", ctime(&when));
         }
         if (Aa[0][0].a > CIVIL) {
            printf("civil up\t: %s", ctime(&when));
         }
         else {
            printf("civil down\t: %s", ctime(&when));
         }
         if (Aa[0][0].a > SOLAR) {
            printf("solar up\t: %s", ctime(&when));
         }
         else {
            printf("solar down\t: %s", ctime(&when));
         }
         if (Aa[0][0].a > LUNAR) {
            printf("lunar up\t: %s", ctime(&when));
         }
         else {
            printf("lunar down\t: %s", ctime(&when));
         }
         for (int j = 2; j < 8; j++) {
            if (Aa[j][0].a > HORIZON) {
               printf("%s up\t: %s", names[j], ctime(&when));
            }
            else {
               printf("%s down\t: %s", names[j], ctime(&when));
            }
         }
      }
      else {
         for (int j = 0; j < 8; j++) {
            if (i == max[j]) {
               printf("%s transit\t: %s", names[j], ctime(&when));
            }
         }

         if (Aa[0][i-1].a < SOLAR && Aa[0][i].a >= SOLAR) {
            printf("solar rise\t: %s", ctime(&when));
         }
         if (Aa[0][i-1].a >= SOLAR && Aa[0][i].a < SOLAR) {
            printf("solar set\t: %s", ctime(&when));
         }
         if (Aa[0][i-1].a < CIVIL && Aa[0][i].a >= CIVIL) {
            printf("civil rise\t: %s", ctime(&when));
         }
         if (Aa[0][i-1].a >= CIVIL && Aa[0][i].a < CIVIL) {
            printf("civil set\t: %s", ctime(&when));
         }
         if (Aa[0][i-1].a < NAUTICAL && Aa[0][i].a >= NAUTICAL) {
            printf("nautical rise\t: %s", ctime(&when));
         }
         if (Aa[0][i-1].a >= NAUTICAL && Aa[0][i].a < NAUTICAL) {
            printf("nautical set\t: %s", ctime(&when));
         }
         if (Aa[0][i-1].a < ASTRONOMICAL && Aa[0][i].a >= ASTRONOMICAL) {
            printf("astronomical rise\t: %s", ctime(&when));
         }
         if (Aa[0][i-1].a >= ASTRONOMICAL && Aa[0][i].a < ASTRONOMICAL) {
            printf("astronomical set\t: %s", ctime(&when));
         }

         for (int j = 1; j < 8; j++) {
            if (Aa[j][i-1].a < HORIZON && Aa[j][i].a >= HORIZON) {
               printf("%s rise\t: %s", names[j], ctime(&when));
            }
            if (Aa[j][i-1].a >= HORIZON && Aa[j][i].a < HORIZON) {
               printf("%s set\t: %s", names[j], ctime(&when));
            }
         }
      }
   }
}
#endif

#ifdef PLANET
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
   time_t now = time(NULL);
   now /= 60;
   now *= 60;

   struct φλ φλ = { atof(argv[1]), atof(argv[2]) };
   int planet = atoi(argv[3]);

   struct αδ αδ = ə54(time_t2julian(now), planet);
   printf("%f %f\n", αδ.α, αδ.δ);

}
#endif
