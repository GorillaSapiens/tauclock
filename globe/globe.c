//  Sunclock, draw a clock with local solar and lunar information
//  Copyright (C) 2022,2023 Adam Wozniak / GorillaSapiens
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma GCC optimize("Ofast")

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "precise.h"
#include "draw.h"
#include "globei.h"

typedef struct quat {
   double w;
   double i;
   double j;
   double k;
} quat;

quat mult(quat a, quat b) {
   quat ret;

// z1 = [a bi cj dk]
// z2 = [e fi gj hk]
//
// z1 * z2=    a*e - b*f - c*g - d*h +
//          i (b*e + a*f + c*h - d*g) +
//          j (a*g - b*h + c*e + d*f) +
//          k (a*h + b*g - c*f + d*e)

   ret.w = a.w * b.w - a.i * b.i - a.j * b.j - a.k * b.k;
   ret.i = a.i * b.w + a.w * b.i + a.j * b.k - a.k * b.j;
   ret.j = a.w * b.j - a.i * b.k + a.j * b.w + a.k * b.i;
   ret.k = a.w * b.k + a.i * b.j - a.j * b.i + a.k * b.w;

   return ret;
}

quat rotate(quat point, quat rot) {
   quat inv = { rot.w, -rot.i, -rot.j, -rot.k };

   quat ret =  mult(mult(rot, point),inv);

   return ret;
}

#ifndef STANDALONE
extern DrawFont mFONT_BOLD_BIG;
#define FONT_BOLD_BIG (&mFONT_BOLD_BIG)

void do_text(Canvas *canvas, int width, double lat, double lon, const char *tzname) {
   char location[128];
   char location2[128];

   char NS = 'N';
   if (lat < 0.0) {
      lat = -lat;
      NS = 'S';
   }

   char EW = 'E';
   if (lon < 0.0) {
      lon = -lon;
      EW = 'W';
   }
   char *degree = "\u00B0";     // in utf8, degree symbol

   if (lat > 90.0 || lon > 180.0 || lat < -90.0 || lon < -180.0) {
      sprintf(location , "INVALID");
      sprintf(location2, "LOCATION");
   }
   else {
      sprintf(location, "%0.4f%s%c", lat, degree, NS);
      sprintf(location2, "%0.4f%s%c", lon, degree, EW);
   }
   int wh = text_canvas(canvas, FONT_BOLD_BIG, -1000, -1000,
               COLOR_WHITE, COLOR_BLACK, location, 1, 2);
   int w = wh >> 16;
   int h = wh & 0xffff;

   text_canvas(canvas, FONT_BOLD_BIG, w/2+10, (h/2+10),
               COLOR_WHITE, COLOR_BLACK, location, 1, 2);

   wh = text_canvas(canvas, FONT_BOLD_BIG, -1000, -1000,
               COLOR_WHITE, COLOR_BLACK, location2, 1, 2);
   w = wh >> 16;
   h = wh & 0xffff;

   text_canvas(canvas, FONT_BOLD_BIG, width - (w/2+10), (h/2+10),
               COLOR_WHITE, COLOR_BLACK, location2, 1, 2);

    wh = text_canvas(canvas, FONT_BOLD_BIG, -1000, -1000,
                     COLOR_WHITE, COLOR_BLACK, tzname, 1, 2);
    w = wh >> 16;
    h = wh & 0xffff;

    text_canvas(canvas, &mFONT_BOLD_BIG, (w/2+10), width - (h/2+10),
                COLOR_WHITE, COLOR_BLACK, tzname, 1, 2);
}
#endif

