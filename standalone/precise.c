#include <math.h>

double sin_deg(double angle) {
   return sin(angle * M_PI / 180.0);
}

double cos_deg(double angle) {
   return cos(angle * M_PI / 180.0);
}

double tan_deg(double angle) {
   return tan(angle * M_PI / 180.0);
}

double asin_deg(double sine) {
   return asin(sine) * 180.0 / M_PI;
}

double acos_deg(double cosine) {
   return acos(cosine) * 180.0 / M_PI;
}

double atan_deg(double tangent) {
   return atan(tangent) * 180.0 / M_PI;
}

double atan2_deg(double y, double x) {
   return atan2(y, x) * 180.0 / M_PI;
}

// vim: expandtab:noai:ts=3:sw=3
