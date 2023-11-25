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
#include <stdbool.h>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <math.h>

#include <locale.h>

#include "precise.h"
#include "astro.h"
#include "draw.h"
#include "moon_xpm.h"

#include "fonts.h"

int SIZE = 1024;

DrawFont mFONT_ASTRO         = (DrawFont) { NULL, 32};
DrawFont mFONT_BOLD_BIG      = (DrawFont) { NULL, 50};
DrawFont mFONT_BOLD_LARGER   = (DrawFont) { NULL, 28};
DrawFont mFONT_BOLD_LARGE    = (DrawFont) { NULL, 24};
DrawFont mFONT_BOLD_MED      = (DrawFont) { NULL, 22};
DrawFont mFONT_BOLD_SMALL    = (DrawFont) { NULL, 18};
DrawFont mFONT_ITALIC_LARGER = (DrawFont) { NULL, 28};
DrawFont mFONT_ITALIC_LARGE  = (DrawFont) { NULL, 24};
DrawFont mFONT_ITALIC_MED    = (DrawFont) { NULL, 22};

#define FONT_ASTRO         (&mFONT_ASTRO)
#define FONT_BOLD_BIG      (&mFONT_BOLD_BIG)
#define FONT_BOLD_LARGER   (&mFONT_BOLD_LARGER)
#define FONT_BOLD_LARGE    (&mFONT_BOLD_LARGE)
#define FONT_BOLD_MED      (&mFONT_BOLD_MED)
#define FONT_BOLD_SMALL    (&mFONT_BOLD_SMALL)
#define FONT_ITALIC_LARGER (&mFONT_ITALIC_LARGER)
#define FONT_ITALIC_LARGE  (&mFONT_ITALIC_LARGE)
#define FONT_ITALIC_MED    (&mFONT_ITALIC_MED)

#define SCALE(x) ((double)((double)(x) * SIZE / 1024))

#define COLOR_SUNUP           COLOR_YELLOW
#define COLOR_CIVIL           COLOR_ORANGE
#define COLOR_NAUTICAL        COLOR_LIGHTBLUE
#define COLOR_ASTRONOMICAL    COLOR_BLUE
#define COLOR_NIGHT           COLOR_DARKBLUE

#define COLOR_LIGHT           COLOR_WHITE
#define COLOR_TWILIGHT        COLOR_LIGHTGRAY
#define COLOR_DARK            COLOR_DARKGRAY

#define COLOR_MERCURY         COLOR_GRAY
#define COLOR_VENUS           COLOR_WHITE
#define COLOR_MARS            COLOR_RED
#define COLOR_JUPITER         COLOR_ORANGE
#define COLOR_SATURN          COLOR_LIGHTBLUE
#define COLOR_ARIES           COLOR_GREEN

#define MOON_R 100

#define X_MOVE_OK 1
#define Y_MOVE_OK 2
#define XY_MOVE_OK (X_MOVE_OK | Y_MOVE_OK)
#define NO_XY_MOVE 0
#define NO_X_MOVE Y_MOVE_OK
#define NO_Y_MOVE X_MOVE_OK

void handle_font_sizes(void) {
   if (mFONT_ASTRO.sft_font != NULL) {
      return;
   }

   mFONT_ASTRO.sft_font         =
      sft_loadmem(astro_font, sizeof(astro_font));

   mFONT_BOLD_BIG.sft_font      =
   mFONT_BOLD_LARGER.sft_font   =
   mFONT_BOLD_LARGE.sft_font    =
   mFONT_BOLD_MED.sft_font      =
   mFONT_BOLD_SMALL.sft_font    =
      sft_loadmem(bold_font, sizeof(bold_font));

   mFONT_ITALIC_LARGER.sft_font =
   mFONT_ITALIC_LARGE.sft_font  =
   mFONT_ITALIC_MED.sft_font    =
      sft_loadmem(italic_font, sizeof(italic_font));

#define FONTSCALE(x) \
   ~1 & ((int)SCALE(x) * 3 / 2)

   mFONT_ASTRO.point         = FONTSCALE(mFONT_ASTRO.point);
   mFONT_BOLD_BIG.point      = FONTSCALE(mFONT_BOLD_BIG.point);
   mFONT_BOLD_LARGER.point   = FONTSCALE(mFONT_BOLD_LARGER.point);
   mFONT_BOLD_LARGE.point    = FONTSCALE(mFONT_BOLD_LARGE.point);
   mFONT_BOLD_MED.point      = FONTSCALE(mFONT_BOLD_MED.point);
   mFONT_BOLD_SMALL.point    = FONTSCALE(mFONT_BOLD_SMALL.point);
   mFONT_ITALIC_LARGER.point = FONTSCALE(mFONT_ITALIC_LARGER.point);
   mFONT_ITALIC_LARGE.point  = FONTSCALE(mFONT_ITALIC_LARGE.point);
   mFONT_ITALIC_MED.point    = FONTSCALE(mFONT_ITALIC_MED.point);
}

struct delayed_text {
   int movable;
   int x, y;
   int w, h;
   unsigned int fg, bg;
   const char *p;
   int mult, gap;
   DrawFont *font;
};

struct delayed_text_queue {
   int size;
   struct delayed_text *queue;
};

static void delayed_text_canvas(
                                 struct delayed_text_queue *dtq,
                                 Canvas * canvas, DrawFont * font,
                                 int x, int y,
                                 unsigned int fg, unsigned int bg,
                                 const char *p, int mult, int gap,
                                 int movable) {
   for (int i = 0; i < dtq->size; i++) {
      struct delayed_text *q = dtq->queue + i;
      if (x == q->x && y == q->y) {
         if (fg == q->fg && bg == q->bg) {
            if (mult == q->mult && gap == q->gap) {
               if (font == q->font) {
                  if (!strcmp(q->p, p)) {
                     return;
                  }
               }
            }
         }
      }
   }

   struct delayed_text *dt = dtq->queue + dtq->size;
   int wh = text_size(font, p, mult, gap);

   dt->movable = movable;
   dt->x = x;
   dt->y = y;
   dt->w = wh >> 16;
   dt->h = wh & 0xFFFF;
   dt->fg = fg;
   dt->bg = bg;
   dt->p = strdup(p);
   dt->mult = mult;
   dt->gap = gap;
   dt->font = font;

   dtq->size++;
}

static struct delayed_text_queue *alloc_dtq(int n) {
   struct delayed_text_queue *ret =
      (struct delayed_text_queue *) malloc (sizeof(struct delayed_text_queue));
   ret->size = 0;
   ret->queue = (struct delayed_text *) malloc (n * sizeof(struct delayed_text));
   return ret;
}

