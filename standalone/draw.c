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

#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define INCLUDE_FONT_DATA
#include "draw.h"
#include "trig1.h"

#define FILELINE __FILE__, __LINE__
//#define dprintf(format, ...) printf("%s:%d " format, FILELINE, __VA_ARGS__)
#define dprintf(format, ...)

#define FNORD exit(0)
#define sin(x) FNORD
#define cos(x) FNORD
#define tan(x) FNORD

/// @brief Create a new canvas suitable for drawing on
///
/// @param w The width of the canvas
/// @param h The height of the canvas
/// @param fill The byte value to fill with on initialization
/// @return A new Canvas object
Canvas *new_canvas(int w, int h, unsigned int fill) {
   Canvas *ret = (Canvas *) malloc(sizeof(Canvas));
   ret->w = w;
   ret->h = h;
   ret->data = (unsigned int *)malloc(sizeof(unsigned int) * w * h);
   unsigned int *p = ret->data;
   for (int i = 0; i < w * h; i++) {
      *p++ = fill;
   }
   return ret;
}

/// @brief Deletes an existing Canvas object
///
/// @param canvas The canvas to delete
/// @return void
void delete_canvas(Canvas * canvas) {
   free(canvas->data);
   free(canvas);
}

/// @brief Write raw Canvas data to a file
///
/// @param canvas The Canvas to dump
/// @param fname The filename to write to
/// @return void
void dump_canvas(Canvas * canvas, const char *fname) {
   int fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);

   if (fd == -1) {
      fprintf(stderr, "%s COULD NOT OPEN %s\n", __FUNCTION__, fname);
      exit(-1);
   }

   for (int i = 0; i < canvas->w * canvas->h; i++) {
      canvas->data[i] |= 0xFF000000;
   }

   int ret = write(fd, canvas->data,
         sizeof(unsigned int) * canvas->w * canvas->h);
   if (ret != sizeof(unsigned int) * canvas->w * canvas->h) {
      fprintf(stderr, "%s BAD WRITE %d\n", __FUNCTION__, ret);
      exit(-1);
   }
   close(fd);
}

/// @brief Apply an XOR mask to a canvas
///
/// @param mask The Canvas used as a mask
/// @param target The Canvas written to
/// @return void
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

/// @brief Write a color to an x,y on a canvas
///
/// @param canvas The Canvas to write to
/// @param x The X coordinate to write to
/// @param y The Y coordinate to write to
/// @param color The color to write
/// @return void
void poke_canvas(Canvas * canvas, int x, int y, unsigned int color) {
   if (0 <= x && x < canvas->w) {
      if (0 <= y && y < canvas->h) {
         unsigned int current = canvas->data[y * canvas->w + x];
         if (!ISLOCKED(current) || ISLOCKED(color)) {
            if (color == COLOR_LOCK) {
               canvas->data[y * canvas->w + x] = LOCK(current);
            }
            else {
               if (color != COLOR_XOR) {
                  canvas->data[y * canvas->w + x] = color;
               }
               else {
                  canvas->data[y * canvas->w + x] = current ^ 0x00FFFFFF;
               }
            }
         }
      }
   }
}

/// @brief Write a color to an x,y on a canvas if it equals a certain color
///
/// @param canvas The Canvas to write to
/// @param x The X coordinate to write to
/// @param y The Y coordinate to write to
/// @param color The new color to write
/// @param old The old color to match against
/// @return void
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

/// @brief Get the current color of an x,y coordinate on the canvas
///
/// @param canvas The Canvas to read from
/// @param x The X coordinate to read from
/// @param y The Y coordinate to read from
/// @return The color at x,y on the canvas
unsigned int peek_canvas(Canvas * canvas, int x, int y) {
   if (0 <= x && x < canvas->w) {
      if (0 <= y && y < canvas->h) {
         return canvas->data[y * canvas->w + x];
      }
   }
   return COLOR_NONE;
}

