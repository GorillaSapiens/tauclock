#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define RADIUS 127

typedef struct Point {
   int xyz;
   int rgb;
} Point;

Point points[] = {
#include "precomputed.h"
};

int page[256][256];

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

int main(int argc, char **argv) {
   memset(page, 0, sizeof(page));

   // coordinates are wonky...
   double xspin = -atof(argv[1]) * M_PI / 180.0;
   double yspin = -atof(argv[2]) * M_PI / 180.0;
   double zspin = atof(argv[3]) * M_PI / 180.0;

   double coshalfx = cos(xspin/2.0);
   double sinhalfx = sin(xspin/2.0);

   double coshalfy = cos(yspin/2.0);
   double sinhalfy = sin(yspin/2.0);

   double coshalfz = cos(zspin/2.0);
   double sinhalfz = sin(zspin/2.0);

   quat qx = { coshalfx, sinhalfx, 0, 0 };
   quat qy = { coshalfy, 0, sinhalfy, 0 };
   quat qz = { coshalfz, 0, 0, sinhalfz };

   for (int i = 0; points[i].xyz != 0; i++) {
      int xyz = points[i].xyz;

      int x = xyz >> 16;
      if (x & 0x80) x |= ~0xFF;

      int y = (xyz >> 8) & 0xFF;
      if (y & 0x80) y |= ~0xFF;

      int z = (xyz) & 0xFF;
      if (z & 0x80) z |= ~0xFF;

      quat qp = { 0, x, y, z };

      qp = rotate(qp, qy);
      qp = rotate(qp, qx);
      qp = rotate(qp, qz);

      x = round(qp.i);
      y = round(qp.j);
      z = round(qp.k);

      if (z >= 0) {
         page[y+128][x+128] = points[i].rgb;
      }
   }

   for (int y = 1; y < 255; y++) {

      int yy = y - 128;
      // 127 = sqrt(yy*yy+xx*xx)
      // 127*127 - yy*yy = xx*xx
      int xx;
      if (yy != 0) {
         xx = sqrt(127*127 - yy*yy);
      }
      else {
         xx = 127;
      }

      for (int x = 128-xx; x < 128+xx; x++) {
         int n = 0;
         int sr = 0;
         int sg = 0;
         int sb = 0;
         if (page[y][x] == 0) {
            for (int dy = -1; dy < 2; dy++) {
               for (int dx = -1; dx < 2; dx++) {
                  if (page[y+dy][x+dx] != 0) {
                     int c = page[y+dy][x+dx];
                     int r = c >> 16;
                     int g = (c >> 8) & 0xff;
                     int b = c & 0xff;

                     n++;
                     sr += r;
                     sg += g;
                     sb += b;
                  }
               }
            }
            if (n != 0) {
               sr /= n;
               sg /= n;
               sb /= n;
               int c = (sr << 16) | (sg << 8) | (sb);
               page[y][x] = c;
            }
         }
      }
   }

   for (int y = 0; y < 256; y++) {
      page[y][256/2] ^= 0xffffff;
      page[256/2][y] ^= 0xffffff;
   }

   for (int y = 0; y < 256; y++) {
      for (int x = 0; x < 256; x++) {
         unsigned char r = page[y][x] >> 16;
         unsigned char g = (page[y][x] >> 8) & 0xFF;
         unsigned char b = (page[y][x]) & 0xFF;

         int unused;
         unused = write(1, &r, 1);
         unused = write(1, &g, 1);
         unused = write(1, &b, 1);
      }
   }
}
