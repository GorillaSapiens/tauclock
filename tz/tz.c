#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const char database[] = "/usr/share/zoneinfo/zone1970.tab";

// ugh!
double defogle(char *p, int n) {
   while (*p && *p != '+' && *p != '-') {
      p++;
   }
   if (!*p) {
      return 0;
   }
   double neg = (*p == '-') ? -1 : 1;
   p++;
   double ret = 0;
   for (int i = 0; i < n; i++) {
      ret *= 10.0;
      ret += (*p - '0');
      p++;
   }
   double min = 0;
   for (int i = 0; i < 2; i++) {
      min *= 10.0;
      min += (*p - '0');
      p++;
   }
   if (*p == '+' || *p == '-' || *p == 0) {
      return neg * (ret + min / 60.0);
   }
   double sec = 0;
   for (int i = 0; i < 2; i++) {
      sec *= 10.0;
      sec += (*p - '0');
      p++;
   }
   return neg * (ret + min / 60.0 + sec / 3600.0);
}

void sortem(double lat0, double lon0) {
   double x0 = cos(lat0 * M_PI / 180.0) * cos(lon0 * M_PI / 180.0);
   double y0 = cos(lat0 * M_PI / 180.0) * sin(lon0 * M_PI / 180.0);
   double z0 = sin(lat0 * M_PI / 180.0);

   FILE *f = fopen(database, "r");
   if (f != NULL) {
      char buffer[1024];
      while (fgets(buffer, sizeof(buffer) - 1, f) != NULL) {
         if (buffer[0] != '#') {
            char *country = buffer;
            char *coords = country;
            while (*coords >= ' ') {
               coords++;
            }
            *coords = 0;
            coords++;
            char *tz = coords;
            while (*tz >= ' ') {
               tz++;
            }
            *tz = 0;
            tz++;
            char *comment = tz;
            while (*comment >= ' ') {
               comment++;
            }
            *comment = 0;
            comment++;

            // ugh, this is a HORRIBLE format!
            double lat = defogle(coords, 2);
            double lon = defogle(coords + 1, 3);

//            lat = lat0;

            double x = cos(lat * M_PI / 180.0) * cos(lon * M_PI / 180.0);
            double y = cos(lat * M_PI / 180.0) * sin(lon * M_PI / 180.0);
            double z = sin(lat * M_PI / 180.0);

            double dist = sqrt(
                  (x - x0)*(x - x0)+
                  (y - y0)*(y - y0)+
                  (z - z0)*(z - z0));
            printf("%f|%s|%s|%f|%f|%s|\n", dist, country, coords, lat, lon, tz);
         }
      }
      fclose(f);
   }
}

int main (int argc, char **argv) {
   double lat = atof(argv[1]);
   double lon = atof(argv[2]);

   sortem(lat, lon);
}