static void resolve_delayed_text(struct delayed_text_queue *dtq, Canvas * canvas) {
   int problems;
   int iterations = 0;

   do {
      problems = 0;

      for (int i = 0; i < dtq->size - 1; i++) {
         struct delayed_text *dti = dtq->queue + i;
         int i_lef = dti->x - (dti->w+7)/2;
         int i_rig = dti->x + (dti->w+7)/2;
         int i_top = dti->y - (dti->h+7)/2;
         int i_bot = dti->y + (dti->h+7)/2;

         for (int j = i + 1; j < dtq->size; j++) {
            struct delayed_text *dtj = dtq->queue + j;
            int j_lef = dtj->x - (dtj->w+7)/2;
            int j_rig = dtj->x + (dtj->w+7)/2;
            int j_top = dtj->y - (dtj->h+7)/2;
            int j_bot = dtj->y + (dtj->h+7)/2;

            // check for overlaps
            if (!(i_bot < j_top || i_top > j_bot || i_rig < j_lef || i_lef > j_rig)) {
               problems++;
               if (dti->x < dtj->x) {
                  if ((dti->movable & X_MOVE_OK) && i_lef > 0) dti->x--;
                  if ((dtj->movable & X_MOVE_OK) && j_rig < (canvas->w - 1)) dtj->x++;
               }
               else if (dti->x > dtj->x) {
                  if ((dti->movable & X_MOVE_OK) && i_rig < (canvas->w - 1)) dti->x++;
                  if ((dtj->movable & X_MOVE_OK) && j_lef > 0) dtj->x--;
               }

               if (dti->y < dtj->y) {
                  if ((dti->movable & Y_MOVE_OK)&& i_top > 0) dti->y--;
                  if ((dtj->movable & Y_MOVE_OK)&& j_bot < (canvas->h - 1)) dtj->y++;
               }
               else if (dti->y > dtj->y) {
                  if ((dti->movable & Y_MOVE_OK)&& i_bot < (canvas->h - 1)) dti->y++;
                  if ((dtj->movable & Y_MOVE_OK)&& j_top > 0) dtj->y--;
               }
            }
         }
      }
      iterations++;
      // fprintf(stderr, "problems=%d iterations=%d\n", problems, iterations);
   } while (problems && iterations < 9999);

   for (int i = 0; i < dtq->size; i++) {
      struct delayed_text *dt = dtq->queue + i;
      text_canvas(canvas, dt->font, dt->x, dt->y,
         dt->fg, dt->bg, dt->p, dt->mult, dt->gap);
      free((void *)dt->p);
      dt->p = NULL;
   }
   free(dtq->queue);
   dtq->queue = NULL;
   free(dtq);
   dtq = NULL;
}

#define text_canvas(a,b,c,d,e,f,g,h,i) \
   delayed_text_canvas(dtq,a,b,c,d,e,f,g,h,i,XY_MOVE_OK)
#define im_text_canvas(a,b,c,d,e,f,g,h,i,k) \
   delayed_text_canvas(dtq,a,b,c,d,e,f,g,h,i,k)

#if 0
static void set_font(uint8_t ** target,
                     uint8_t ** choices,
                     int desire,
                     int width) {
   int close = 0;
   // want closest desire/1024 to height/width
   int truth = desire * width / 1024;
   int delta = abs(choices[0][1] - truth);

   for (int i = 0; choices[i] != NULL; i++) {
      int tmp = abs(choices[i][1] - truth);
      if (tmp < delta) {
         delta = tmp;
         close = i;
      }
   }
   *target = choices[close];
}
#endif

/// @brief Draw the "now" hand
///
/// The now hand is drawn using xor logic
///
/// @param canvas The Canvas to draw on
/// @param now_angle The angle which is now
/// @return void
void do_now_hand(Canvas * canvas, double now_angle) {
   double xc =
      canvas->w / 2.0 + (canvas->w / 8.0) * cos_deg(now_angle);
   double yc =
      canvas->h / 2.0 + (canvas->h / 8.0) * sin_deg(now_angle);
   double xc2 =
      canvas->w / 2.0 + (canvas->w / 2.0) * cos_deg(now_angle);
   double yc2 =
      canvas->h / 2.0 + (canvas->h / 2.0) * sin_deg(now_angle);

   thick_line_canvas_semilock(canvas, (int)xc2, (int)yc2, (int)xc, (int)yc, COLOR_RED, 3);
}

/// @brief Draw ticks every hour around the outside edge
///
/// Ticks are drawn using xor logic, so they should always be visible
///
/// @param dtq The delayed text queue
/// @param canvas The Canvas to draw on
/// @param now The current time_t
/// @param r The radius of the clock
/// @param now_angle The angle used as "now"
/// @return void
void do_hour_ticks(struct delayed_text_queue *dtq, Canvas * canvas, time_t now, int r, double now_angle) {
   int x = SIZE / 2;
   int y = SIZE / 2;

   time_t ticktime = now - 12 * 60 * 60;
   struct tm local = *localtime(&ticktime);
   time_t surplus = local.tm_min * 60 +  local.tm_sec;
   ticktime += ((60 * 60) - surplus);
   ticktime++;

   for (int i = 0; i < 24; i++) {
      local = *localtime(&ticktime);

      double angle = now_angle - 180.0 +
         360.0 *
         (double)(ticktime - (now - (12 * 60 * 60))) /
         (double)(24 * 60 * 60);

      double xa, ya;
      double xc, yc;
      xa = x + (r - (14/2)) * cos_deg(angle);
      ya = y + (r - (14/2)) * sin_deg(angle);
      xc = x + (r + (14/2)) * cos_deg(angle);
      yc = y + (r + (14/2)) * sin_deg(angle);

      line_canvas(canvas, (int)xa, (int)ya, (int)xc, (int)yc, COLOR_RED);

      xa = x + (r - SCALE(30)) * cos_deg(angle);
      ya = y + (r - SCALE(30)) * sin_deg(angle);
      char buf[32];
      sprintf(buf, "%02d", local.tm_hour);

      text_canvas(canvas, FONT_BOLD_SMALL, (int)xa, (int)ya,
            COLOR_XOR, COLOR_NONE, buf, 1, 3);

      ticktime += 60*60;
   }
}

void duration_text(struct delayed_text_queue *dtq, Canvas *canvas,
                   const char *text,
                   double degrees,
                   unsigned int color,
                   int y) {
   char buf[128];

   degrees /= 15.0;

   int h = degrees;
   int m = (degrees - (double) (h)) * 60.0;

   if (h < 1.0) {
      sprintf(buf, "%s\n%dm", text, m);
   }
   else if (m == 0) {
      sprintf(buf, "%s\n%dh", text, h);
   }
   else {
      sprintf(buf, "%s\n%dh%dm", text, h, m);
   }

   unsigned int bg = COLOR_WHITE;
   if (color == COLOR_WHITE) { bg = COLOR_BLACK; }

   text_canvas(canvas, FONT_BOLD_MED, SIZE / 2, y,
      color, bg, buf, 1, 3);
}

void duration_top_text(struct delayed_text_queue *dtq,
                       Canvas *canvas,
                       const char *text,
                       double degrees,
                       unsigned int color) {
   duration_text(dtq, canvas, text, degrees, color, SIZE * 5 / 32);
}

void duration_bottom_text(struct delayed_text_queue *dtq,
                          Canvas *canvas,
                          const char *text,
                          double degrees,
                          unsigned int color) {
   duration_text(dtq, canvas, text, degrees, color, SIZE * 27 / 32);
}

void edgetime(struct delayed_text_queue *dtq,
              Canvas *canvas,
              time_t now,
              int index,
              double now_angle,
              unsigned int color) {

   time_t when = now - 12 * 60 * 60 + index * 60;
   struct tm tm = *localtime(&when);

   char buf[128];
   sprintf(buf, "%02d:%02d", tm.tm_hour, tm.tm_min);

   int cx = SIZE / 2;
   int cy = SIZE / 2;

   double theta = now_angle - 180.0 + (double) index * 360.0 / (24.0*60.0);
   ZRANGE(theta, 360.0);

   int r;

   if (theta > 270.0 - 45.0 && theta < 270.0 + 45.0) {
      r = (SIZE/2) * 13 / 16;
   }
   else {
      r = (SIZE/2) * 13 / 16;
   }

   int x = cx + r * cos_deg(theta);
   int y = cy + r * sin_deg(theta);

   unsigned int bg = COLOR_WHITE;
   if (color == COLOR_WHITE) { bg = COLOR_BLACK; }

   text_canvas(canvas,
               index < (12 * 60) ? FONT_ITALIC_LARGER : FONT_BOLD_LARGER,
               x, y,
               color, bg, buf, 1, 3);
}

