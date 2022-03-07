/*
    Sunclock, draw a clock with local solar and lunar information
    Copyright (C) 2022 Adam Wozniak / GorillaSapiens

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define INCLUDE_FONT_DATA
#include "draw.h"

Canvas *new_canvas(int w, int h, unsigned char fill) {
   Canvas *ret = (Canvas *) malloc(sizeof(Canvas));
   ret->w = w;
   ret->h = h;
   ret->data = (unsigned int *)malloc(sizeof(unsigned int) * w * h);
   memset(ret->data, fill, sizeof(unsigned int) * w * h);
   return ret;
}

void delete_canvas(Canvas * canvas) {
   free(canvas->data);
   free(canvas);
}

void dump_canvas(Canvas * canvas, const char *fname) {
   int fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);

   if (fd == -1) {
      fprintf(stderr, "%s COULD NOT OPEN %s\n", __FUNCTION__, fname);
      exit(-1);
   }

   int ret = write(fd, canvas->data,
                   sizeof(unsigned int) * canvas->w * canvas->h);
   if (ret != sizeof(unsigned int) * canvas->w * canvas->h) {
      fprintf(stderr, "%s BAD WRITE %d\n", __FUNCTION__, ret);
      exit(-1);
   }
   close(fd);
}

void xor_canvas(Canvas * mask, Canvas * target) {
   for (int y = 0; y < mask->h; y++) {
      for (int x = 0; x < mask->w; x++) {
         if (mask->data[y * mask->w + x]) {
            if (y < target->h) {
               if (x < target->w) {
                  target->data[y * target->w + x] ^= 0xFFFFFF;
               }
            }
         }
      }
   }
}

void poke_canvas(Canvas * canvas, int x, int y, unsigned int color) {
   if (0 <= x && x < canvas->w) {
      if (0 <= y && y < canvas->h) {
         canvas->data[y * canvas->w + x] = color;
      }
   }
}

void
conditional_poke_canvas(Canvas * canvas, int x, int y, unsigned int color,
                        unsigned int old) {
   if (0 <= x && x < canvas->w) {
      if (0 <= y && y < canvas->h) {
         if (canvas->data[y * canvas->w + x] == old) {
            canvas->data[y * canvas->w + x] = color;
         }
      }
   }
}

unsigned int peek_canvas(Canvas * canvas, int x, int y) {
   if (0 <= x && x < canvas->w) {
      if (0 <= y && y < canvas->h) {
         return canvas->data[y * canvas->w + x];
      }
   }
   return COLOR_NONE;
}

static int utf8parse(const char *p) {
   static const unsigned char *q;
   int ret = 0;

   if (p != NULL) {
      q = (unsigned char *)p;
   }

   if (*q < 0x80) {
      ret = *q++;
      return ret;
   }
   else if ((*q & 0xE0) == 0xC0) {
      // 110xxxxx 10xxxxxx
      ret = *q++ & 0x1F;
      ret <<= 6;
      ret |= *q++ & 0x3F;
      return ret;
   }
   else if ((*q & 0xF0) == 0xE0) {
      // 1110xxxx 10xxxxxx 10xxxxxx
      ret = *q++ & 0x0F;
      ret <<= 6;
      ret |= *q++ & 0x3F;
      ret <<= 6;
      ret |= *q++ & 0x3F;
      return ret;
   }
   else if ((*q & 0xF1) == 0xF0) {
      // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      ret = *q++ & 0x07;
      ret <<= 6;
      ret = *q++ & 0x0F;
      ret <<= 6;
      ret |= *q++ & 0x3F;
      ret <<= 6;
      ret |= *q++ & 0x3F;
      return ret;
   }
   else {
      // TODO FIX what to do in event of malformed?
      // TERMINATE!
      return 0;
   }
}

struct Glyph {
   int glyph;
   int width;
   int height;
   int step;
   int dx;
   int dy;
   uint8_t *data;
};

struct Glyph font_find_glyph(uint8_t * font, uint16_t glyph) {
   int font_height = font[1];
   int font_dy = (signed char)font[3];

   font += 4;

   struct Glyph ret;
   do {
      ret.glyph = (((unsigned int)font[0]) << 8) | ((unsigned int)font[1]);
      font += 2;

      ret.width = font[0];
      ret.height = font[1];
      ret.step = (ret.width + 7) / 8;
      ret.dx = 0;
      ret.dy = (font_height + font_dy) - (ret.height + (signed char)font[3]);
      ret.data = font + 4;

      font += 4 + ret.step * ret.height;
   }
   while (ret.glyph != glyph && ret.glyph != 0xFFFF);

   return ret;
}

int
text_canvas(Canvas * canvas, uint8_t * font, int x, int y, unsigned int fg,
            unsigned int bg, const char *p, int mult, int gap) {
   int outline = 2;

   int encoded;

   int max_x = 0;
   int min_x = 0;
   int max_y = 0;
   int min_y = 0;
   int first = 1;
   int fake_x = 0;
   int fake_y = 0;

   for (encoded = utf8parse(p); encoded; encoded = utf8parse(NULL)) {
      struct Glyph glyph = font_find_glyph(font, encoded);
      for (int h = 0; h < glyph.height; h++) {
         for (int w = 0; w < glyph.width; w++) {
            int offset = w / 8;
            int bit = glyph.data[offset] & (1 << (7 - (w % 8)));
            if (bit) {
               for (int mx = 0; mx < mult; mx++) {
                  for (int my = 0; my < mult; my++) {
                     int tx = fake_x + w * mult + mx + glyph.dx * mult;
                     int ty = fake_y + h * mult + my + glyph.dy * mult;

                     if (first || tx > max_x) {
                        max_x = tx;
                     }
                     if (first || tx < min_x) {
                        min_x = tx;
                     }
                     if (first || ty > max_y) {
                        max_y = ty;
                     }
                     if (first || ty < min_y) {
                        min_y = ty;
                     }
                     first = 0;
                  }
               }
            }
         }
         glyph.data += glyph.step;
      }
      fake_x += glyph.width * mult + gap * mult;
   }

   int ret = ((max_x - min_x) << 16) | (max_y - min_y);

   x -= (max_x / 2) + min_x;
   y -= (max_y / 2) + min_y;

   int ox = x;

   if (bg != COLOR_NONE) {
      for (encoded = utf8parse(p); encoded; encoded = utf8parse(NULL)) {
         struct Glyph glyph = font_find_glyph(font, encoded);
         for (int h = 0; h < glyph.height; h++) {
            for (int w = 0; w < glyph.width; w++) {
               int offset = w / 8;
               int bit = glyph.data[offset] & (1 << (7 - (w % 8)));
               if (bit) {
                  for (int mx = 0; mx < mult; mx++) {
                     for (int my = 0; my < mult; my++) {
                        for (int dx =
                             -mult * outline; dx <= mult * outline; dx++) {
                           for (int
                                dy
                                = -mult * outline; dy <= mult * outline; dy++) {
                              poke_canvas
                                 (canvas,
                                  x + w * mult + mx + dx + glyph.dx * mult,
                                  y + h * mult + my + dy + glyph.dy * mult, bg);
                           }
                        }
                     }
                  }
               }
            }
            glyph.data += glyph.step;
         }
         x += glyph.width * mult + gap * mult;
      }
   }

   x = ox;

   for (encoded = utf8parse(p); encoded; encoded = utf8parse(NULL)) {
      struct Glyph glyph = font_find_glyph(font, encoded);
      for (int h = 0; h < glyph.height; h++) {
         for (int w = 0; w < glyph.width; w++) {
            int offset = w / 8;
            int bit = glyph.data[offset] & (1 << (7 - (w % 8)));
            if (bit) {
               for (int mx = 0; mx < mult; mx++) {
                  for (int my = 0; my < mult; my++) {
                     poke_canvas(canvas,
                                 x + w * mult + mx + glyph.dx * mult,
                                 y + h * mult + my + glyph.dy * mult, fg);
                  }
               }
            }
         }
         glyph.data += glyph.step;
      }
      x += glyph.width * mult + gap * mult;
   }

   return ret;
}

void
blur_poke_canvas(Canvas * canvas, int x, int y, unsigned int color, int blur) {
   for (int i = -blur; i <= blur; i++) {
      for (int j = -blur; j <= blur; j++) {
         poke_canvas(canvas, x + i, y + j, color);
      }
   }
}

void
line_canvas(Canvas * canvas, int x1, int y1, int x2, int y2,
            unsigned int color) {
   blur_poke_canvas(canvas, x1, y1, color, 1);
   blur_poke_canvas(canvas, x2, y2, color, 1);

   int d = ((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2));

   if (d > 2) {
      int mx = (x1 + x2) / 2;
      int my = (y1 + y2) / 2;
      line_canvas(canvas, x1, y1, mx, my, color);
      line_canvas(canvas, mx, my, x2, y2, color);
   }
}

void
thick_line_canvas(Canvas * canvas, int x1, int y1, int x2, int y2,
                  unsigned int color, int thickness) {
   for (int i = -thickness; i < thickness; i++) {
      line_canvas(canvas, x1 + i, y1 + thickness, x2 + i,
                  y2 + thickness, color);
      line_canvas(canvas, x1 + i, y1 - thickness, x2 + i,
                  y2 - thickness, color);
      line_canvas(canvas, x1 + thickness, y1 + i, x2 + thickness,
                  y2 + i, color);
      line_canvas(canvas, x1 - thickness, y1 + i, x2 - thickness,
                  y2 + i, color);
   }
}

#define THETA_STEP 0.10
void
arc_canvas(Canvas * canvas,
           int center_x, int center_y, int radius,
           int strokewidth, unsigned int strokecolor,
           unsigned int forecolor, double begin_deg, double end_deg) {
   float theta;

   if (end_deg <= begin_deg) {
      end_deg += 360.0;
   }

   if (forecolor != COLOR_NONE) {
      for (theta = begin_deg; theta <= end_deg; theta += THETA_STEP) {
         int x, y;
         x = center_x + radius * cos(theta * M_PI / 180.0);
         y = center_y + radius * sin(theta * M_PI / 180.0);
         line_canvas(canvas, center_x, center_y, x, y, forecolor);
      }
   }
   if (strokecolor != COLOR_NONE) {
      for (theta = begin_deg; theta < end_deg; theta += THETA_STEP) {
         int x1, y1, x2, y2;
         x1 = center_x + ((float)radius -
                          ((float)strokewidth) / 2.0) *
            cos(theta * M_PI / 180.0);
         y1 = center_y + ((float)radius -
                          ((float)strokewidth) / 2.0) *
            sin(theta * M_PI / 180.0);
         x2 = center_x + ((float)radius +
                          ((float)strokewidth) / 2.0) *
            cos(theta * M_PI / 180.0);
         y2 = center_y + ((float)radius +
                          ((float)strokewidth) / 2.0) *
            sin(theta * M_PI / 180.0);
         line_canvas(canvas, x1, y1, x2, y2, strokecolor);
      }
   }
}
