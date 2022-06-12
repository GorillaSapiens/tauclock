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

int main(int argc, char **argv) {
   memset(page, 0, sizeof(page));

   double pitch = atof(argv[1]);
   double roll = atof(argv[2]);
   double yaw = atof(argv[3]);

   // from https://stackoverflow.com/questions/34050929/3d-point-rotation-algorithm
   double cosa = cos(yaw);
   double sina = sin(yaw);

   double cosb = cos(pitch);
   double sinb = sin(pitch);

   double cosc = cos(roll);
   double sinc = sin(roll);

   double Axx = cosa*cosb;
   double Axy = cosa*sinb*sinc - sina*cosc;
   double Axz = cosa*sinb*cosc + sina*sinc;

   double Ayx = sina*cosb;
   double Ayy = sina*sinb*sinc + cosa*cosc;
   double Ayz = sina*sinb*cosc - cosa*sinc;

   double Azx = -sinb;
   double Azy = cosb*sinc;
   double Azz = cosb*cosc;

   for (int i = 0; points[i].xyz != 0; i++) {
      int xyz = points[i].xyz;

      int x = xyz >> 16;
      if (x & 0x80) x |= ~0xFF;

      int y = (xyz >> 8) & 0xFF;
      if (y & 0x80) y |= ~0xFF;

      int z = (xyz) & 0xFF;
      if (z & 0x80) z |= ~0xFF;

      // from https://stackoverflow.com/questions/34050929/3d-point-rotation-algorithm

      double px = x;
      double py = y;
      double pz = z;

      x = Axx*px + Axy*py + Axz*pz;
      y = Ayx*px + Ayy*py + Ayz*pz;
      z = Azx*px + Azy*py + Azz*pz;

      if (z >= 0) {
         page[y+128][x+128] = points[i].rgb;

         if (y+128 >= 1 && x+128 >= 1 && y+128 < 255 && x+128 < 255) {
            for (int dy = -1; dy < 2; dy++) {
               for (int dx = -1; dx < 2; dx++) {
                  if (page[y+128+dy][x+128+dx] == 0) {
                     page[y+128+dy][x+128+dx] = points[i].rgb;
                  }
               }
            }
         }
      }
   }

   for (int y = 0; y < 256; y++) {
      for (int x = 0; x < 256; x++) {
         unsigned char r = page[y][x] >> 16;
         unsigned char g = (page[y][x] >> 8) & 0xFF;
         unsigned char b = (page[y][x]) & 0xFF;

         write(1, &r, 1);
         write(1, &g, 1);
         write(1, &b, 1);
      }
   }
}