double do_sun_bands(struct delayed_text_queue *dtq,
                    Canvas *canvas,
                    time_t now,
                    double jd,
                    struct φλ φλ,
                    int lightdark) {

   static const double HORIZON_SUN = -1.0;
   static const double HORIZON_CIVIL = -6.56;
   static const double HORIZON_NAUTICAL = -12.56;
   static const double HORIZON_ASTRONOMICAL = -18.56;

   double night = 0.0;
   double astronomical = 0.0;
   double nautical = 0.0;
   double civil = 0.0;
   double sunup = 0.0;

   double light = 0.0;
   double dark = 0.0;

   unsigned int lightdark_civil = COLOR_TWILIGHT;
   unsigned int lightdark_nautical = COLOR_TWILIGHT;
   unsigned int lightdark_astronomical = COLOR_TWILIGHT;

   if (lightdark != 0) {
      int light = lightdark >> 8;
      int dark = lightdark & 0xFF;

      switch (light) {
         case 3: lightdark_astronomical = COLOR_LIGHT; // fallthrough
         case 2: lightdark_nautical = COLOR_LIGHT;     // fallthrough
         case 1: lightdark_civil = COLOR_LIGHT;        // fallthrough
      }

      switch (dark) {
         case 3: lightdark_civil = COLOR_DARK;        // fallthrough
         case 2: lightdark_nautical = COLOR_DARK;     // fallthrough
         case 1: lightdark_astronomical = COLOR_DARK; // fallthrough
      }
   }

   double a[24*60 + 15];
   for (int i = 0; i < 24 * 60 + 15; i++) {
      double when = jd - 0.5 + (double) i / (24.0* 60.0);
      struct Aa Aa = ə25(when, φλ, ə27(when, ə46(when)));
      a[i] = Aa.a;
   }

   int max = -1;
   for (int i = 1; i < 24 * 60 + 15 - 1; i++) {
      if (a[i] >= a[i-1] && a[i] >= a[i + 1]) {
         max = i;
      }
   }

   if (max == -1) {
      if (a[0] > a[24 * 60 - 1]) {
         max = 0;
      }
      else {
         max = 24 * 60 - 1;
      }
   }

   //double now_angle = -180.0 + (360.0 * (double) max / (24.0 * 60.0));
   double now_angle = -180.0 + ((double) max / 4.0);
   now_angle = 270.0 - now_angle;
   ZRANGE(now_angle, 360.0);

   unsigned int color;
   unsigned int oldcolor;
   unsigned int bandcolor;
   unsigned int oldbandcolor;

   if (a[0] < HORIZON_ASTRONOMICAL) {
      color = COLOR_NIGHT;
      bandcolor = COLOR_DARK;
   }
   else if (a[0] < HORIZON_NAUTICAL) {
      color = COLOR_ASTRONOMICAL;
      bandcolor = lightdark_astronomical;
   }
   else if (a[0] < HORIZON_CIVIL) {
      color = COLOR_NAUTICAL;
      bandcolor = lightdark_nautical;
   }
   else if (a[0] < HORIZON_SUN) {
      color = COLOR_CIVIL;
      bandcolor = lightdark_civil;
   }
   else {
      color = COLOR_SUNUP;
      bandcolor = COLOR_LIGHT;
   }

   oldcolor = color;
   oldbandcolor = bandcolor;

   double start_angle = now_angle - 180.0;

   for (int i = 1; i < 24*60; i++) {
      if (a[i-1] < HORIZON_SUN && a[i] >= HORIZON_SUN) {
         color = COLOR_SUNUP;
         bandcolor = COLOR_LIGHT;
      }
      if (a[i-1] >= HORIZON_SUN && a[i] < HORIZON_SUN) {
         color = COLOR_CIVIL;
         bandcolor = lightdark_civil;
      }
      if (a[i-1] < HORIZON_CIVIL && a[i] >= HORIZON_CIVIL) {
         color = COLOR_CIVIL;
         bandcolor = lightdark_civil;
      }
      if (a[i-1] >= HORIZON_CIVIL && a[i] < HORIZON_CIVIL) {
         color = COLOR_NAUTICAL;
         bandcolor = lightdark_nautical;
      }
      if (a[i-1] < HORIZON_NAUTICAL && a[i] >= HORIZON_NAUTICAL) {
         color = COLOR_NAUTICAL;
         bandcolor = lightdark_nautical;
      }
      if (a[i-1] >= HORIZON_NAUTICAL && a[i] < HORIZON_NAUTICAL) {
         color = COLOR_ASTRONOMICAL;
         bandcolor = lightdark_astronomical;
      }
      if (a[i-1] < HORIZON_ASTRONOMICAL && a[i] >= HORIZON_ASTRONOMICAL) {
         color = COLOR_ASTRONOMICAL;
         bandcolor = lightdark_astronomical;
      }
      if (a[i-1] >= HORIZON_ASTRONOMICAL && a[i] < HORIZON_ASTRONOMICAL) {
         color = COLOR_NIGHT;
         bandcolor = COLOR_DARK;
      }
      if (color != oldcolor) {
         double stop_angle =
            now_angle - 180.0 + 360.0 * ((double) i / (24.0 * 60.0));

#define RADIUS(inner,outer) (((outer) + (inner)) / 2)
#define STROKE(inner,outer) (((outer) - (inner)))

         arc_canvas(canvas,
            SIZE / 2, SIZE / 2,
            RADIUS(SIZE/8, SIZE/2),
            STROKE(SIZE/8, SIZE/2),
            oldcolor, start_angle, stop_angle);

         switch(oldcolor) {
            case COLOR_SUNUP:
               sunup += (stop_angle - start_angle);
               break;
            case COLOR_CIVIL:
               civil += (stop_angle - start_angle);
               break;
            case COLOR_NAUTICAL:
               nautical += (stop_angle - start_angle);
               break;
            case COLOR_ASTRONOMICAL:
               astronomical += (stop_angle - start_angle);
               break;
            case COLOR_NIGHT:
               night += (stop_angle - start_angle);
               break;
         }

         // borders
         arc_canvas(canvas,
               SIZE / 2, SIZE / 2, SIZE / 8 + (14/2), 14,
               oldbandcolor, start_angle, stop_angle);
         arc_canvas(canvas,
               SIZE / 2, SIZE / 2, SIZE / 2 - (14/2), 14,
               oldbandcolor, start_angle, stop_angle);

         switch (oldbandcolor) {
            case COLOR_LIGHT:
               light += (stop_angle - start_angle);
               break;
            case COLOR_DARK:
               dark += (stop_angle - start_angle);
               break;
         }

         oldcolor = color;
         oldbandcolor = bandcolor;
         start_angle = stop_angle;
      }
   }

   double stop_angle =
      now_angle + 180.0;

   arc_canvas(canvas,
      SIZE / 2, SIZE / 2,
      RADIUS(SIZE/8, SIZE/2),
      STROKE(SIZE/8, SIZE/2),
      color, start_angle, stop_angle);

   switch(oldcolor) {
      case COLOR_SUNUP:
         sunup += (stop_angle - start_angle);
         break;
      case COLOR_CIVIL:
         civil += (stop_angle - start_angle);
         break;
      case COLOR_NAUTICAL:
         nautical += (stop_angle - start_angle);
         break;
      case COLOR_ASTRONOMICAL:
         astronomical += (stop_angle - start_angle);
         break;
      case COLOR_NIGHT:
         night += (stop_angle - start_angle);
         break;
   }

   // borders
   arc_canvas(canvas,
         SIZE / 2, SIZE / 2, SIZE / 8 + (14/2), 14,
         oldbandcolor, start_angle, stop_angle);
   arc_canvas(canvas,
         SIZE / 2, SIZE / 2, SIZE / 2 - (14/2), 14,
         oldbandcolor, start_angle, stop_angle);

   switch (oldbandcolor) {
      case COLOR_LIGHT:
         light += (stop_angle - start_angle);
         break;
      case COLOR_DARK:
         dark += (stop_angle - start_angle);
         break;
   }

   do_now_hand(canvas, now_angle);

   do_hour_ticks(dtq, canvas, now, SIZE / 2 - (14/2), now_angle);

   if (lightdark == 0) {
      bool skiplower = false;

      // what to put at the top?
      if (sunup > 0.0) {
         duration_top_text(dtq, canvas, "sun up", sunup, COLOR_BLACK);
         if (civil == 0.0) {
            skiplower = true;
         }
      }
      else if (civil > 0.0) {
         duration_top_text(dtq, canvas, "civil", civil, COLOR_BLACK);
         if (nautical == 0.0) {
            skiplower = true;
         }
      }
      else if (nautical > 0.0) {
         duration_top_text(dtq, canvas, "nautical", nautical, COLOR_WHITE);
         if (astronomical == 0.0) {
            skiplower = true;
         }
      }
      else if (astronomical > 0.0) {
         duration_top_text(dtq, canvas, "astronomical", astronomical, COLOR_WHITE);
         if (night == 0.0) {
            skiplower = true;
         }
      }
      else if (night > 0.0) {
         duration_top_text(dtq, canvas, "night", night, COLOR_WHITE);
         skiplower = true;
      }

      // what to put at the bottom
      if (!skiplower) {
         if (night > 0.0) {
            duration_bottom_text(dtq, canvas, "night", night, COLOR_WHITE);
         }
         else if (astronomical > 0.0) {
            duration_bottom_text(dtq, canvas, "astronomical", astronomical, COLOR_WHITE);
         }
         else if (nautical > 0.0) {
            duration_bottom_text(dtq, canvas, "nautical", nautical, COLOR_WHITE);
         }
         else if (civil > 0.0) {
            duration_bottom_text(dtq, canvas, "civil", civil, COLOR_BLACK);
         }
         else if (sunup > 0.0) {
            // what madness is this ?!?!?
            duration_bottom_text(dtq, canvas, "MADNESS", sunup, COLOR_BLACK);
         }
      }
   }
   else {
      // what to put at the top?
      double twilight = 360.0 - light - dark;

      if (light > 0.0) {
         duration_top_text(dtq, canvas, "light", light, COLOR_BLACK);
      }
      else if (twilight > 0.0) {
         duration_top_text(dtq, canvas, "twilight", twilight, COLOR_BLACK);
      }

      if (dark > 0.0) {
         duration_bottom_text(dtq, canvas, "dark", dark, COLOR_WHITE);
      }
      else if (twilight > 0.0 && light > 0.0) {
         duration_bottom_text(dtq, canvas, "twilight", twilight, COLOR_WHITE);
      }
   }

   for (int i = 1; i < 24*60; i++) {
      if (a[i-1] < HORIZON_SUN && a[i] >= HORIZON_SUN) {
         edgetime(dtq, canvas, now, i, now_angle, COLOR_BLACK);
      }
      if (a[i-1] >= HORIZON_SUN && a[i] < HORIZON_SUN) {
         edgetime(dtq, canvas, now, i, now_angle, COLOR_BLACK);
      }
      if (a[i-1] < HORIZON_CIVIL && a[i] >= HORIZON_CIVIL) {
         edgetime(dtq, canvas, now, i, now_angle, COLOR_BLACK);
      }
      if (a[i-1] >= HORIZON_CIVIL && a[i] < HORIZON_CIVIL) {
         edgetime(dtq, canvas, now, i, now_angle, COLOR_BLACK);
      }
      if (a[i-1] < HORIZON_NAUTICAL && a[i] >= HORIZON_NAUTICAL) {
         edgetime(dtq, canvas, now, i, now_angle, COLOR_WHITE);
      }
      if (a[i-1] >= HORIZON_NAUTICAL && a[i] < HORIZON_NAUTICAL) {
         edgetime(dtq, canvas, now, i, now_angle, COLOR_WHITE);
      }
      if (a[i-1] < HORIZON_ASTRONOMICAL && a[i] >= HORIZON_ASTRONOMICAL) {
         edgetime(dtq, canvas, now, i, now_angle, COLOR_WHITE);
      }
      if (a[i-1] >= HORIZON_ASTRONOMICAL && a[i] < HORIZON_ASTRONOMICAL) {
         edgetime(dtq, canvas, now, i, now_angle, COLOR_WHITE);
      }
   }

   unsigned int maxcolor = COLOR_BLACK;
   if (sunup == 0.0 && civil == 0.0) {
      maxcolor = COLOR_WHITE;
   }
   edgetime(dtq, canvas, now, max, now_angle, maxcolor);

   return now_angle;
}

