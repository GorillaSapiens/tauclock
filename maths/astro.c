#include "trig1.h"
#include "astro.h"

double time_t2julian(time_t t) {
   // https://stackoverflow.com/questions/466321/convert-unix-timestamp-to-julian
   //
   // The Unix epoch (zero-point) is January 1, 1970 GMT.
   // That corresponds to the Julian day of 2440587.5

   return (double)t / 86400.0 + 2440587.5;
}

struct αδ sec27(double jd, double λ, double β) {
   double T = (jd - 2451545.0) / 36525.0;
   double DE = (46.815 * T + 0.0006 * T * T - 0.00181*T*T*T) / 3600.0;
   double ε = 23.439292 - DE;

   struct αδ ret;

   ret.α =
      atan_deg(
         (sin_deg(λ)*cos_deg(ε) - tan_deg(β)*sin_deg(ε)) /
         cos_deg(λ));

   ret.δ = asin_deg(sin_deg(β)*cos_deg(ε) + cos_deg(β)*sin_deg(ε)*sin_deg(λ));

   return ret;
}

struct αδ sec46(double jd) {
   // 2010 January 0.0 (JD = 2 455 196.5)
   static const double epoch = 2455196.5;

   // εg (ecliptic longitude at epoch 2010.0) = 279.557208 degrees
   static const double εg = 279.557208;

   // ϖg (ecliptic longitude of perigee at epoch 2010.0) = 283.112438 degrees
   static const double ϖg = 283.112438;

   // e (eccentricity of orbit at epoch 2010.0) = 0.016705
   static const double e  = 0.016705;

   // r0 (semi-major axis) = 1.495985×10^8 km
   static const double r0 = 1.495985e8;

   // θ0 (angular diameter at r = r0) = 0.533128 degrees
   static const double θ0 = 0.533128;

   // ==================

   double D = jd - epoch;

   double N = (360.0 * D / 365.242191);
   while (N < 0.0) N += 360.0;
   while (N >= 360.0) N -= 360.0;

   double M = N + εg - ϖg;
   while (M < 0.0) M += 360.0;
   while (M >= 360.0) M -= 360.0;

   double Ec = (360.0 / M_PI) * e * sin_deg(M);

   double λ = N + Ec + εg;
      while (λ < 0.0) λ += 360.0;
      while (λ >= 360.0) λ -= 360.0;

   return sec27(jd, λ, 0.0);
}