/// @brief A helper function to decode UTF-8 text
///
/// @param p The string pointer
/// @return The unicode value of the first character of the string
static int utf8parse(const char *p, const unsigned char **q) {
   int ret = 0;

   if (p != NULL) {
      *q = (unsigned char *)p;
   }

   if (**q < 0x80) {
      ret = *(*q)++;
      return ret;
   }
   else if ((**q & 0xE0) == 0xC0) {
      // 110xxxxx 10xxxxxx
      ret = *(*q)++ & 0x1F;
      ret <<= 6;
      ret |= *(*q)++ & 0x3F;
      return ret;
   }
   else if ((**q & 0xF0) == 0xE0) {
      // 1110xxxx 10xxxxxx 10xxxxxx
      ret = *(*q)++ & 0x0F;
      ret <<= 6;
      ret |= *(*q)++ & 0x3F;
      ret <<= 6;
      ret |= *(*q)++ & 0x3F;
      return ret;
   }
   else if ((**q & 0xF1) == 0xF0) {
      // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      ret = *(*q)++ & 0x07;
      ret <<= 6;
      ret = *(*q)++ & 0x0F;
      ret <<= 6;
      ret |= *(*q)++ & 0x3F;
      ret <<= 6;
      ret |= *(*q)++ & 0x3F;
      return ret;
   }
   else {
      // TODO FIX what to do in event of malformed?
      // TERMINATE!
      return 0;
   }
}

/// @brief a struct to hold information about a glyph
struct Glyph {
   int glyph;
   int width;
   int height;
   int step;
   int dx;
   int dy;
   uint8_t *data;
};