/// @brief Draw the moon
///
/// @param canvas The Canvas to draw on
/// @param jd The current Julian Date
/// @param cx Center X coordinate
/// @param cy Center Y coordinate
/// @param radius Radius of drawing
/// @param FD computed FD data
/// @param is_up Whether or not the moon is currently up
/// @param bla The angle position of the moon's bright limb
/// @param rot The visual rotation angle
/// @return void
void
do_moon_draw_helper(Canvas * canvas,
             double jd,
             int cx,
             int cy,
             int radius,
             struct FD FD,
             bool is_up,
             double bla,
             double rot) {

   // for u,v calc, below...
   double blarot = -(90.0 - bla);
   double blarot_cos = cos_deg(blarot);
   double blarot_sin = sin_deg(blarot);

   // do the drawing
   int x, y;

   for (y = -radius; y < radius; y++) {
      for (x = -radius; x < radius; x++) {
         if (x*x+y*y < radius*radius) {
            // within the circle.

            // from https://en.wikipedia.org/wiki/Rotation_matrix
            // ey is the rotated form of x
            // ez is the rotated form of y
            double ey =
               ((double)x * cos_deg(rot) - (double)y * sin_deg(rot)) / (double) radius;
            double ez =
               ((double)x * sin_deg(rot) + (double)y * cos_deg(rot)) / (double) radius;

            // find ex on the unit sphere
            double ex = sqrt(1.0 - ey * ey - ez * ez);

            // also calculate the u,v (rotated bright limb to 90 degrees)
            double u = ((double)ey * blarot_cos - (double)ez * blarot_sin);
            double v = ((double)ey * blarot_sin + (double)ez * blarot_cos);
            double uvt = asin_deg(v);
            double ru = cos_deg(uvt);
            double lu = -ru;
            bool dark = ((u - lu) / (ru - lu)) > FD.F;

            // find lat lon
            int my = 180 - (int) acos_deg(ez / 1.0);
            int mx = 180 + (ey > 0.0 ? 1 : -1) *
               (int) acos_deg(ex / sqrt(ex*ex+ey*ey));

            //fprintf(stderr, "%d %d => %lf %lf %lf => %d %d\n",
            //   x, y, ex, ey, ez, mx, my);

            if (my < 0) my = 0;
            if (my > 179) my = 179;
            if (mx < 0) mx = 0;
            if (mx > 359) mx = 359;

            unsigned int c = moon_xpm_palette[moon_xpm_pixels[my][mx]];

            if (dark) {
               c = COLOR_DRAGON(48); //COLOR_MOONBAND;
            }

            if ((cx+x+cy+y) % 2) {
               poke_canvas(canvas, cx + x, cy + y, LOCK(c));
            }
            else {
               poke_canvas(canvas, cx + x, cy + y, c);
            }
         }
      }
   }

   arc_canvas(canvas, cx, cy, radius + 3, 7,
      is_up ? COLOR_WHITE : COLOR_BLACK, 0, 360.0);
}

