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

//#pragma GCC optimize("Ofast")

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

#include <assert.h>

#include "trig1.h"
#include "astro.h"
#include "draw.h"

#define FNORD KNARF
#define sin(x) FNORD
#define cos(x) FNORD
#define tan(x) FNORD

#define ORIG

#ifdef ORIG
//#define SIZE 1024
int SIZE = 1024;
uint8_t *ASTRO_FONT;
uint8_t *FONT_BOLD_BIG;
uint8_t *FONT_BOLD_MED;
uint8_t *FONT_BOLD_SMALL;
uint8_t *FONT_ITALIC_MED;
#endif

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

static void set_astro_font(int width) {
   uint8_t *choices[] = {
      astro_16_bdf,
      astro_20_bdf,
      astro_24_bdf,
      astro_32_bdf,
      astro_50_bdf,
      NULL
   };
   set_font(&ASTRO_FONT, choices, astro_32_bdf[1], width);
}

static void set_bold_big(int width) {
   uint8_t *choices[] = {
      djsmb_8_bdf,
      djsmb_10_bdf,
      djsmb_16_bdf,
      djsmb_20_bdf,
      djsmb_32_bdf,
      djsmb_40_bdf,
      djsmb_50_bdf,
      djsmb_60_bdf,
      NULL
   };
   set_font(&FONT_BOLD_BIG, choices, djsmb_50_bdf[1], width);
}

static void set_bold_med(int width) {
   uint8_t *choices[] = {
      djsmb_8_bdf,
      djsmb_10_bdf,
      djsmb_12_bdf,
      djsmb_14_bdf,
      djsmb_16_bdf,
      djsmb_18_bdf,
      djsmb_20_bdf,
      djsmb_22_bdf,
      djsmb_24_bdf,
      djsmb_32_bdf,
      djsmb_40_bdf,
      djsmb_50_bdf,
      djsmb_60_bdf,
      NULL
   };
   set_font(&FONT_BOLD_MED, choices, djsmb_22_bdf[1], width);
}

static void set_bold_small(int width) {
   uint8_t *choices[] = {
      djsmb_8_bdf,
      djsmb_10_bdf,
      djsmb_12_bdf,
      djsmb_14_bdf,
      djsmb_16_bdf,
      djsmb_18_bdf,
      djsmb_20_bdf,
      djsmb_22_bdf,
      djsmb_24_bdf,
      djsmb_32_bdf,
      djsmb_40_bdf,
      djsmb_50_bdf,
      djsmb_60_bdf,
      NULL
   };
   set_font(&FONT_BOLD_SMALL, choices, djsmb_18_bdf[1], width);
}

static void set_italic_med(int width) {
   uint8_t *choices[] = {
      djsmo_8_bdf,
      djsmo_10_bdf,
      djsmo_12_bdf,
      djsmo_14_bdf,
      djsmo_16_bdf,
      djsmo_20_bdf,
      djsmo_22_bdf,
      djsmo_24_bdf,
      djsmo_32_bdf,
      djsmo_40_bdf,
      djsmo_50_bdf,
      djsmo_60_bdf,
      NULL
   };
   set_font(&FONT_ITALIC_MED, choices, djsmo_22_bdf[1], width);
}

/// @brief Draw the "now" hand
///
/// The now hand is drawn using xor logic
///
/// @param canvas The Canvas to draw on
/// @param now_angle The angle which is now
/// @return void
void do_now_hand(Canvas * canvas, double now_angle) {
   double xc =
      canvas->w / 2.0 + (canvas->w / 2.0 / 2.0 +
            SCALE(128)) * cos_deg(now_angle);
   double yc =
      canvas->h / 2.0 + (canvas->h / 2.0 / 2.0 +
            SCALE(128)) * sin_deg(now_angle);
   double xc2 =
      canvas->w / 2.0 + (canvas->w / 2.0 / 2.0 -
            SCALE(128)) * cos_deg(now_angle);
   double yc2 =
      canvas->h / 2.0 + (canvas->h / 2.0 / 2.0 -
            SCALE(128)) * sin_deg(now_angle);

   thick_line_canvas(canvas, (int)xc2, (int)yc2, (int)xc, (int)yc, COLOR_RED, 3);
}