/// @brief Do globe things
///
/// @param lat The observer's Latitude in degrees, South is negative
/// @param lon The observer's Longitude in degrees, West is negative
/// @param offset An offset from the current Julian Date
/// @return A canvas that has been drawn upon
Canvas *do_globe(double lat, double lon, double spin, int width, const char *tzname) {

   Canvas *canvas = new_canvas(width, width, COLOR_BLACK);

   int size = width;

   int radius = size / 2;

   double xspin = -lat;
   double yspin = -lon;
   double zspin = -spin; // TODO FIX is the sign right?

   double coshalfx = cos_deg(xspin/2.0);
   double sinhalfx = sin_deg(xspin/2.0);

   double coshalfy = cos_deg(yspin/2.0);
   double sinhalfy = sin_deg(yspin/2.0);

   double coshalfz = cos_deg(zspin/2.0);
   double sinhalfz = sin_deg(zspin/2.0);

   quat qx = { coshalfx, sinhalfx, 0, 0 };
   quat qy = { coshalfy, 0, sinhalfy, 0 };
   quat qz = { coshalfz, 0, 0, sinhalfz };

   //quat qrot = mult(mult(qz, qy), qx);
   //quat qrot = mult(mult(qx, qy), qz);
   quat qrot = mult(mult(qy, qx), qz);

   //quat qrot = mult(qz, mult(qy, qx));
   //quat qrot = mult(qy, qx);

   static const double BPI=3.5;

#define RSTEP(r) (((r) < size/6) ? (1) : (((r) < size/3) ? (3) : (5)))
#define TSTEP(r) (RSTEP(r) * (360.0 / (2.0 * 3.1415 * (double)(r))))

   for (int r = 1; r < size/2; r += RSTEP(r)) {
      for (double t = 0.0; t < 360.0; t += TSTEP(r)) {

	      int x = ((double)size/2.0) + (double) r * cos_deg(t) + .5;
	      int y = ((double)size/2.0) + (double) r * sin_deg(t) + .5;

         // compute z based on x and y

         double xx = (x - radius) + 0.5;
         double yy = (y - radius) + 0.5;
         double zz = (radius*radius) - (xx * xx) - (yy * yy);

         if (zz >= 0.0) {  // if it's less than zero, this bit of the screen is off the globe
            zz = -sqrt(zz); // negative, because right handed

            quat qp = { 0, xx, yy, zz };

#if 0
            if (zspin != 0.0) {
               qp = rotate(qp, qz);
            }
            if (xspin != 0.0) {
               qp = rotate(qp, qx);
            }
            if (yspin != 0.0) {
               qp = rotate(qp, qy);
            }
#else
            qp = rotate(qp, qrot);
#endif

            xx = qp.i;
            yy = qp.j;
            zz = qp.k;

            // convert back to lat/lon

            // i.e. reverse this
            // int z = RADIUS * cos(lat) * cos(lon);
            // int x = RADIUS * cos(lat) * sin(lon);
            // int y = -RADIUS * sin(lat);

            double lat = asin_deg(-yy / (double) radius);
            double lon = atan2_deg(xx, -zz);

            // now convert to bitmap coordinates
            int by = round((180.0 - (lat + 90.0)) * ((double) rows) / 180.0);
            int bx = round((lon + 180.0) * ((double) columns) / 360.0);

            if (bx < 0) bx = 0;
            if (by < 0) by = 0;
            if (bx >= columns) bx = columns - 1;
            if (by >= rows) by = rows - 1;

            int c = palette[bitmap[by][bx]] | 0xff000000;
#ifdef STANDALONE
            int ccc = c & 0xFF00FF00;
            ccc |= (c & 0x00FF0000) >> 16;
            ccc |= (c & 0x000000FF) << 16;
            c = ccc;
#endif

            int xl = RSTEP(r) / 2;
            int yl = xl;
            for (int dx = -xl; dx < xl+1; dx++) {
               for (int dy = -yl; dy < yl+1; dy++) {
                  poke_canvas(canvas, x+dx, y+dy, c);
               }
            }
         }
      }
   }

   int margin = radius / 25;
   if (margin < 4) {
      margin = 4;
   }

   arc_canvas(canvas, radius, radius, radius/10,
              20, COLOR_BLACK,
              0.0, 360.0);
   arc_canvas(canvas, radius, radius, radius/10,
                    5, COLOR_MAGENTA,
                    0.0, 360.0);

   line_canvas(canvas,radius-radius/14,radius-radius/14,radius+radius/14,radius+radius/14,COLOR_MAGENTA);
   line_canvas(canvas,radius-radius/14,radius+radius/14,radius+radius/14,radius-radius/14,COLOR_MAGENTA);

#ifndef STANDALONE
   do_text(canvas, width, lat, lon, tzname);
#endif

   return canvas;
}

#ifdef STANDALONE
int main (int argc, char **argv) {
   int size = atoi(argv[1]);
   double lat = atof(argv[2]);
   double lon = atof(argv[3]);
   double spin = atof(argv[4]);

   Canvas *canvas = do_globe(lat, lon, spin, size, NULL);
   dump_canvas(canvas, NULL);
}
#endif