void
do_moon_draw_tf(Canvas * canvas,
             double jd,
             bool is_up,
             struct φλ φλ,
             bool debug) {

   struct αδ moon = ə27(jd, ə65(jd));
   double brightlimbangle =
      ə68(ə27(jd, ə46(jd)), moon);

   struct Aa Aa_moon = ə25(jd, φλ, moon);
   struct Aa Aa_sun = ə25(jd, φλ, ə27(jd, ə46(jd)));
   // big A is azimuth

   double brng;

   // TODO FIX: this can likely be optimized
   // from https://stackoverflow.com/questions/22392045/calculating-moon-face-rotation-as-a-function-of-earth-coordinates
   {
      double dLon = Aa_sun.A - Aa_moon.A;
      double y = sin_deg(dLon) * cos_deg(Aa_sun.a);
      double x = cos_deg(Aa_moon.a) * sin_deg(Aa_sun.a) - sin_deg(Aa_moon.a) * cos_deg(Aa_sun.a) * cos_deg(dLon);
      brng = atan2_deg(y, x);

      // equation requires adjustment
      brng += brightlimbangle;

      // equation gives clockwise, we want anticlockwise
      brng = -brng;
   }

   struct FD FD = ə67(jd);

   if (!debug) {
      // WHERE to draw it.
      double where_angle = FD.D - 90.0;
      int cx, cy;
      cx = canvas->w / 2 +
         (int)((canvas->w * 17.5 / 48.0) * cos_deg(where_angle));
      cy = canvas->h / 2 +
         (int)((canvas->h * 17.5 / 48.0) * sin_deg(where_angle));
      int radius = SCALE(MOON_R);

#if 0
      // PLOT TWIST!
      radius = (canvas->w / 2) * (sqrt(2.0) - 1.0) / 2.5;
      cx = canvas->w - radius - SCALE(10);
      cy = radius + SCALE(10);
#endif

      do_moon_draw_helper(canvas, jd, cx, cy, radius,
            FD, is_up, brightlimbangle, brng);
   }
   else {
      printf("bla=%lf brng=%lf\n", brightlimbangle, brng);
      do_moon_draw_helper(canvas, jd, 512, 512, 512,
                         FD, is_up, brightlimbangle, brng);
   }
}

void
do_moon_draw(Canvas * canvas,
             double jd,
             bool is_up,
             struct φλ φλ) {
   do_moon_draw_tf(canvas, jd, is_up, φλ, false);
}

void
do_moon_draw_debug(Canvas * canvas,
             double jd,
             bool is_up,
             struct φλ φλ) {
   do_moon_draw_tf(canvas, jd, is_up, φλ, true);
}

void do_planet_bands(struct delayed_text_queue *dtq,
                     Canvas *canvas,
                     double now_angle,
                     double jd,
                     struct φλ φλ) {

   static const int HORIZON = 0.0;
   static const unsigned int colors[] = {
      COLOR_MOONBAND,
      COLOR_MERCURY,
      COLOR_VENUS,
      COLOR_MARS,
      COLOR_JUPITER,
      COLOR_SATURN,
      COLOR_ARIES,
   };
   static const char *syms = "BCDEFGa";

   // initial
   double radius = SIZE / 8 + SCALE(32);

   for (int p = 0; p < 7; p++) {
      int ticked = 0;
      double a[24*60 + 2];
      for (int i = 0; i < 24 * 60 + 2; i++) {
         double when = jd - 0.5 + (double) (i - 1) / (24.0* 60.0);
         struct Aa Aa;

         if (p == 0) Aa = ə25(when, φλ, ə27(when, ə65(when)));
         else if (p < 3) Aa = ə25(when, φλ, ə54(when, p - 1));
         else if (p < 6) Aa = ə25(when, φλ, ə54(when, p));
         else Aa = ə25(when, φλ, (struct αδ) { 0.0, 0.0 });

         a[i] = Aa.a;
      }

      int max = -1;
      for (int i = 1; i < 24 * 60 + 1; i++) {
         if (a[i] >= a[i - 1] && a[i] >= a[i + 1]) {
            max = i;
         }
      }

      if (max == -1) {
         if (a[0] > a[24 * 60 + 1]) {
            max = 0;
         }
         else {
            max = 24 * 60 + 1;
         }
      }

      unsigned int color = COLOR_NONE;

      if (a[0] > HORIZON) {
         color = colors[p];
      }
      unsigned int oldcolor = color;

      double start_angle = now_angle - 180.0;

      for (int i = 1; i < 24 * 60 + 2; i++) {
         if (a[i-1] < HORIZON && a[i] >= HORIZON) {
            color = colors[p];
         }
         if (a[i-1] >= HORIZON && a[i] < HORIZON) {
            color = COLOR_NONE;
         }

         if (color != oldcolor || i == 24 * 60 + 1) {
            double stop_angle =
               now_angle - 180.0 + 360.0 * ((double) (i - 1) / (24.0 * 60.0));

            if (oldcolor != COLOR_NONE) {
               arc_canvas(canvas, SIZE / 2, SIZE / 2,
                     radius, SCALE(16), COLOR_BLACK, start_angle, stop_angle);
            }
            arc_canvas(canvas, SIZE / 2, SIZE / 2,
                  radius, SCALE(12), oldcolor, start_angle, stop_angle);
            oldcolor = color;
            start_angle = stop_angle;
         }
      }
      for (int i = 1; i < 24 * 60 + 2; i++) {
         bool rising = (a[i-1] < HORIZON && a[i] >= HORIZON);
         bool setting = (a[i-1] >= HORIZON && a[i] < HORIZON);

         if (rising || setting) {
            ticked++;

            double stop_angle =
               now_angle - 180.0 + 360.0 * ((double) (i - 1) / (24.0 * 60.0));

            double x1, y1, x2, y2;

            x1 = canvas->w / 2 + (radius - 16) * cos_deg(stop_angle);
            y1 = canvas->h / 2 + (radius - 16) * sin_deg(stop_angle);
            x2 = canvas->w / 2 + (radius + 16) * cos_deg(stop_angle);
            y2 = canvas->h / 2 + (radius + 16) * sin_deg(stop_angle);

            thick_line_canvas(canvas, x1, y1, x2, y2, COLOR_BLACK, 3);
            line_canvas(canvas, x1, y1, x2, y2, colors[p]);

            char sym[2] = { syms[p], 0 };
            double offset = (rising ? -7.0 : 7.0);
            int x = (SIZE / 2) + radius * cos_deg(stop_angle + offset);
            int y = (SIZE / 2) + radius * sin_deg(stop_angle + offset);
            text_canvas(canvas, FONT_ASTRO, x, y, colors[p], COLOR_BLACK, sym, 1, 1);
         }
      }
      if (a[max] > HORIZON) {
         double stop_angle =
            now_angle - 180.0 + 360.0 * ((double) (max - 1) / (24.0 * 60.0));

         double x1, y1, x2, y2;

         x1 = canvas->w / 2 + (radius - 16) * cos_deg(stop_angle);
         y1 = canvas->h / 2 + (radius - 16) * sin_deg(stop_angle);
         x2 = canvas->w / 2 + (radius + 16) * cos_deg(stop_angle);
         y2 = canvas->h / 2 + (radius + 16) * sin_deg(stop_angle);

         thick_line_canvas(canvas, x1, y1, x2, y2, COLOR_BLACK, 3);
         line_canvas(canvas, x1, y1, x2, y2, colors[p]);

         if (!ticked) {
            char sym[2] = { syms[p], 0 };
            double offset = -7.0;
            int x = (SIZE / 2) + radius * cos_deg(stop_angle + offset);
            int y = (SIZE / 2) + radius * sin_deg(stop_angle + offset);
            text_canvas(canvas, FONT_ASTRO, x, y, colors[p], COLOR_BLACK, sym, 1, 1);
         }
      }

      // update
      radius += SCALE(20);
      max = -1;

      if (p == 0) {
         do_moon_draw(canvas, jd, a[12*60] > HORIZON, φλ);
      }
   }
}