/// @brief Draw ticks every hour around the outside edge
///
/// Ticks are drawn using xor logic, so they should always be visible
///
/// @param canvas The Canvas to draw on
/// @param now The current time_t
/// @param r The radius of the clock
/// @param now_angle The angle used as "now"
/// @return void
void do_hour_ticks(Canvas * canvas, time_t now, int r, double now_angle) {
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
      xa = x + (r - SCALE(8)) * cos_deg(angle);
      ya = y + (r - SCALE(8)) * sin_deg(angle);
      xc = x + r * cos_deg(angle);
      yc = y + r * sin_deg(angle);

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

void duration_text(Canvas *canvas,
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

   text_canvas(canvas, FONT_BOLD_MED, SIZE / 2, y,
      color, COLOR_NONE, buf, 1, 3);
}

void duration_top_text(Canvas *canvas,
                       const char *text,
                       double degrees,
                       unsigned int color) {
   duration_text(canvas, text, degrees, color, SIZE / 4);
}

void duration_bottom_text(Canvas *canvas,
                          const char *text,
                          double degrees,
                          unsigned int color) {
   duration_text(canvas, text, degrees, color, SIZE * 3 / 4);
}

void edgetime(Canvas *canvas,
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
      r = (SIZE/2) * 5 / 8;
   }
   else {
      r = (SIZE/2) * 9 / 16;
   }

   int x = cx + r * cos_deg(theta);
   int y = cy + r * sin_deg(theta);

   text_canvas(canvas,
               index < (12 * 60) ? FONT_ITALIC_MED : FONT_BOLD_MED,
               x, y,
               color, COLOR_NONE, buf, 1, 3);
}

