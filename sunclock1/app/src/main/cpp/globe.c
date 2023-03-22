#pragma GCC optimize("Ofast")
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "trig1.h"
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

extern uint8_t *FONT_BOLD_BIG;

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

    text_canvas(canvas, FONT_BOLD_BIG, (w/2+10), width - (h/2+10),
                COLOR_WHITE, COLOR_BLACK, tzname, 1, 2);
}

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

   for (int x = 0; x < size; x++) {
      for (int y = 0; y < size; y++) {

         // compute z based on x and y

         double xx = (x - radius) + 0.5;
         double yy = (y - radius) + 0.5;
         double zz = (radius*radius) - (xx * xx) - (yy * yy);

         if (zz >= 0.0) {  // if it's less than zero, this bit of the screen is off the globe
            zz = -sqrt(zz); // negative, because right handed

            quat qp = { 0, xx, yy, zz };

            if (zspin != 0.0) {
               qp = rotate(qp, qz);
            }
            if (xspin != 0.0) {
               qp = rotate(qp, qx);
            }
            if (yspin != 0.0) {
               qp = rotate(qp, qy);
            }

            xx = qp.i;
            yy = qp.j;
            zz = qp.k;

            // convert back to lat/lon

            // i.e. reverse this
            // int z = RADIUS * cos(lat) * cos(lon);
            // int x = RADIUS * cos(lat) * sin(lon);
            // int y = -RADIUS * sin(lat);

            double lat = asin_deg(-yy / (double) radius);
            //double lon = atan2(xx, -zz) * 180.0 / M_PI;
            double lon = atan2_deg(xx, -zz);

            // now convert to bitmap coordinates
            int by = round((180.0 - (lat + 90.0)) * ((double) rows) / 180.0);
            int bx = round((lon + 180.0) * ((double) columns) / 360.0);

            if (bx < 0) bx = 0;
            if (by < 0) by = 0;
            if (bx >= columns) bx = columns - 1;
            if (by >= rows) by = rows - 1;

            int c = palette[bitmap[by][bx]] | 0xff000000;

	        poke_canvas(canvas, x, y, c);
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

   do_text(canvas, width, lat, lon, tzname);

   return canvas;
}
