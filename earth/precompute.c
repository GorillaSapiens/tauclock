#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "world.xpm"

#define RADIUS 127
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
   int cols, rows, colors;
   sscanf (world[0], "%d %d %d", &cols, &rows, &colors);
   for (int x = 0; x < cols; x++) {
      for (int y = 0; y < rows; y++) {
         double lat = -(((double) y - ((double) rows / 2.0)) /
               ((double)rows/2.0)) * M_PI / 2.0;
         double lon = (((double) x - ((double) cols / 2.0)) /
               ((double)cols/2.0)) * M_PI;

         char c = world[1+colors+y][x];
         for (int i = 0; i < colors; i++) {
            if (world[1+i][0] == c) {
               int rgb = strtol(world[1+i]+5, NULL, 16);
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
         int x = tmp->x & 0xFF;
         int y = tmp->y & 0xFF;
         int z = tmp->z & 0xFF;
         printf("{ 0x%06x, 0x%06x },\n",
               (x << 16) | (y << 8) | z, tmp->c);
      }
   }
}