/// @brief Given a font and a unicode point, find the Glyph
///
/// @param font Pointer to a font
/// @param glyph The unicode point of interest
/// @return The Glyph struct coresponding to the unicode point
static struct Glyph font_find_glyph(uint8_t * font, uint16_t glyph) {
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

/// @brief Draw text on a canvas
///
/// @param canvas The Canvas to draw on
/// @param font A pointer to the font being used
/// @param x The X coordinate of the center of the text
/// @param y The Y coordinate of the center of the text
/// @param fg The foreground color
/// @param bg The background color
/// @param p A pointer to a UTF-8 string
/// @param mult A magnification multiplier
/// @param gap The pixel gap between characters
/// @return An integer encoding the text width and height
int
text_canvas(Canvas * canvas, uint8_t * font, int x, int y, unsigned int fg,
      unsigned int bg, const char *p, int mult, int gap) {

   // mad recursion for multilined strings
   if (strchr(p, '\n')) {
      char buf[1024];
      strcpy(buf, p);
      p = buf;
      int wh[16];
      int h[16];
      int maxw = 0;
      const char *ps[16];
      int i = 0;
      char *q;
      do {
         ps[i] = p;
         q = strchr(p, '\n');
         if (q != NULL) {
            *q = 0;
            q++;
         }
         wh[i++] =
            text_canvas(canvas, font, -1000, -1000, fg, bg, p, mult, gap);
         if ((wh[i-1] >> 16) > maxw) {
            maxw = wh[i-1] >> 16;
         }
         h[i - 1] = wh[i - 1] & 0xFFFF;
         p = q;
      } while (p != NULL && strlen(p));

      int ret;
      int height = 0;
      for (int j = 0; j < i; j++) {
         height += h[j];
      }
      height += (gap*3*(i-1));
      ret = (maxw << 16) | height;

      // now put them where they go...
      for (int j = 0; j < i; j++) {
         int offset = - (height / 2);
         for (int k = 0; k < j; k++) {
            offset += h[k] + gap*3;
         }
         offset += h[j] / 2;
         text_canvas(canvas, font, x, y + offset, fg, bg, ps[j], mult, gap);
      }

      return ret;
   }

   int outline = 2;

   int encoded;

   int max_x = 0;
   int min_x = 0;
   int max_y = 0;
   int min_y = 0;
   int first = 1;
   int fake_x = 0;
   int fake_y = 0;

   const unsigned char *context;

   for (encoded = utf8parse(p, &context);
        encoded;
        encoded = utf8parse(NULL, &context)) {

      struct Glyph glyph = font_find_glyph(font, encoded);
      if (encoded == ' ') {
         glyph = font_find_glyph(font, '_');
      }
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
      for (encoded = utf8parse(p, &context);
           encoded;
           encoded = utf8parse(NULL, &context)) {

         struct Glyph glyph = font_find_glyph(font, encoded);
         if (encoded == ' ') {
            glyph = font_find_glyph(font, '_');
         }
         for (int h = 0; h < glyph.height; h++) {
            for (int w = 0; w < glyph.width; w++) {
               int offset = w / 8;
               int bit = glyph.data[offset] & (1 << (7 - (w % 8)));
               if (bit) {
                  for (int mx = 0; mx < mult; mx++) {
                     for (int my = 0; my < mult; my++) {
                        for (int dx =
                              -mult * outline; dx <= mult * outline; dx++) {
                           for (int dy = -mult * outline; dy <= mult * outline; dy++) {
                              if (encoded != ' ') {
                                 poke_canvas (canvas,
                                    x + w * mult + mx + dx + glyph.dx * mult,
                                    y + h * mult + my + dy + glyph.dy * mult, bg);
                              }
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

   for (encoded = utf8parse(p, &context);
        encoded;
        encoded = utf8parse(NULL, &context)) {

      struct Glyph glyph = font_find_glyph(font, encoded);
      if (encoded == ' ') {
         glyph = font_find_glyph(font, '_');
      }
      for (int h = 0; h < glyph.height; h++) {
         for (int w = 0; w < glyph.width; w++) {
            int offset = w / 8;
            int bit = glyph.data[offset] & (1 << (7 - (w % 8)));
            if (bit) {
               for (int mx = 0; mx < mult; mx++) {
                  for (int my = 0; my < mult; my++) {
                     if (encoded != ' ') {
                        poke_canvas(canvas,
                           x + w * mult + mx + glyph.dx * mult,
                           y + h * mult + my + glyph.dy * mult, fg);
                     }
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

/// @brief Like poke_canvas(), but with a larger area
///
/// @param canvas The Canvas to draw on
/// @param x The X coordinate to center on
/// @param y The Y coordinate to center on
/// @param color The color to draw
/// @param blur A value indicating the size of the poke
/// @return void
void
blur_poke_canvas(Canvas * canvas, int x, int y, unsigned int color, int blur) {
   for (int i = -blur; i <= blur; i++) {
      for (int j = -blur; j <= blur; j++) {
         poke_canvas(canvas, x + i, y + j, color);
      }
   }
}

/// @brief Draw a line
///
/// This function uses recursion to draw the line
///
/// @param canvas The Canvas to draw on
/// @param x1 The start X coordinate
/// @param y1 The start Y coordinate
/// @param x2 The end X coordinate
/// @param y2 The end Y coordinate
/// @param color The color to draw
/// @return void
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

/// @brief Draw a thick line
///
/// This function uses the recursive line_canvas to draw the line
///
/// @param canvas The Canvas to draw on
/// @param x1 The start X coordinate
/// @param y1 The start Y coordinate
/// @param x2 The end X coordinate
/// @param y2 The end Y coordinate
/// @param color The color to draw
/// @param thickness The thickness of the line to be drawn
/// @return void
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

static inline double biggestof4(double a, double b, double c, double d) {
   double ret = a;
   if (b > ret) { ret = b; }
   if (c > ret) { ret = c; }
   if (d > ret) { ret = d; }
   return ret;
}

static inline double smallestof4(double a, double b, double c, double d) {
   double ret = a;
   if (b < ret) { ret = b; }
   if (c < ret) { ret = c; }
   if (d < ret) { ret = d; }
   return ret;
}

static inline void arc_normalize (double *begin_deg, double *end_deg) {
   double b = *begin_deg;
   double e = *end_deg;

   // sane normalization
   // faster than some other solutions which involve division
   while (b < 0.0) {
      b += 360.0;
      e += 360.0;
   }
   while (b >= 360.0) {
      b -= 360.0;
      e -= 360.0;
   }
   while (e < b) {
      e += 360.0;
   }

   *begin_deg = b;
   *end_deg = e;
}

static void
arc_canvas_helper(Canvas * canvas,
      int center_x, int center_y, int radius,
      int strokewidth, unsigned int strokecolor,
      double begin_deg, double end_deg, bool rotated) {

   // assumes we're already broken into pieces of 8

   if (begin_deg == end_deg) {
      // broken edge case, and really, who cares?
      return;
   }

   dprintf("!!! begin_deg=%f end_deg=%f color=%08X radius=%d, spun=%d\n",
      begin_deg, end_deg, strokecolor, radius, rotated);

   arc_normalize(&begin_deg, &end_deg);

   dprintf("222 begin_deg=%f end_deg=%f color=%08X radius=%d, spun=%d\n",
      begin_deg, end_deg, strokecolor, radius, rotated);

   // we are now guaranteed to have a begin and end in
   // the same quadrant, and same octant

   // we are also guaranteed to be in octants 2, 3, 6, or 7
   // (because that's how we get called, with rotated set if
   // we're in octants 1, 4, 5, or 8)

   // stuff we're going to use a lot
   double sin_begin = sin_deg(begin_deg);
   double cos_begin = cos_deg(begin_deg);

   double sin_end = sin_deg(end_deg);
   double cos_end = cos_deg(end_deg);

   double cos_middle = cos_deg((begin_deg + end_deg) / 2.0);

   double inner = (double)radius - round((double)strokewidth / 2.0 + .5);
   double outer = inner + (double) strokewidth;

   dprintf("!!! inner=%f outer=%f\n", inner, outer);

   // we shall consider 4 points:
   // the end point of each of the outer and inner arcs.
   // further, we look only at y values for now.

   double y_outer_begin = outer * sin_begin;
   double y_outer_end   = outer * sin_end;
   double y_inner_begin = inner * sin_begin;
   double y_inner_end   = inner * sin_end;

   // now find the largest and smallest
   double y_large =
      biggestof4(y_outer_begin, y_outer_end, y_inner_begin, y_inner_end);
   double y_small =
      smallestof4(y_outer_begin, y_outer_end, y_inner_begin, y_inner_end);

   y_large = round(y_large);
   y_small = round(y_small);

   dprintf("y_small=%f y_large=%f\n", y_small, y_large);

   // for each scanline
   for (double y = y_small; y <= y_large; y++) {
      // find start and stop points on scanline
      // it'll be on the line, or on the curve
      double square;

      double x_inner;
      square = inner*inner - y*y;
      if (square < 0.0) {
         x_inner = 0.0;
      }
      else {
         x_inner = sqrt(square);
         if (signbit(cos_middle)) {
            x_inner = -x_inner;
         }
      }

      double x_outer;
      square = outer*outer - y*y;
      if (square < 0.0) {
         x_outer = 0.0;
      }
      else {
         x_outer = sqrt(square);
         if (signbit(cos_middle)) {
            x_outer = -x_outer;
         }
      }

      dprintf("]]] y=%f xbl=%f xel=%f\n", y, x_inner, x_outer);

      // TANGENT BASED FIXUPS
      // tangent based fixups

      double tmp;

      if (begin_deg < 90.0) {
         tmp = y * cos_end / sin_end;
         if (tmp < outer) {
            if (x_inner < tmp) {
               dprintf("... y=%f xbl=%f sug=%f\n", y, x_inner, tmp);
               x_inner = tmp;
            }
         }
         else {
            dprintf("### %f %f %f\n", x_inner, tmp, x_outer);
         }

         tmp = y * cos_begin / sin_begin;
         if (tmp < outer) {
            if (x_outer > tmp) {
               dprintf("... y=%g xel=%f sug=%f\n", y, x_outer, tmp);
               x_outer = tmp;
            }
         }
         else {
            dprintf("### %f %f %f\n", x_inner, tmp, x_outer);
         }
      }
      else if (begin_deg < 180.0) {
         tmp = y * cos_begin / sin_begin;
         if (tmp > -outer) {
            if (x_inner > tmp) {
               dprintf("... y=%f xbl=%f sug=%f\n", y, x_inner, tmp);
               x_inner = tmp;
            }
         }
         else {
            dprintf("### %f %f %f\n", x_inner, tmp, x_outer);
         }

         tmp = y * cos_end / sin_end;
         if (tmp > -outer) {
            if (x_outer < tmp) {
               dprintf("... y=%g xel=%f sug=%f\n", y, x_outer, tmp);
               x_outer = tmp;
            }
         }
         else {
            dprintf("### %f %f %f\n", x_inner, tmp, x_outer);
         }
      }
      else if (begin_deg < 270.0) {
         tmp = y * cos_end / sin_end;
         if (tmp > -outer) {
            if (x_inner > tmp) {
               dprintf("... y=%f xbl=%f sug=%f\n", y, x_inner, tmp);
               x_inner = tmp;
            }
         }
         else {
            dprintf("### %f %f %f\n", x_inner, tmp, x_outer);
         }

         tmp = y * cos_begin / sin_begin;
         if (tmp > -outer) {
            if (x_outer < tmp) {
               dprintf("... y=%g xel=%f sug=%f\n", y, x_outer, tmp);
               x_outer = tmp;
            }
         }
         else {
            dprintf("### %f %f %f\n", x_inner, tmp, x_outer);
         }
      }
      else { // < 360.0
         tmp = y * cos_begin / sin_begin;
         if (tmp < outer) {
            if (x_inner < tmp) {
               dprintf("... y=%f xbl=%f sug=%f\n", y, x_inner, tmp);
               x_inner = tmp;
            }
         }
         else {
            dprintf("### %f %f %f\n", x_inner, tmp, x_outer);
         }

         tmp = y * cos_end / sin_end;
         if (tmp < outer) {
            if (x_outer > tmp) {
               dprintf("... y=%g xel=%f sug=%f\n", y, x_outer, tmp);
               x_outer = tmp;
            }
         }
         else {
            dprintf("### %f %f %f\n", x_inner, tmp, x_outer);
         }
      }
      dprintf("[[[ y=%f xbl=%f xel=%f\n", y, x_inner, x_outer);

      double x_small = x_inner;
      double x_large = x_inner;

      if (x_outer < x_small) { x_small = x_outer; }
      if (x_outer > x_large) { x_large = x_outer; }

      x_small = round(x_small);
      x_large = round(x_large);

      for (int x = (int)x_small; x <= (int)x_large; x++) {
         if (!rotated) {
            poke_canvas(canvas, center_x + x, center_y + y, strokecolor);
         }
         else {
            poke_canvas(canvas, center_x - y, center_y + x, strokecolor);
         }
      }
   }
}

/// @brief Draw an arc on the canvas
///
/// This function uses the recursive line_canvas to draw short segments of arc
///
/// @param canvas The Canvas to draw on
/// @param center_x The center X coordinate used by the arc
/// @param center_y The center Y coordinate used by the arc
/// @param radius The radius of the arc
/// @param strokewidth The width of the drawn arc
/// @param strokecolor The color to draw inside the arc
/// @param begin_deg The starting angle for the arc
/// @param end_deg The ending angle for the arc
/// @return void
void
arc_canvas(Canvas * canvas,
      int center_x, int center_y, int radius,
      int strokewidth, unsigned int strokecolor,
      double begin_deg, double end_deg) {

   static const bool rotations[8] = {
      true, false, false, true,
      true, false, false, true
   };

   if (strokecolor == COLOR_NONE) {
      return;
   }

   dprintf("=== begin_deg=%f end_deg=%f color=%08X radius=%d\n",
      begin_deg, end_deg, strokecolor, radius);

   arc_normalize(&begin_deg, &end_deg);

   dprintf(">>> begin_deg=%f end_deg=%f color=%08X radius=%d\n",
      begin_deg, end_deg, strokecolor, radius);

   // sweep eigths
   int i = 0;
   double a = 0.0;
   double b = 45.0;
   while (a <= end_deg) {
      if (!(end_deg < a || begin_deg > b)) {
         bool rotated = rotations[i & 7];
         double rot = rotated ? -90.0 : 0.0;
         double from = fmax(a, begin_deg);
         double to = fmin(b, end_deg);

         arc_canvas_helper(canvas, center_x, center_y, radius,
               strokewidth, strokecolor,
               from+rot, to+rot, rotated);
      }

      i++;
      a = b;
      b += 45.0;
   }
}
