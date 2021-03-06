#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "world.h"

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

int main (int argc, char **argv) {
   int size = atoi(argv[1]);

   int radius = size / 2;

   double xspin = -atof(argv[2]) * M_PI / 180.0;
   double yspin = -atof(argv[3]) * M_PI / 180.0;
   double zspin = atof(argv[4]) * M_PI / 180.0;

   double coshalfx = cos(xspin/2.0);
   double sinhalfx = sin(xspin/2.0);

   double coshalfy = cos(yspin/2.0);
   double sinhalfy = sin(yspin/2.0);

   double coshalfz = cos(zspin/2.0);
   double sinhalfz = sin(zspin/2.0);

   quat qx = { coshalfx, sinhalfx, 0, 0 };
   quat qy = { coshalfy, 0, sinhalfy, 0 };
   quat qz = { coshalfz, 0, 0, sinhalfz };

   unsigned char *image = (unsigned char *) calloc(3*size*size, 1);

   for (int x = 0; x < size; x++) {
      for (int y = 0; y < size; y++) {

         // compute z based on x and y

         double xx = (x - radius) + 0.5;
         double yy = (y - radius) + 0.5;
         double zz = (radius*radius) - (xx * xx) - (yy * yy);

         if (zz >= 0.0) {
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

            double lat = asin(-yy / (double) radius) * 180.0 / M_PI;
            double lon = atan2(xx, -zz) * 180.0 / M_PI;

            // now convert to bitmap coordinates
            int by = round((180.0 - (lat + 90.0)) * ((double) rows) / 180.0);
            int bx = round((lon + 180.0) * ((double) columns) / 360.0);

            if (bx < 0) bx = 0;
            if (by < 0) by = 0;
            if (bx >= columns) bx = columns - 1;
            if (by >= rows) by = rows - 1;

            int c = palette[bitmap[by][bx]];

            image[3*y*size + 3*x] = c >> 16;
            image[3*y*size + 3*x + 1] = (c >> 8) & 0xff;
            image[3*y*size + 3*x + 2] = c & 0xff;
         }
      }
   }

   int margin = radius / 25;
   if (margin < 4) {
      margin = 4;
   }

   for (int i = radius - margin; i < radius + margin + 1; i++) {
      image[3*radius*size + 3*i] ^= 0xff;
      image[3*radius*size + 3*i + 1] ^= 0xff;
      image[3*radius*size + 3*i + 2] ^= 0xff;

      image[3*i*size + 3*radius] = 0xff;
      image[3*i*size + 3*radius + 1] ^= 0xff;
      image[3*i*size + 3*radius + 2] ^= 0xff;
   }

   write(1, image, 3 * size * size);
}