/// @brief Draw the location in the center of the Canvas
///
/// @param dtq The delayed text queue
/// @param canvas The Canvas to draw on
/// @param now The current time_t
/// @param φλ The lat/long of the observer position
/// @param monam Array of month names for localization, Jan=0
/// @param wenam Array of weekday names for localization, Sun=0
/// @return void
void do_center(struct delayed_text_queue *dtq,
      Canvas * canvas, time_t now, struct φλ φλ,
      const char *monam[], const char *wenam[]) {

   {
      char location[128];

      double lat = φλ.φ;
      char NS = 'N';
      if (lat < 0.0) {
         lat = -lat;
         NS = 'S';
      }

      double lon = φλ.λ;
      char EW = 'E';
      if (lon < 0.0) {
         lon = -lon;
         EW = 'W';
      }
      char *degree = "\u00B0";     // in utf8, degree symbol

      if (lat > 90.0 || lon > 180.0 || lat < -90.0 || lon < -180.0) {
         sprintf(location, "INVALID LOCATION");

         // border bands
         arc_canvas(canvas,
             SIZE / 2, SIZE / 2, SIZE / 8 + (14/2), 14,
               COLOR_GRAY, 0, 360.0);
         arc_canvas(canvas,
               SIZE / 2, SIZE / 2, SIZE / 2 - (14/2), 14,
               COLOR_GRAY, 0, 360.0);

      }
      else {
         sprintf(location, "%0.2f%s%c,%0.2f%s%c", lat, degree, NS, lon, degree,
               EW);
      }
      im_text_canvas(canvas, FONT_BOLD_SMALL, canvas->w / 2,
            canvas->h / 2 + (int)SCALE(48), COLOR_WHITE, COLOR_BLACK,
            location, 1, 2, NO_X_MOVE);
   }

   struct tm local = *localtime(&now);

   {
      char time[32];

      sprintf(time, "%04d-%02d-%02d",
            local.tm_year + 1900,
            local.tm_mon + 1,
            local.tm_mday);

      im_text_canvas(canvas, FONT_BOLD_SMALL, canvas->w / 2,
            canvas->h / 2 + (int)SCALE(72), COLOR_WHITE, COLOR_BLACK, time,
            1, 2, NO_X_MOVE);
   }

   {
      char time[32];

      sprintf(time, "%02d:%02d", local.tm_hour, local.tm_min);

      im_text_canvas(canvas, FONT_BOLD_BIG, canvas->w / 2, canvas->h / 2,
            COLOR_WHITE, COLOR_BLACK, time, 1, 12, NO_XY_MOVE);
   }

   {
      char text[64];
      if (!wenam) {
         strftime(text, sizeof(text), "%A", &local);
      }
      else {
         sprintf(text, "%s", wenam[local.tm_wday]);
      }
      im_text_canvas(canvas, FONT_BOLD_MED, canvas->w / 2,
            canvas->h / 2 - (int)SCALE(80), COLOR_WHITE, COLOR_BLACK, text,
            1, 2, NO_X_MOVE);
   }

   {
      char text[64];
      if (!monam) {
         strftime(text, sizeof(text), "%B", &local);
      }
      else {
         sprintf(text, "%s", monam[local.tm_mon]);
      }
      sprintf(text + strlen(text), "-%d", local.tm_mday);
      im_text_canvas(canvas, FONT_BOLD_MED, canvas->w / 2,
            canvas->h / 2 - (int)SCALE(50), COLOR_WHITE, COLOR_BLACK, text,
            1, 2, NO_X_MOVE);
   }
}