double do_sun_bands(Canvas *canvas,
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

   double a[24*60];
   int max = -1;
   for (int i = 0; i < 24 * 60; i++) {
      double when = jd - 0.5 + (double) i / (24.0* 60.0);
      struct Aa Aa = ə25(when, φλ, ə27(when, ə46(when)));
      a[i] = Aa.a;
      if (max == -1 || Aa.a > a[max]) {
         max = i;
      }
   }

   double up_angle = -180.0 + (360.0 * (double) max / (24.0 * 60.0));
   up_angle = 270.0 - up_angle;
   ZRANGE(up_angle, 360.0);

   double now_angle = up_angle - (12*60-max)/(24.0 * 60.0);
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

   double start_angle = up_angle - 180.0;

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
            up_angle - 180.0 + 360.0 * ((double) i / (24.0 * 60.0));

         arc_canvas(canvas,
            SIZE / 2, SIZE / 2, SIZE / 2 / 2, SIZE / 2 / 2,
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
               SIZE / 2, SIZE / 2, SIZE / 2 / 2 + SCALE(126), 7,
               oldbandcolor, start_angle, stop_angle);
         arc_canvas(canvas,
               SIZE / 2, SIZE / 2, SIZE / 2 / 2 - SCALE(128), 7,
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
      up_angle + 180.0;

   arc_canvas(canvas,
      SIZE / 2, SIZE / 2, SIZE / 2 / 2, SIZE / 2 / 2,
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
         SIZE / 2, SIZE / 2, SIZE / 2 / 2 + SCALE(126), 7,
         oldbandcolor, start_angle, stop_angle);
   arc_canvas(canvas,
         SIZE / 2, SIZE / 2, SIZE / 2 / 2 - SCALE(128), 7,
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

   do_hour_ticks(canvas, now, SIZE / 2 / 2 + SCALE(128), now_angle);

   if (lightdark == 0) {
      bool skiplower = false;

      // what to put at the top?
      if (sunup > 0.0) {
         duration_top_text(canvas, "sun up", sunup, COLOR_BLACK);
         if (civil == 0.0) {
            skiplower = true;
         }
      }
      else if (civil > 0.0) {
         duration_top_text(canvas, "civil", civil, COLOR_BLACK);
         if (nautical == 0.0) {
            skiplower = true;
         }
      }
      else if (nautical > 0.0) {
         duration_top_text(canvas, "nautical", nautical, COLOR_WHITE);
         if (astronomical == 0.0) {
            skiplower = true;
         }
      }
      else if (astronomical > 0.0) {
         duration_top_text(canvas, "astronomical", astronomical, COLOR_WHITE);
         if (night == 0.0) {
            skiplower = true;
         }
      }
      else if (night > 0.0) {
         duration_top_text(canvas, "night", night, COLOR_WHITE);
         skiplower = true;
      }

      // what to put at the bottom
      if (!skiplower) {
         if (night > 0.0) {
            duration_bottom_text(canvas, "night", night, COLOR_WHITE);
         }
         else if (astronomical > 0.0) {
            duration_bottom_text(canvas, "astronomical", astronomical, COLOR_WHITE);
         }
         else if (nautical > 0.0) {
            duration_bottom_text(canvas, "nautical", nautical, COLOR_WHITE);
         }
         else if (civil > 0.0) {
            duration_bottom_text(canvas, "civil", civil, COLOR_BLACK);
         }
         else if (sunup > 0.0) {
            // what madness is this ?!?!?
            duration_bottom_text(canvas, "MADNESS", sunup, COLOR_BLACK);
         }
      }
   }
   else {
      // what to put at the top?
      double twilight = 360.0 - light - dark;

      if (light > 0.0) {
         duration_top_text(canvas, "light", light, COLOR_BLACK);
      }
      else if (twilight > 0.0) {
         duration_top_text(canvas, "twilight", twilight, COLOR_BLACK);
      }

      if (dark > 0.0) {
         duration_bottom_text(canvas, "dark", dark, COLOR_WHITE);
      }
      else if (twilight > 0.0 && light > 0.0) {
         duration_bottom_text(canvas, "twilight", twilight, COLOR_WHITE);
      }
   }

   for (int i = 1; i < 24*60; i++) {
      if (a[i-1] < HORIZON_SUN && a[i] >= HORIZON_SUN) {
         edgetime(canvas, now, i, now_angle, COLOR_BLACK);
      }
      if (a[i-1] >= HORIZON_SUN && a[i] < HORIZON_SUN) {
         edgetime(canvas, now, i, now_angle, COLOR_BLACK);
      }
      if (a[i-1] < HORIZON_CIVIL && a[i] >= HORIZON_CIVIL) {
         edgetime(canvas, now, i, now_angle, COLOR_BLACK);
      }
      if (a[i-1] >= HORIZON_CIVIL && a[i] < HORIZON_CIVIL) {
         edgetime(canvas, now, i, now_angle, COLOR_BLACK);
      }
      if (a[i-1] < HORIZON_NAUTICAL && a[i] >= HORIZON_NAUTICAL) {
         edgetime(canvas, now, i, now_angle, COLOR_WHITE);
      }
      if (a[i-1] >= HORIZON_NAUTICAL && a[i] < HORIZON_NAUTICAL) {
         edgetime(canvas, now, i, now_angle, COLOR_WHITE);
      }
      if (a[i-1] < HORIZON_ASTRONOMICAL && a[i] >= HORIZON_ASTRONOMICAL) {
         edgetime(canvas, now, i, now_angle, COLOR_WHITE);
      }
      if (a[i-1] >= HORIZON_ASTRONOMICAL && a[i] < HORIZON_ASTRONOMICAL) {
         edgetime(canvas, now, i, now_angle, COLOR_WHITE);
      }
   }

   edgetime(canvas, now, max, now_angle, COLOR_BLACK);

   return now_angle;
}

/// @brief Draw the moon
///
/// @param canvas The Canvas to draw on
/// @param jd The current Julian Date
/// @param is_up Whether or not the moon is currently up
/// @return void
void
do_moon_draw(Canvas * canvas, double jd, int is_up) {
   struct FD FD = ə67(jd);

   // WHERE to draw it.
   double where_angle = FD.D - 90.0;
   int cx, cy;
   cx = canvas->w / 2 +
      (int)((10.0 + canvas->w / 6.0) * cos_deg(where_angle));
   cy = canvas->h / 2 +
      (int)((10.0 + canvas->h / 6.0) * sin_deg(where_angle));

   // a little circle (interior)
   // with a chunk cut out of it.

   // the chunk is a circle, centered to the right or to the left.
   unsigned int interior_color;
   unsigned int chunk_color;

   // from wolfram alpha
   // a circle passing through points (0,a), (0,-a), and (b,0)
   double b;

   if (FD.D < 90.0) {
      interior_color = COLOR_SILVER;
      chunk_color = COLOR_BLACK;
      // b = +a at 0 degrees, and 0 at 90 degrees
      b = (90.0 - FD.D) / 90.0;
   }
   else if (FD.D < 180.0) {
      interior_color = COLOR_BLACK;
      chunk_color = COLOR_SILVER;
      // b = 0 at 90 degrees and -a at 180 degrees
      b = -(FD.D - 90.0) / 90.0;
   }
   else if (FD.D < 270.0) {
      interior_color = COLOR_BLACK;
      chunk_color = COLOR_SILVER;
      // b = +a at 180 degrees and 0 at 270 degrees
      b = (270.0 - FD.D) / 90.0;
   }
   else {
      interior_color = COLOR_SILVER;
      chunk_color = COLOR_BLACK;
      // b = 0 at 270 degrees and -a at 360 degrees
      b = -(FD.D - 270.0) / 90.0;
   }

   // avoid possible division by zero
   if (b == 0.0) {
      b = 1.0;
   }
   b *= (double) SCALE(40);

   int chunk_x = (b * b - (double)SCALE(40) * (double)SCALE(40)) / (2.0 * b);
   int chunk_r = abs(chunk_x - b);

   // this won't make sense, because it's derived from earlier code...
   // but it really does draw the moon...
   for (int dx = -SCALE(40); dx <= SCALE(40); dx++) {
      for (int dy = -SCALE(40); dy <= SCALE(40); dy++) {
         double d_interior = sqrt(dx * dx + dy * dy);
         if (d_interior <= SCALE(40.0)) {
            double d_chunk = sqrt((dx - chunk_x) * (dx - chunk_x) + dy * dy);
            if (d_chunk < chunk_r) {
               poke_canvas(canvas, cx + dx, cy + dy, chunk_color);
            }
            else {
               poke_canvas(canvas, cx + dx, cy + dy, interior_color);
            }
         }
      }
   }

   arc_canvas(canvas, cx, cy, SCALE(43), 7,
      is_up ? COLOR_WHITE : COLOR_BLACK, 0, 360.0);
}

void do_planet_bands(Canvas *canvas,
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

   double radius = SIZE / 2 / 2 + SCALE(128 + 16 + 5);

   struct symxy {
      int x;
      int y;
      int p;
   } symxy[32];

   int symxy_spot = 0;

   for (int p = 0; p < 7; p++) {
      int ticked = 0;
      double a[24*60];
      int max = -1;
      for (int i = 0; i < 24 * 60; i++) {
         double when = jd - 0.5 + (double) i / (24.0* 60.0);
         struct Aa Aa;

         if (p == 0) Aa = ə25(when, φλ, ə27(when, ə65(when)));
         else if (p < 3) Aa = ə25(when, φλ, ə54(when, p - 1));
         else if (p < 6) Aa = ə25(when, φλ, ə54(when, p));
         else Aa = ə25(when, φλ, (struct αδ) { 0.0, 0.0 });

         a[i] = Aa.a;
         if (max == -1 || Aa.a > a[max]) {
            max = i;
         }
      }

      unsigned int color = COLOR_NONE;

      if (a[0] > HORIZON) {
         color = colors[p];
      }
      unsigned int oldcolor = color;

      double start_angle = now_angle - 180.0;

      for (int i = 1; i < 24 * 60; i++) {
         if (a[i-1] < HORIZON && a[i] >= HORIZON) {
            color = colors[p];
         }
         if (a[i-1] >= HORIZON && a[i] < HORIZON) {
            color = COLOR_NONE;
         }

         if (color != oldcolor) {
            double stop_angle =
               now_angle - 180.0 + 360.0 * ((double) i / (24.0 * 60.0));

            arc_canvas(canvas, SIZE / 2, SIZE / 2,
               radius, SCALE(5), oldcolor, start_angle, stop_angle);

            {
               double offset =
                  (oldcolor == COLOR_NONE) ? -3.0 : 3.0;

               if (symxy_spot < (sizeof(symxy)/sizeof(symxy[0]))) {
                  symxy[symxy_spot].p = p;
                  symxy[symxy_spot].x =
                     (SIZE / 2) + radius * cos_deg(stop_angle + offset);
                  symxy[symxy_spot].y =
                     (SIZE / 2) + radius * sin_deg(stop_angle + offset);
                  symxy_spot++;
               }

               ticked++;
            }

            oldcolor = color;
            start_angle = stop_angle;

            double x1, y1, x2, y2;

            x1 = canvas->w / 2 + (radius - 16) * cos_deg(stop_angle);
            y1 = canvas->h / 2 + (radius - 16) * sin_deg(stop_angle);
            x2 = canvas->w / 2 + (radius + 16) * cos_deg(stop_angle);
            y2 = canvas->h / 2 + (radius + 16) * sin_deg(stop_angle);

            line_canvas(canvas, x1, y1, x2, y2, colors[p]);
         }
      }

      double stop_angle = now_angle + 180.0;

      arc_canvas(canvas, SIZE / 2, SIZE / 2,
         radius, SCALE(5), oldcolor, start_angle, stop_angle);

      if (a[max] > HORIZON) {
         double transit_angle =
            now_angle - 180.0 + 360.0 * ((double) max / (24.0 * 60.0));

         double x1, y1, x2, y2;

         x1 = canvas->w / 2 + (radius - 16) * cos_deg(transit_angle);
         y1 = canvas->h / 2 + (radius - 16) * sin_deg(transit_angle);
         x2 = canvas->w / 2 + (radius + 16) * cos_deg(transit_angle);
         y2 = canvas->h / 2 + (radius + 16) * sin_deg(transit_angle);

         line_canvas(canvas, x1, y1, x2, y2, colors[p]);

         if (!ticked) {
            symxy[symxy_spot].p = p;
            symxy[symxy_spot].x =
               (SIZE / 2) + (radius + SCALE(20)) * cos_deg(transit_angle + 3.0);
            symxy[symxy_spot].y =
               (SIZE / 2) + (radius + SCALE(20)) * sin_deg(transit_angle + 3.0);
            symxy_spot++;
         }
      }

      radius += SCALE(15);
      max = -1;

      if (p == 0) {
         do_moon_draw(canvas, jd, a[12*60] > HORIZON);
      }
   }

   for (int i = 0; i < symxy_spot; i++) {
      char sym[2] = { syms[symxy[i].p], 0 };
      text_canvas(canvas, ASTRO_FONT,
         symxy[i].x,
         symxy[i].y,
         colors[symxy[i].p],
            COLOR_BLACK, sym, 1, 1);
   }
}

/// @brief Draw the location in the center of the Canvas
///
/// @param canvas The Canvas to draw on
/// @param now The current time_t
/// @param φλ The lat/long of the observer position
/// @param monam Array of month names for localization, Jan=0
/// @param wenam Array of weekday names for localization, Sun=0
/// @return void
void do_center(Canvas * canvas, time_t now, struct φλ φλ,
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
         int mid = canvas->w / 2;
         arc_canvas(canvas,
               mid, mid, mid / 2 - SCALE(126), 7, COLOR_WHITE, 0, 360.0);
         arc_canvas(canvas,
               mid, mid, mid / 2 + SCALE(126), 7, COLOR_WHITE, 0, 360.0);

      }
      else {
         sprintf(location, "%0.2f%s%c,%0.2f%s%c", lat, degree, NS, lon, degree,
               EW);
      }
      text_canvas(canvas, FONT_BOLD_SMALL, canvas->w / 2,
            canvas->h / 2 + (int)SCALE(48), COLOR_WHITE, COLOR_BLACK,
            location, 1, 2);
   }

   struct tm local = *localtime(&now);

   {
      char time[32];

      sprintf(time, "%04d-%02d-%02d",
            local.tm_year + 1900,
            local.tm_mon + 1,
            local.tm_mday);

      text_canvas(canvas, FONT_BOLD_SMALL, canvas->w / 2,
            canvas->h / 2 + (int)SCALE(72), COLOR_WHITE, COLOR_BLACK, time,
            1, 2);
   }

   {
      char time[32];

      sprintf(time, "%02d:%02d", local.tm_hour, local.tm_min);

      text_canvas(canvas, FONT_BOLD_BIG, canvas->w / 2, canvas->h / 2,
            COLOR_WHITE, COLOR_BLACK, time, 1, 12);
   }

   {
      char text[64];
      if (!wenam) {
         strftime(text, sizeof(text), "%A", &local);
      }
      else {
         sprintf(text, "%s", wenam[local.tm_wday]);
      }
      text_canvas(canvas, FONT_BOLD_MED, canvas->w / 2,
            canvas->h / 2 - (int)SCALE(90), COLOR_WHITE, COLOR_BLACK, text,
            1, 2);
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
      text_canvas(canvas, FONT_BOLD_MED, canvas->w / 2,
            canvas->h / 2 - (int)SCALE(60), COLOR_WHITE, COLOR_BLACK, text,
            1, 2);
   }
}

