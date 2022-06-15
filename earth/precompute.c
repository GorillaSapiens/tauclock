#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "world.xpm"

#define RADIUS (256-1)
#define BINS 4096

struct Node;

typedef struct Node {
   int x, y, z, c;
   struct Node *next;
} Node;

Node *heads[BINS]= { NULL };
int count = 0;

void addnode(int x, int y, int z, int c) {
   int bin = (abs(x)*abs(y)*abs(z)) % BINS;

   Node *tmp = heads[bin];

   while (tmp) {
      if (tmp->x == x && tmp->y == y && tmp->z == z) {
         return;
      }
#ifdef MANHATTAN
      if (abs(tmp->x - x) + abs(tmp->y - y) + abs(tmp->z - z) < 3) {
         if (tmp->c == c) {
            return;
         }
      }
#endif
      tmp = tmp->next;
   }

   tmp = (Node *) malloc(sizeof(Node));
   tmp->x = x;
   tmp->y = y;
   tmp->z = z;
   tmp->c = c;
   tmp->next = heads[bin];
   heads[bin] = tmp;
   count++;
}

void main(void) {
   memset(heads, 0, sizeof(heads));
   // 2048 1023 84 1
   int cols, rows, colors, cpp;
   sscanf (world[0], "%d %d %d %d", &cols, &rows, &colors, &cpp);
   for (int x = 0; x < cols; x++) {
      for (int y = 0; y < rows; y++) {
         double lat = -(((double) y - ((double) rows / 2.0)) /
               ((double)rows/2.0)) * M_PI / 2.0;
         double lon = (((double) x - ((double) cols / 2.0)) /
               ((double)cols/2.0)) * M_PI;

         short c = world[1+colors+y][x*cpp];
         if (cpp == 2) {
            c <<= 8;
            c |= world[1+colors+y][x*cpp + 1];
         }
         for (int i = 0; i < colors; i++) {
            short cc = world[1+i][0];
            if (cpp == 2) {
               cc <<= 8;
               cc |= world[1+i][1];
            }
            if (cc == c) {
               int rgb = strtol(world[1+i]+5+(cpp-1), NULL, 16);
               int z = RADIUS * cos(lat) * cos(lon);
               int x = RADIUS * cos(lat) * sin(lon);
               int y = -RADIUS * sin(lat);

               addnode(x,y,z,rgb);
            }
         }
      }
   }

   for (int bin = 0; bin < BINS; bin++) {
      for (Node *tmp = heads[bin]; tmp; tmp = tmp->next) {
         int x = tmp->x & 0x1FF;
         int y = tmp->y & 0x1FF;
         int z = tmp->z & 0x1FF;
         printf("{ 0x%08x, 0x%06x },\n",
               (x << 18) | (y << 9) | z, tmp->c);
      }
   }
}