/// @brief Draw debugging information
///
/// Timezone in lower left, Julian Date in lower right
///
/// @param dtq The delayed text queue
/// @param canvas The Canvas to draw on
/// @param now The current time_t
/// @param JD The current Julian Date
/// @param offset The offset passed into do_clock
/// @param tzProvider Name of the timezone provider
/// @return void
void do_debug_info(struct delayed_text_queue *dtq,
                   Canvas * canvas,
                   time_t now,
                   double JD,
                   double offset,
                   const char *tzProvider) {

   // buffer for julian date
   char jd_buf[1024];
   sprintf(jd_buf, "JD=%0.6f", JD);

   // buffer for offset
   char offset_buf[1024];
   if (offset != 0.0) {
      sprintf(offset_buf, "offset=%0.2f", offset);
   }
   else {
      offset_buf[0] = 0;
   }

   // buffer for tz abbreviation
   char abbrev_buf[1024];
   struct tm *tm = localtime(&now);
   if (tzname[0] != NULL && (tzname[1] != NULL && tzname[1][0] != 0)) {
      sprintf(abbrev_buf, "\a%s%s%s/%s%s%s",
            tm->tm_isdst ? "" : "\a[",
            tzname[0],
            tm->tm_isdst ? "" : "]\a",
            tm->tm_isdst ? "\a[" : "", tzname[1], tm->tm_isdst ? "]\a" : "");
   }
   else if (tzname[0] != NULL) {
      sprintf(abbrev_buf, "[%s]", tzname[0]);
   }
   else {
      abbrev_buf[0] = 0;
   }

   // setting of tz
   const char *env_tz = getenv("TZ");
   if (env_tz == NULL) {
      env_tz = "(null)";
   }
   char tz[128];
   strcpy(tz, env_tz);
   for (char *p = tz; *p; p++) {
      if (*p == '/') {
         *p = '\n';
      }
   }

   char tzpbuf[strlen(tzProvider) + 2];
   tzpbuf[0] = '\v';
   strcpy(tzpbuf + 1, tzProvider);

   // get sizes of various pieces
   int wh0 = text_size(FONT_BOLD_LARGE, jd_buf, 1, 3);
   int h0 = wh0 & 0xFFFF;
   int w0 = wh0 >> 16;

   int wh1 = (offset == 0.0) ? 0 :
      text_size(FONT_BOLD_LARGE, offset_buf, 1, 3);
   int h1 = wh1 & 0xFFFF;
   int w1 = wh1 >> 16;

   int wh2 = text_size(FONT_BOLD_LARGE, abbrev_buf, 1, 3);
   int h2 = wh2 & 0xFFFF;
   int w2 = wh2 >> 16;

   char tzbuf[strlen(tz) + 2];
   tzbuf[0] = '\v';
   strcpy(tzbuf + 1, tz);

   int wh3 = text_size(FONT_BOLD_LARGER, tzbuf, 1, 3);
   int h3 = wh3 & 0xFFFF;
   int w3 = wh3 >> 16;

   int wh4 = text_size(FONT_BOLD_LARGE, tzpbuf, 1, 3);
   int h4 = wh4 & 0xFFFF;
   int w4 = wh4 >> 16;

   // now output the things...

   // date side
   im_text_canvas(canvas, FONT_BOLD_LARGE, canvas->w - 5 - w0 / 2,
         canvas->h - 5 - h0 / 2, COLOR_WHITE, COLOR_BLACK, jd_buf, 1, 3,
         NO_X_MOVE);
   if (offset != 0.0) {
      im_text_canvas(canvas, FONT_BOLD_LARGE, canvas->w - 5 - w1 / 2,
            canvas->h - 5 - h0 - h1 / 2, COLOR_YELLOW, COLOR_BLACK,
            offset_buf, 1, 3,
            NO_X_MOVE);
   }

   // timezone side
   im_text_canvas(canvas, FONT_BOLD_LARGE, 5 + w2 / 2,
         canvas->h - 5 - h2 / 2,
         COLOR_WHITE, COLOR_BLACK, abbrev_buf, 1, 3,
         NO_X_MOVE);
   im_text_canvas(canvas, FONT_BOLD_LARGER, 5 + w3 / 2,
         canvas->h - 5 - h2 - h3 / 2, COLOR_WHITE, COLOR_BLACK, tzbuf, 1,
         3,
         NO_X_MOVE);
   im_text_canvas(canvas, FONT_BOLD_LARGE, 5 + w4 / 2,
         canvas->h - 5 - h2 - h3 - h4 / 2, COLOR_WHITE, COLOR_BLACK,
         tzpbuf, 1, 3,
         NO_X_MOVE);
}

/// @brief Draw provider information
///
/// upper left
///
/// @param dtq The delayed text queue
/// @param canvas The Canvas to draw on
/// @param locprovider The current location provider
/// @return void
void do_provider_info(
      struct delayed_text_queue *dtq,
      Canvas * canvas,
      const char *locprovider) {

   int wh, w, h;

   char buffer[strlen(locprovider) + 2];
   buffer[0] = '\v';
   strcpy(buffer + 1, locprovider);

   wh = text_size(FONT_BOLD_LARGER, buffer, 1, 3);
   w = wh >> 16;
   h = wh & 0xFFFF;

   im_text_canvas(canvas, FONT_BOLD_LARGER, w / 2 + 20,
         h / 2 + 20, COLOR_WHITE, COLOR_BLACK, buffer, 1, 3,
         NO_X_MOVE);
}

void do_lunar_eclipse(Canvas *canvas, double jd, double now_angle) {
   double a = jd - 0.5;
   double b = jd + 0.5;
   double c;
   struct FD FD;

   do {
      c = (a + b) / 2.0;
      FD = ə67(c);

      if (FD.D < 180.0) {
         a = c;
      }
      else if (FD.D > 180.0) {
         b = c;
      }
   } while ((b - a) > 1.0/(2.0 * 24.0 * 60.0));

   double theta = ə73(c);
   if (theta < 180.0) {
      theta = 180.0 - theta;
   }
   else {
      theta = theta - 180.0;
   }

   if (theta > 12.25) {
      // lunar eclipse not possible!
      return;
   }

#if 0
   bool must = false;

   if (theta < 9.5) {
      must = true;
   }

   printf("lunar eclipse %s %f [%f %f %f]\n",
      must ? "must" : "might",
      theta, jd - 0.5, c, jd + 0.5);
#endif

   double radius = SIZE / 8 + SCALE(32);
   double angle =
      now_angle - 180.0 + 360.0 * (c - (jd - 0.5));

   double narrow_begin = angle - 27.5;
   double narrow_end = angle + 27.5;

   if (narrow_end > now_angle - 180.0 &&
       narrow_begin < now_angle + 180.0) {

      if (narrow_begin < now_angle - 180.0) {
         narrow_begin = now_angle - 180.0;
      }
      if (narrow_end > now_angle + 180.0) {
         narrow_end = now_angle + 180.0;
      }

      arc_canvas(canvas, SIZE / 2, SIZE / 2,
         radius, SCALE(6), COLOR_BLOOD, narrow_begin, narrow_end);

      double wide_begin = angle - 12.5;
      double wide_end = angle + 12.5;

      if (wide_end > now_angle - 180.0 &&
          wide_begin < now_angle + 180.0) {

         if (wide_begin < now_angle - 180.0) {
            wide_begin = now_angle - 180.0;
         }

         if (wide_end > now_angle + 180.0) {
            wide_end = now_angle + 180.0;
         }

         arc_canvas(canvas, SIZE / 2, SIZE / 2,
            radius, SCALE(12), COLOR_BLOOD, wide_begin, wide_end);
      }
   }

   double delta = now_angle - angle;
   ZRANGE(delta, 360.0);
   if (delta > 180.0) {
      delta = 360.0 - delta;
   }

   if (delta < 25.7) {
      FD = ə67(jd);

      double where_angle = FD.D - 90.0;
      int cx, cy;
      cx = canvas->w / 2 +
         (int)((canvas->w * 17.0 / 48.0) * cos_deg(where_angle));
      cy = canvas->h / 2 +
         (int)((canvas->h * 17.0 / 48.0) * sin_deg(where_angle));

      for (int dx = -SCALE(MOON_R); dx <= SCALE(MOON_R); dx++) {
         for (int dy = -SCALE(MOON_R); dy <= SCALE(MOON_R); dy++) {
            double d_interior = sqrt(dx * dx + dy * dy);
            if (d_interior <= SCALE(MOON_R)) {
               int nx = cx + dx;
               int ny = cy + dy;
               if (delta < 12.5 || (nx + ny) % 2 == 1) {
                  poke_canvas(canvas, nx, ny, COLOR_BLOOD);
               }
            }
         }
      }
   }
}