/// @brief Draw debugging information
///
/// Timezone in lower left, Julian Date in lower right
///
/// @param canvas The Canvas to draw on
/// @param now The current time_t
/// @param JD The current Julian Date
/// @param offset The offset passed into do_all
/// @param tzProvider Name of the timezone provider
/// @return void
void do_debug_info(Canvas * canvas,
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
      sprintf(abbrev_buf, "%s%s%s/%s%s%s",
            tm->tm_isdst ? "" : "[",
            tzname[0],
            tm->tm_isdst ? "" : "]",
            tm->tm_isdst ? "[" : "", tzname[1], tm->tm_isdst ? "]" : "");
   }
   else if (tzname[0] != NULL) {
      sprintf(abbrev_buf, "[%s]", tzname[0]);
   }
   else {
      abbrev_buf[0] = 0;
   }

   // setting of tz
   const char *tz = getenv("TZ");
   if (tz == NULL) {
      tz = "(null)";
   }

   // no buffer needed for tzProvider

   // probe our font
#define TEST_STRING "A_gy"      // includes upercase and descenders
   int whn = text_canvas(canvas, FONT_BOLD_MED, -1000, -1000,
         COLOR_WHITE, COLOR_BLACK, TEST_STRING, 1, 3);
   int hn = whn & 0xFFFF;

   // get sizes of various pieces
   int wh0 = text_canvas(canvas, FONT_BOLD_MED, -1000, -1000,
         COLOR_WHITE, COLOR_BLACK, jd_buf, 1, 3);
   int w0 = wh0 >> 16;

   int wh1 = text_canvas(canvas, FONT_BOLD_MED, -1000, -1000,
         COLOR_WHITE, COLOR_BLACK, offset_buf, 1, 3);
   int w1 = wh1 >> 16;

   int wh2 = text_canvas(canvas, FONT_BOLD_MED, -1000, -1000,
         COLOR_WHITE, COLOR_BLACK, abbrev_buf, 1, 3);
   int w2 = wh2 >> 16;

   int wh3 = text_canvas(canvas, FONT_BOLD_MED, -1000, -1000,
         COLOR_WHITE, COLOR_BLACK, tz, 1, 3);
   int w3 = wh3 >> 16;

   int wh4 = text_canvas(canvas, FONT_BOLD_MED, -1000, -1000,
         COLOR_WHITE, COLOR_BLACK, tzProvider, 1, 3);
   int w4 = wh4 >> 16;

   // find highest height
   int h = hn;

   // now output the things...

   // date side
   text_canvas(canvas, FONT_BOLD_MED, canvas->w - 5 - w0 / 2,
         canvas->h - 5 - h / 2, COLOR_WHITE, COLOR_BLACK, jd_buf, 1, 3);
   text_canvas(canvas, FONT_BOLD_MED, canvas->w - 5 - w1 / 2,
         canvas->h - 5 - (h + 5) - h / 2, COLOR_YELLOW, COLOR_BLACK,
         offset_buf, 1, 3);

   // timezone side
   text_canvas(canvas, FONT_BOLD_MED, 5 + w2 / 2, canvas->h - 5 - h / 2,
         COLOR_WHITE, COLOR_BLACK, abbrev_buf, 1, 3);
   text_canvas(canvas, FONT_BOLD_MED, 5 + w3 / 2,
         canvas->h - 5 - (h + 5) - h / 2, COLOR_WHITE, COLOR_BLACK, tz, 1,
         3);
   text_canvas(canvas, FONT_BOLD_MED, 5 + w4 / 2,
         canvas->h - 5 - 2 * (h + 5) - h / 2, COLOR_WHITE, COLOR_BLACK,
         tzProvider, 1, 3);
}

/// @brief Draw provider information
///
/// upper left
///
/// @param canvas The Canvas to draw on
/// @param locprovider The current location provider
/// @return void
void do_provider_info(Canvas * canvas, const char *locprovider) {
   int wh = text_canvas(canvas, FONT_BOLD_MED, -1000, -1000,
         COLOR_WHITE, COLOR_BLACK, locprovider, 1, 3);
   int w = wh >> 16;
   int h = wh & 0xFFFF;
   text_canvas(canvas, FONT_BOLD_MED, w / 2 + 20,
         h / 2 + 20, COLOR_WHITE, COLOR_BLACK, locprovider, 1, 3);
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

   double radius = SIZE / 2 / 2 + SCALE(128 + 16 + 5);
   double angle =
      now_angle - 180.0 + 360.0 * (c - (jd - 0.5));

   arc_canvas(canvas, SIZE / 2, SIZE / 2,
      radius, SCALE(5), COLOR_BLOOD, angle - 12.5, angle + 12.5);
   arc_canvas(canvas, SIZE / 2, SIZE / 2,
      radius, SCALE(3), COLOR_BLOOD, angle - 27.5, angle + 27.5);

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
         (int)((10.0 + canvas->w / 6.0) * cos_deg(where_angle));
      cy = canvas->h / 2 +
         (int)((10.0 + canvas->h / 6.0) * sin_deg(where_angle));

      for (int dx = -SCALE(40); dx <= SCALE(40); dx++) {
         for (int dy = -SCALE(40); dy <= SCALE(40); dy++) {
            double d_interior = sqrt(dx * dx + dy * dy);
            if (d_interior <= SCALE(40.0)) {
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

void do_solar_eclipse(Canvas *canvas, double jd, double now_angle) {
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

   if (theta > 18.5) {
      // solar eclipse not possible!
      return;
   }

   double angle =
      now_angle - 180.0 + 360.0 * (c - (jd - 0.5));

   // border
   arc_canvas(canvas,
         SIZE / 2, SIZE / 2, SIZE / 2 / 2, SCALE(180),
         COLOR_XOR, angle - 1.875, angle + 1.875);
}

void do_eclipses(Canvas *canvas, double jd, double now_angle) {
   double a = jd - 0.5;
   struct FD FDa = ə67(a);

   double b = jd + 0.5;
   struct FD FDb = ə67(b);

   if (FDa.D < 180.0 && FDb.D > 180.0) {
      do_lunar_eclipse(canvas, jd, now_angle);
   }
   else if (FDb.D < FDa.D) {
      do_solar_eclipse(canvas, jd, now_angle);
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
Canvas *do_all(double lat,
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

   double jd; // when is NOW?

   // assign global SIZE used for scaling
   SIZE = width;

   set_astro_font(width);
   set_bold_big(width);
   set_bold_med(width);
   set_bold_small(width);
   set_italic_med(width);

   // observer's location
   struct φλ φλ = { lat, lon };

   time_t now = time(NULL) + offset * 24.0 * 60.0 * 60.0;
   now /= 60;
   now *= 60;

   jd = time_t2julian(now);

   //// drawing begins here
   Canvas *canvas = new_canvas(width, width, COLOR_BLACK);

   if (!(lat > 90.0 || lon > 180.0 || lat < -90.0 || lon < -180.0)) {
      double now_angle =
         do_sun_bands(canvas, now, jd, φλ, lightdark);
      do_planet_bands(canvas, now_angle, jd, φλ);

      do_eclipses(canvas, jd, now_angle);
   }

   // information in the center
   do_center(canvas, now, φλ, monam, wenam);

   // lower right corner
   do_debug_info(canvas, now, jd, offset, tzprovider);

   // upper left corner
   do_provider_info(canvas, locprovider);

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