void do_solar_eclipse(struct delayed_text_queue *dtq, Canvas *canvas, double jd, double now_angle) {
   double a = jd - 0.5;
   double b = jd + 0.5;
   double c;
   struct FD FD;

   do {
      c = (a + b) / 2.0;
      FD = ə67(c);

      if (FD.D < 180.0) {
         b = c;
      }
      else if (FD.D > 180.0) {
         a = c;
      }
   } while ((b - a) > 1.0/(2.0 * 24.0 * 60.0));

   double theta = ə73(c);
   ZRANGE(theta, 360.0);
   if (theta > 180.0) {
      theta = 360.0 - theta;
   }

   if (theta > 18.5 && theta < (180.0 - 18.5)) {
      // solar eclipse not possible!
      return;
   }

   double radius = SIZE / 8 + SCALE(32);
   double angle =
      now_angle - 180.0 + 360.0 * (c - (jd - 0.5));

   arc_canvas(canvas, SIZE / 2, SIZE / 2,
      radius, SCALE(9), COLOR_WHITE, angle - 1.875, angle + 1.875);

   if (now_angle > angle - 1.875 && now_angle < angle + 1.875) {
      double where_angle = FD.D - 90.0;
      int cx, cy;
      cx = canvas->w / 2 +
         (int)((canvas->w * 17.0 / 48.0) * cos_deg(where_angle));
      cy = canvas->h / 2 +
         (int)((canvas->h * 17.0 / 48.0) * sin_deg(where_angle));

      arc_canvas(canvas, cx, cy, SCALE(MOON_R + 3), 7, COLOR_WHITE, 0, 360.0);

      for (int i = 0; i < 13; i++) {
         double theta = 360.0 * i / 13.0;
         double ix = cx + SCALE(MOON_R + 3 + 7) * cos_deg(theta);
         double iy = cy + SCALE(MOON_R + 3 + 7) * sin_deg(theta);
         double ox = cx + SCALE(MOON_R + 3 + 21) * cos_deg(theta);
         double oy = cy + SCALE(MOON_R + 3 + 21) * sin_deg(theta);
         thick_line_canvas(canvas, (int)ix, (int)iy, (int)ox, (int)oy, COLOR_WHITE, 3);
      }
   }
}

void do_eclipses(struct delayed_text_queue *dtq, Canvas *canvas, double jd, double now_angle) {
   double a = jd - 0.5;
   struct FD FDa = ə67(a);

   double b = jd + 0.5;
   struct FD FDb = ə67(b);

   if (FDa.D < 180.0 && FDb.D > 180.0) {
      do_lunar_eclipse(canvas, jd, now_angle);
   }
   else if (FDb.D < FDa.D) { // misordering indicates wraparound
      do_solar_eclipse(dtq, canvas, jd, now_angle);
   }
}

/// @brief Do all of the things
///
/// @param lat The observer's Latitude in degrees, South is negative
/// @param lon The observer's Longitude in degrees, West is negative
/// @param offset An offset from the current Julian Date, in days
/// @param width Width of canvas to draw
/// @param locprovider Name of the location provider to be displayed
/// @param tzprovider Name of the timezone provider to be displayed
/// @param tz Name of timezone to be used
/// @param lightdark controls which regions are considered light and dark
/// @param monam Array of month names for localization, Jan=0
/// @param wenam Array of weekday names for localization, Sun=0
/// @return A canvas that has been drawn upon
Canvas *do_clock(double lat,
                 double lon,
                 double offset,
                 int width,
                 const char *locprovider,
                 const char *tzprovider,
                 const char *tz,
                 int lightdark,
                 const char *monam[],
                 const char *wenam[]) {

   // clear locale, to use the system provided locale
   setlocale(LC_ALL, "");

   // set the timezone
   if (tz != NULL && tz[0] != 0) {
      setenv("TZ", tz, 1);
      tzset();
   }

   // assign global SIZE used for scaling
   SIZE = width;

   handle_font_sizes();

   struct delayed_text_queue *dtq = alloc_dtq(128);

   // observer's location
   struct φλ φλ = { lat, lon };

   time_t now = time(NULL) + offset * 24.0 * 60.0 * 60.0;

   if (offset > 19000000.0) { // it's a date!
      struct tm tm = { 0 };
      tm.tm_year = (int)(offset / 10000.0) - 1900;
      tm.tm_mon = ((int)(offset / 100.0) % 100) - 1;
      tm.tm_mday = ((int)(offset) % 100);
      now = mktime(&tm);

      //printf("%f %d %d %d %d %ld\n", offset,
      //   (int)offset, tm.tm_year, tm.tm_mon, tm.tm_mday, now);

      now += (offset - (double)((int)offset)) * 24.0 * 60.0 * 60.0;
   }

   // round to nearest minute
   now = (now + 30) / 60;
   now *= 60;

   double jd = time_t2julian(now);

   //// drawing begins here
   Canvas *canvas = new_canvas(width, width, COLOR_BLACK);

   if (!(lat > 90.0 || lon > 180.0 || lat < -90.0 || lon < -180.0)) {
      double now_angle =
         do_sun_bands(dtq, canvas, now, jd, φλ, lightdark);
      do_planet_bands(dtq, canvas, now_angle, jd, φλ);

      do_eclipses(dtq, canvas, jd, now_angle);
   }

   // information in the center
   do_center(dtq, canvas, now, φλ, monam, wenam);

   // lower right corner
   do_debug_info(dtq, canvas, now, jd, offset, tzprovider);

   // upper left corner
   do_provider_info(dtq, canvas, locprovider);

   // actually draw the text for realsies
   resolve_delayed_text(dtq, canvas);

   //do_moon_draw_debug(canvas, jd, true, φλ);

   return canvas;
}

int do_when_is_it(double lat, double lon, int category, int type, int delayMinutes) {
   // observer's location
   struct φλ φλ = { lat, lon };

   type -= 2; // why?  i have no idea.  tech debt i guess...

   time_t now = time(NULL);
   now /= 60;
   now *= 60;

   // we must conform to...
   //
   // val categorynames = arrayOf(
   //     "SOLAR", "CIVIL", "NAUTICAL", "ASTRONOMICAL",
   //     "LUNAR",                   // moon and planets from here on
   //     "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN",
   //     "ARIES")
   // val typenames = arrayOf("RISE","TRANSIT","SET")

   struct Aa Aa;
   double a[24*60];

   int max = -1;
   double HORIZON = 0.0;

   for (int i = 0; i < 24 * 60; i++) {
      time_t when = now + i * 60;
      double jd = time_t2julian(when);
      switch (category) {
         case 0:
         case 1:
         case 2:
         case 3:
            // solar
            Aa = ə25(jd, φλ, ə27(jd, ə46(jd)));
            break;
         case 4:
            // lunar
            Aa = ə25(jd, φλ, ə27(jd, ə65(jd)));
            break;
         case 5:
         case 6:
            // mercury, venus (inner planets)
            Aa = ə25(jd, φλ, ə54(jd, category - 5));
            break;
         case 7:
         case 8:
         case 9:
            // mars, jupiter, saturn (outer planets)
            Aa = ə25(jd, φλ, ə54(jd, category - 5));
            break;
         case 10:
            // aries
            Aa = ə25(jd, φλ, (struct αδ) { 0.0, 0.0 });
            break;
      }

      a[i] = Aa.a;
      if (max == -1 || a[i] > a[max]) {
         max = i;
      }
   }

   if (type == 1) {
      return 60 * max + delayMinutes * 60;
   }

   switch (category) {
      case 0:
         HORIZON = -1.0;
         break;
      case 1:
         HORIZON = -6.56;
         break;
      case 2:
         HORIZON = -12.56;
         break;
      case 3:
         HORIZON = -18.56;
         break;
      default:
         HORIZON = 0.0;
   }

   for (int i = 1; i < 24 * 60; i++) {
      if (type == 0) {
         if (a[i - 1] < HORIZON && a[i] >= HORIZON) {
            return 60 * i + delayMinutes * 60;
         }
      }
      else if (type == 2) {
         if (a[i - 1] >= HORIZON && a[i] < HORIZON) {
            return 60 * i + delayMinutes * 60;
         }
      }
   }

   // hrm, not found...
   return -1;
}

// vim: expandtab:noai:ts=3:sw=3
