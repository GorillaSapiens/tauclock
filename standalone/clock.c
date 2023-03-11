#pragma GCC optimize("Ofast")
//  Sunclock, draw a clock with local solar and lunar information
//  Copyright (C) 2022 Adam Wozniak / GorillaSapiens
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <math.h>

#include <assert.h>

#ifdef STANDALONE

#include <libnova/solar.h>
#include <libnova/lunar.h>
#include <libnova/mercury.h>
#include <libnova/venus.h>
#include <libnova/mars.h>
#include <libnova/jupiter.h>
#include <libnova/saturn.h>
#include <libnova/julian_day.h>
#include <libnova/rise_set.h>
#include <libnova/transform.h>

#else

#include "solar.h"
#include "lunar.h"
#include "mercury.h"
#include "venus.h"
#include "mars.h"
#include "jupiter.h"
#include "saturn.h"
#include "julian_day.h"
#include "rise_set.h"
#include "transform.h"

#endif

#include "draw.h"

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

/// @brief A macro to convert Degrees to Radians
#define DEG2RAD(x) ((double)(x) * M_PI / 180.0)

// /// @brief A macro to convert Radians to Degrees
// #define RAD2DEG(x) ((x) * 180.0 / M_PI)

/// @brief An array containing month names.
const char *months[] = {
   "January",
   "February",
   "March",
   "April",
   "May",
   "June",
   "July",
   "August",
   "September",
   "October",
   "November",
   "December"
};

/// @brief An array containing weekday names.
const char *weekdays[] = {
   "Sunday",
   "Monday",
   "Tuesday",
   "Wednesday",
   "Thursday",
   "Friday",
   "Saturday"
};

/// @brief enumeration type for events
///
/// Order is important, as it is used when sorting
typedef enum EventType {
   EVENT_UP,
   EVENT_DOWN,
   EVENT_RISE,
   EVENT_TRANSIT,
   EVENT_SET
} EventType;

/// @brief enumeration type for event categories
typedef enum EventCategory {
   CAT_SOLAR,
   CAT_CIVIL,
   CAT_NAUTICAL,
   CAT_ASTRONOMICAL,
   CAT_LUNAR,                   // moon and planets from here on
   CAT_MERCURY,
   CAT_VENUS,
   CAT_MARS,
   CAT_JUPITER,
   CAT_SATURN,
   CAT_ARIES                   // the first point of aries
} EventCategory;

/// @brief Human readable names for the CAT_* defines
const char *categorynames[] = { "sun", "nautical", "civil", "astronomical",
   "lunar", "mercury", "venus", "mars", "jupiter", "saturn", "aries"
};

#define COLOR_DAYLIGHT        COLOR_YELLOW
#define COLOR_CIVIL           COLOR_ORANGE
#define COLOR_NAUTICAL        COLOR_LIGHTBLUE
#define COLOR_ASTRONOMICAL    COLOR_BLUE
#define COLOR_DARKNESS        COLOR_DARKBLUE

#define COLOR_LIGHT           COLOR_WHITE
#define COLOR_TWILIGHT        COLOR_LIGHTGRAY
#define COLOR_DARK            COLOR_DARKGRAY

#define COLOR_MERCURY         COLOR_GRAY
#define COLOR_VENUS           COLOR_WHITE
#define COLOR_MARS            COLOR_RED
#define COLOR_JUPITER         COLOR_ORANGE
#define COLOR_SATURN          COLOR_LIGHTBLUE
#define COLOR_ARIES           COLOR_GREEN

/// @brief Human readable names for the EVENT_* defines
const char *typenames[] = { "up", "down", "rise", "transit", "set" };

/// @brief A structure denoting a up/down/rise/transit/set event.
typedef struct {
   double jd;
   EventCategory category;
   EventType type;
   int prune;
} Event;

/// @brief The size of the events array
#define NUM_EVENTS 4096

/// @brief JD bounds on cache
#define CACHE_LIMIT 1.0

struct CacheNode {
   int critical;
   double JD;
   struct ln_equ_posn posn; // body position
   struct ln_hrz_posn hrz_posn; // observed position
   struct CacheNode *prev;
   struct CacheNode *next;
};

struct Cache {
   struct ln_lnlat_posn observer;
   struct CacheNode *head;
};

/// @brief A struct used to remember where something is drawn.
typedef struct TimeDrawnMemory {
   double now;
   double jd;
   int x;
   int y;
   int w;
   int h;
   unsigned int fg;
   unsigned int bg;
} TimeDrawnMemory;

#define NUM_TIMEDRAWN 32

/// @brief A struct used to remember where something is drawn.
typedef struct AccumDrawnMemory {
   int x;
   int y;
   unsigned int fg;
   unsigned int bg;
   char *str;
} AccumDrawnMemory;

#define NUM_ACCUM 8

typedef struct {
   /// @brief An array containing events
   Event events[NUM_EVENTS];

   /// @brief An integer denoting the next free slot in the events array
   int event_spot;

   /// @brief An array of things drawn
   TimeDrawnMemory timedrawnmemory[NUM_TIMEDRAWN];

   /// @brief The next open spot in the array
   int timedrawnspot;

   /// @brief An array of things drawn
   AccumDrawnMemory accumdrawnmemory[NUM_ACCUM];

/// @brief The next open spot in the array
   int accumdrawnspot;

   struct Cache *sun_cache;
   struct Cache *moon_cache;
   struct Cache *mercury_cache;
   struct Cache *venus_cache;
   struct Cache *mars_cache;
   struct Cache *jupiter_cache;
   struct Cache *saturn_cache;
   struct Cache *aries_cache;
} Context;

#define events (context->events)
#define event_spot (context->event_spot)
#define timedrawnmemory (context->timedrawnmemory)
#define timedrawnspot (context->timedrawnspot)
#define accumdrawnmemory (context->accumdrawnmemory)
#define accumdrawnspot (context->accumdrawnspot)
#define sun_cache (context->sun_cache)
#define moon_cache (context->moon_cache)
#define mercury_cache (context->mercury_cache)
#define venus_cache (context->venus_cache)
#define mars_cache (context->mars_cache)
#define jupiter_cache (context->jupiter_cache)
#define saturn_cache (context->saturn_cache)
#define aries_cache (context->aries_cache)

void init_context(Context *context) {
   memset(context,0, sizeof(Context));
}

/// @brief Get the true local date
///
/// The libnova ln_get_local_date is buggy in that it gives a time
/// using the now current time zone, and NOT the time zone current
/// at that particular time.  This causes goofy behavior when crossing
/// standard / daylight time boundaries.
///
/// @param JD The Julian Date to convert
/// @param zonedate Pointer to ln_zonedate to be filled in
void my_get_local_date(double JD, struct ln_zonedate *zonedate) {
   struct ln_date date;
   time_t curtime;
   struct tm *loctime;
   long gmtoff;

   ln_get_date(JD, &date);

   /* add day light savings time and hour angle */
#ifdef __WIN32__
   _tzset();
   gmtoff = _timezone;
   if (_daylight)
      gmtoff += 3600;
#else
   ln_get_timet_from_julian(JD, &curtime);      // (AND NOT!!!) curtime = time (NULL);
   loctime = localtime(&curtime);
   gmtoff = loctime->tm_gmtoff;
   // otherwise there is no reasonable way how to get that:(
   // tm_gmtoff already included DST
#endif
   ln_date_to_zonedate(&date, zonedate, gmtoff);
}

/// @brief Normalize an angle in the range 0.0 .. 360.0
///
/// @param angle The angle to normalize, in degrees
/// @return The same angle in the range 0.0 .. 360.0
double normalize_angle(double angle) {
   if (angle < 0.0) {
      angle += 360.0 * (1.0 - (int)(angle / 360.0));
   }
   if (angle >= 360.0) {
      angle -= 360.0 * (int)(angle / 360.0);
   }
   return angle;
}

/// @brief Get the fractional part of a double, discarding the integer part.
///
/// @param arg The double of interest
/// @return The non-integer part of arg
double frac(double arg) {
   double integer;
   return modf(arg, &integer);
}

/// @brief Draw a time at x,y coordinates
///
/// @param canvas The Canvas to draw on
/// @param now The current Julian date
/// @param jd The time to draw
/// @param x X coordinate to center the text on
/// @param y Y coordinate to center the text on
/// @param fg The foreground color of the text
/// @param bg The background color of the text
/// @return The return value of text_canvas()
int
do_xy_time(Canvas * canvas, double now, double jd, int x, int y,
      unsigned int fg, unsigned int bg) {

   char time[32];
   struct ln_zonedate zonedate;
   my_get_local_date(jd, &zonedate);
   sprintf(time, "%02d:%02d", zonedate.hours, zonedate.minutes);
   return text_canvas(canvas, jd < now ? FONT_ITALIC_MED : FONT_BOLD_MED, x, y,
         fg, bg, time, 1, 3);
}

/// @brief Draw ticks every hour around the outside edge
///
/// Ticks are drawn using xor logic, so they should always be visible
///
/// @param canvas The Canvas to draw on
/// @param JD the Julian Date
/// @param x The X coordinate of the center of the clock
/// @param y The Y coordinate of the center of the clock
/// @param r The radius of the clock
/// @param up The angle used as "up"
/// @return void
void do_hour_ticks(Canvas * canvas, double JD, int x, int y, int r, double up) {
   double up_angle = frac(up) * 360.0;

   Canvas *shadow = new_canvas(canvas->w, canvas->h, COLOR_NONE);

#define HALFHOUR (1.0 / 24.0 / 2.0)

   for (int i = 0; i < 24; i++) {
      double when = JD - 0.5 + HALFHOUR + (double)i / 24.0;
      struct ln_zonedate zonedate;
      my_get_local_date(when, &zonedate);

      // account for some rounding errors
      if (zonedate.minutes % 10 == 9) {
         zonedate.minutes++;
         if (zonedate.minutes == 60) {
            zonedate.minutes = 0;
            zonedate.hours++;
            if (zonedate.hours == 24) {
               zonedate.hours = 0;
            }
         }
      }

      int hour = zonedate.hours;

      double mark = when - (float)zonedate.minutes / (24.0 * 60.0);

      double xa, ya;
      double xc, yc;

      double angle = frac(mark) * 360.0;

      xa = x + (r - SCALE(8)) * cos(DEG2RAD(angle - up_angle + 270.0));
      ya = y + (r - SCALE(8)) * sin(DEG2RAD(angle - up_angle + 270.0));
      xc = x + r * cos(DEG2RAD(angle - up_angle + 270.0));
      yc = y + r * sin(DEG2RAD(angle - up_angle + 270.0));
      line_canvas(shadow, (int)xa, (int)ya, (int)xc, (int)yc, COLOR_BLACK);

      xa = x + (r - SCALE(30)) * cos(DEG2RAD(angle - up_angle + 270.0));
      ya = y + (r - SCALE(30)) * sin(DEG2RAD(angle - up_angle + 270.0));
      char buf[32];
      sprintf(buf, "%02d", hour);
      text_canvas(shadow, FONT_BOLD_SMALL, (int)xa, (int)ya,
            COLOR_BLACK, COLOR_NONE, buf, 1, 3);
   }
   xor_canvas(shadow, canvas);

   delete_canvas(shadow);
}

/// @brief Draw the "now" hand
///
/// The now hand is drawn using xor logic
///
/// @param canvas The Canvas to draw on
/// @param up The Julian Date used as "up"
/// @param now The current Julian Date
/// @return void
void do_now_hand(Canvas * canvas, double up, double now) {

   //Canvas *shadow = new_canvas(canvas->w, canvas->h, COLOR_NONE);

   double up_angle = frac(up) * 360.0;
   double now_angle = frac(now) * 360.0 - up_angle + 270.0;
   double xc =
      canvas->w / 2.0 + (canvas->w / 2.0 / 2.0 +
            SCALE(128)) * cos(DEG2RAD(now_angle));
   double yc =
      canvas->h / 2.0 + (canvas->h / 2.0 / 2.0 +
            SCALE(128)) * sin(DEG2RAD(now_angle));
   double xc2 =
      canvas->w / 2.0 + (canvas->w / 2.0 / 2.0 -
            SCALE(128)) * cos(DEG2RAD(now_angle));
   double yc2 =
      canvas->h / 2.0 + (canvas->h / 2.0 / 2.0 -
            SCALE(128)) * sin(DEG2RAD(now_angle));

   thick_line_canvas(canvas, (int)xc2, (int)yc2, (int)xc, (int)yc, COLOR_WHITE,
         3);
   //xor_canvas(shadow, canvas);

   //delete_canvas(shadow);
}

/// @brief Draw the current time in the center of the Canvas
///
/// @param canvas The Canvas to draw on
/// @param now The current Julian Date
/// @return void
void do_now_time(Canvas * canvas, double now) {
   char time[32];
   struct ln_zonedate zonedate;
   my_get_local_date(now, &zonedate);
   sprintf(time, "%02d:%02d", zonedate.hours, zonedate.minutes);
   text_canvas(canvas, FONT_BOLD_BIG, canvas->w / 2, canvas->h / 2,
         COLOR_WHITE, COLOR_BLACK, time, 1, 12);
}

/// @brief Draw the current weekday in the center of the Canvas
///
/// @param canvas The Canvas to draw on
/// @param now The current Julian Date
/// @return void
void do_now_weekday(Canvas * canvas, double now) {
   struct ln_zonedate zonedate;
   my_get_local_date(now, &zonedate);

   struct ln_date date;
   date.years = zonedate.years;
   date.months = zonedate.months;
   date.days = zonedate.days;
   date.hours = date.minutes = 0;
   date.seconds = 0.0;

   unsigned int dow = ln_get_day_of_week(&date);

   char text[32];
   sprintf(text, "%s", weekdays[dow]);
   text_canvas(canvas, FONT_BOLD_MED, canvas->w / 2,
         canvas->h / 2 - (int)SCALE(90), COLOR_WHITE, COLOR_BLACK, text,
         1, 2);
}

/// @brief Draw the current date in the center of the Canvas
///
/// @param canvas The Canvas to draw on
/// @param now The current Julian Date
/// @return void
void do_now_date(Canvas * canvas, double now) {
   char time[32];
   struct ln_zonedate zonedate;
   my_get_local_date(now, &zonedate);
   sprintf(time, "%s-%d", months[zonedate.months - 1], zonedate.days);
   text_canvas(canvas, FONT_BOLD_MED, canvas->w / 2,
         canvas->h / 2 - (int)SCALE(60), COLOR_WHITE, COLOR_BLACK, time,
         1, 2);
}

/// @brief Draw the location in the center of the Canvas
///
/// @param canvas The Canvas to draw on
/// @param observer The lat/long of the observer position
/// @return void
void do_location(Canvas * canvas, struct ln_lnlat_posn *observer) {
   char location[128];

   double lat = observer->lat;
   char NS = 'N';
   if (lat < 0.0) {
      lat = -lat;
      NS = 'S';
   }

   double lon = observer->lng;
   char EW = 'E';
   if (lon < 0.0) {
      lon = -lon;
      EW = 'W';
   }
   char *degree = "\u00B0";     // in utf8, degree symbol

   if (lat > 90.0 || lon > 180.0 || lat < -90.0 || lon < -180.0) {
      sprintf(location, "INVALID_LOCATION");
   }
   else {
      sprintf(location, "%0.4f%s%c,%0.4f%s%c", lat, degree, NS, lon, degree,
            EW);
   }
   text_canvas(canvas, FONT_BOLD_SMALL, canvas->w / 2,
         canvas->h / 2 + (int)SCALE(48), COLOR_WHITE, COLOR_BLACK,
         location, 1, 2);
}

/// @brief Draw the current date in the center of the Canvas
///
/// @param canvas The Canvas to draw on
/// @param now The current Julian Date
/// @return void
void do_now_smalldate(Canvas * canvas, double now) {
   char time[32];
   struct ln_zonedate zonedate;
   my_get_local_date(now, &zonedate);
   sprintf(time, "%04d-%02d-%02d", zonedate.years, zonedate.months,
         zonedate.days);
   text_canvas(canvas, FONT_BOLD_SMALL, canvas->w / 2,
         canvas->h / 2 + (int)SCALE(72), COLOR_WHITE, COLOR_BLACK, time,
         1, 2);
}

/// @brief Draw the moon
///
/// @param context Pointer to computation context
/// @param canvas The Canvas to draw on
/// @param now The current Julian Date
/// @param lunar_phase The current lunar_phase as returned by libnova
/// @param lunar_bright_limb The current lunar_bright_limb as returned by libnova
/// @param where_angle The clock angle at which to draw the moon
/// @return void
void
do_moon_draw(Context *context, Canvas * canvas,
      double now,
      float lunar_phase, float lunar_bright_limb, double where_angle) {

   // this is, quite tedious.  here we go...
   int cx, cy;

   cx = canvas->w / 2 + (int)(canvas->w * cos(DEG2RAD(where_angle)) / 6.0);
   cy = canvas->h / 2 + (int)(canvas->h * sin(DEG2RAD(where_angle)) / 6.0);

   unsigned int interior_color;
   unsigned int chunk_color;
   int cxm;
   if (lunar_phase < 90.0) {
      interior_color = COLOR_BLACK;
      chunk_color = COLOR_LIGHTGRAY;
      if (lunar_bright_limb < 180.0) {
         cxm = 1;
      }
      else {
         cxm = -1;
      }
   }
   else {
      interior_color = COLOR_LIGHTGRAY;
      chunk_color = COLOR_BLACK;
      if (lunar_bright_limb < 180.0) {
         cxm = -1;
      }
      else {
         cxm = 1;
      }
   }

   // from wolfram alpha
   // a circle passing through points (0,a), (0,-a), and (b,0)
   int b = abs((int)(lunar_phase - 90)) * SCALE(40) / 90;
   if (b == 0) {
      b++;
   }
   int chunk_x = (b * b - SCALE(40) * SCALE(40)) / (2 * b);
   int chunk_r = abs(chunk_x - b);

   // this won't make sense, because it's derived from earlier code...
   // but it really does draw the moon...
   for (int dx = -SCALE(40); dx <= SCALE(40); dx++) {
      for (int dy = -SCALE(40); dy <= SCALE(40); dy++) {
         double d_interior = sqrt(dx * dx + dy * dy);
         double d_chunk = sqrt((dx - chunk_x * cxm) * (dx - chunk_x * cxm) +
               dy * dy);

         if (d_interior <= SCALE(40.0)) {
            if (d_chunk < chunk_r) {
               poke_canvas(canvas, cx + dx, cy + dy, chunk_color);
            }
            else {
               poke_canvas(canvas, cx + dx, cy + dy, interior_color);
            }
         }
      }
   }

   // shading for moon
   for (int dx = -SCALE(40); dx <= SCALE(40); dx++) {
      for (int dy = -SCALE(40); dy <= SCALE(40); dy++) {
         if (peek_canvas(canvas, cx + dx, cy + dy) == COLOR_LIGHTGRAY) {
            int d2 = dx * dx + dy * dy;
            unsigned int color = COLOR_LIGHTGRAY & 0xFF;
            color =
               color * (.8 +
                     .2 * (1.0 -
                        (2.0 * (double)(d2) /
                         (double)(SCALE(40) * SCALE(40)))));
            color =
               (COLOR_LIGHTGRAY & 0xFF000000) | (color << 16) | (color << 8) |
               (color);

            poke_canvas(canvas, cx + dx, cy + dy, color);
         }
      }
   }

   int is_up = -1;
   for (int i = 0; i < event_spot; i++) {
      if (events[i].jd > now) {
         break;
      }
      if (events[i].category == CAT_LUNAR) {
         switch (events[i].type) {
            case EVENT_UP:
            case EVENT_RISE:
               is_up = 1;
               break;
            case EVENT_DOWN:
            case EVENT_SET:
               is_up = 0;
               break;
            case EVENT_TRANSIT:
               // do nothing
               break;
         }
      }
   }
   // outline
   if (is_up == 1) {
      arc_canvas(canvas, cx, cy, SCALE(40), 1, COLOR_WHITE, 0, 360.0);
      arc_canvas(canvas, cx, cy, SCALE(43), 1, COLOR_WHITE, 0, 360.0);
   }
   else {
      arc_canvas(canvas, cx, cy, SCALE(40), 1, COLOR_BLACK, 0, 360.0);
      arc_canvas(canvas, cx, cy, SCALE(43), 1, COLOR_BLACK, 0, 360.0);
   }
}

/// @brief Draw the perimeter planet band
///
/// @param context Pointer to computation context
/// @param canvas The Canvas to draw on
/// @param up The Julian Date used as "up" on the clock
/// @param now The current Julian Date
/// @param color The color to use when drawing
/// @param radius The radius at which to draw the band
/// @param category The planet category
/// @param sign_only True indicates only planet sign to be drawn
/// @return void
void
do_planet_band(Context *context, Canvas * canvas, double up, double now,
      unsigned int color, double radius, EventCategory category,
      int sign_only) {

   double base = (double)((int)(now - 0.5));
   double up_angle = frac(up) * 360.0;
   char sym[2] = { 0, 0 };
   sym[0] = 'B' + (category - CAT_LUNAR);
   if (category == CAT_ARIES) { sym[0] = 'a'; }
   int is_up = 0;

   // find initial state

   for (int i = 0; i < event_spot; i++) {
      if (events[i].prune == 0) {
         break;
      }
      if (events[i].category == category) {
         switch(events[i].type) {
            case EVENT_DOWN:
            case EVENT_SET:
               is_up = 0;
               break;
            default:
               is_up = 1;
               break;
         }
      }
   }

   // draw arcs and ticks

   double last = is_up ? (now - .5) : 0.0;
   for (int i = 0; i < event_spot; i++) {
      if (events[i].prune == 0 && events[i].category == category) {

         double angle =
            (events[i].jd - base) * 360.0 - up_angle + 270.0;

         double start_angle;

         switch (events[i].type) {
            case EVENT_UP:
            case EVENT_RISE:
               last = events[i].jd;
               break;
            case EVENT_DOWN:
            case EVENT_TRANSIT:
            case EVENT_SET:
               start_angle = (last - base) * 360.0 - up_angle + 270.0;
               if (!sign_only) {
                  arc_canvas_shaded(canvas, canvas->w / 2, canvas->h / 2,
                     radius, SCALE(5), color, start_angle, angle);
               }
               if (events[i].type == EVENT_SET) {
                  last = 0.0;
               }
               else {
                  last = events[i].jd;
               }
               break;
         }
         // always do a tick, regardless of type
         double x1, y1, x2, y2;

         x1 = canvas->w / 2 + (radius - 16) * cos(DEG2RAD(angle));
         y1 = canvas->h / 2 + (radius - 16) * sin(DEG2RAD(angle));
         x2 = canvas->w / 2 + (radius + 16) * cos(DEG2RAD(angle));
         y2 = canvas->h / 2 + (radius + 16) * sin(DEG2RAD(angle));

         line_canvas(canvas, x1, y1, x2, y2, color);
      }
   }

   // finish the arc

   if (last != 0.0) {
      double start_angle =
         (last - base) * 360.0 - up_angle + 270.0;
      double stop_angle =
         (now + .5 - base) * 360.0 - up_angle + 270.0;

      if (!sign_only) {
         arc_canvas_shaded(canvas, canvas->w / 2, canvas->h / 2,
            radius, SCALE(5), color, start_angle, stop_angle);
      }
   }

   // draw characters at ticks

   bool drawn = false;
   for (int i = 0; i < event_spot; i++) {
      if (events[i].prune == 0 && events[i].category == category) {
         if (events[i].type == EVENT_RISE || events[i].type == EVENT_SET) {
            double offset = (events[i].type == EVENT_SET) ? 3.0 : -3.0;
            double x, y;
            double angle =
               (events[i].jd - base) * 360.0 - up_angle + 270.0;

            x = (canvas->w / 2) + radius * cos(DEG2RAD(angle + offset));
            y = (canvas->h / 2) + radius * sin(DEG2RAD(angle + offset));
            text_canvas(canvas, ASTRO_FONT, x, y, color,
                  COLOR_BLACK, sym, 1, 1);
            drawn = true;
         }
      }
   }

   if (is_up && !drawn) {
      for (int i = 0; i < event_spot; i++) {
         if (events[i].prune == 0 && events[i].category == category) {
            if (events[i].type == EVENT_TRANSIT) {
               double x, y;
               double angle =
                  (events[i].jd - base) * 360.0 - up_angle + 270.0;

               x = (canvas->w / 2) + radius * cos(DEG2RAD(angle + 3.0));
               y = (canvas->h / 2) + radius * sin(DEG2RAD(angle + 3.0));
               text_canvas(canvas, ASTRO_FONT, x, y, color, COLOR_BLACK, sym, 1, 1);
               drawn = true;
            }
         }
      }
   }

   if (is_up && !drawn) {
      for (int i = 0; i < event_spot; i++) {
         if (events[i].category == category) {
            if (events[i].type == EVENT_TRANSIT) {
               double x, y;
               double angle =
                  (events[i].jd - base) * 360.0 - up_angle + 270.0;

               x = (canvas->w / 2) + radius * cos(DEG2RAD(angle + 3.0));
               y = (canvas->h / 2) + radius * sin(DEG2RAD(angle + 3.0));
               text_canvas(canvas, ASTRO_FONT, x, y, color, COLOR_BLACK, sym, 1, 1);
               drawn = true;
            }
         }
      }
   }

   if (is_up && !drawn) { // shazbat
      double x = (canvas->w / 2) +
         (radius + 15) * cos(DEG2RAD((int)(category - CAT_LUNAR) * (360 / 6)));
      double y = (canvas->h / 2) +
         (radius + 15) * sin(DEG2RAD((int)(category - CAT_LUNAR) * (360 / 6)));
      text_canvas(canvas, ASTRO_FONT, x, y, color, COLOR_BLACK, sym, 1, 1);
   }
}

/// @brief Clear the events list
///
/// @param context Pointer to computation context
void events_clear(Context *context) {
   event_spot = 0;
}

/// @brief Compare two events
///
/// This is a helpwer function used by qsort() to order
/// the events in the event list.
int event_compar(const void *a, const void *b) {
   Event *pa = (Event *) a;
   Event *pb = (Event *) b;

   if (pa->jd < pb->jd) {
      return -1;
   }
   else if (pa->jd > pb->jd) {
      return 1;
   }
   else if (pa->type < pb->type) {
      return -1;
   }
   else if (pa->type > pb->type) {
      return 1;
   }
   else {
      if (pa->category == pb->category) {
         return 0;
      }

      // same type, different category
      static const EventCategory order_up[] = { //"ancsl"
         CAT_ASTRONOMICAL,
         CAT_NAUTICAL,
         CAT_CIVIL,
         CAT_SOLAR,
         CAT_LUNAR,
         CAT_MERCURY,
         CAT_VENUS,
         CAT_MARS,
         CAT_JUPITER,
         CAT_SATURN,
         CAT_ARIES
      };

      static const EventCategory order_down[] = {       //"lscna"
         CAT_LUNAR,
         CAT_MERCURY,
         CAT_VENUS,
         CAT_MARS,
         CAT_JUPITER,
         CAT_SATURN,
         CAT_ARIES,
         CAT_SOLAR,
         CAT_CIVIL,
         CAT_NAUTICAL,
         CAT_ASTRONOMICAL
      };

      const EventCategory *order = order_up;

      if (pa->type == EVENT_DOWN) {
         order = order_down;
      }

      // TODO FIX revisit this logic

      for (int p = 0; p < 10; p++) {
         if (pa->category == order[p]) {
            return -1;
         }
         if (pb->category == order[p]) {
            return 1;
         }
      }

      // huh??? this should never happen
      return 0;
   }
}

/// @brief Sort the events list
///
/// Uses the event_compar function
///
/// @param context Pointer to computation context
void events_sort(Context *context) {
   qsort(events, event_spot, sizeof(Event), event_compar);
}

/// @brief Uniq the events list
///
/// Uses the event_compar function
///
/// @param context Pointer to computation context
void events_uniq(Context *context) {
   int i = 0;
   while (i < event_spot - 1) {
      if (!event_compar(events + i, events + (i + 1))) {
         assert(event_spot > 0);
         memmove(events + i, events + (i + 1),
               sizeof(Event) * (event_spot - i));
         event_spot--;
      }
      else {
         i++;
      }
   }
}

/// @brief Mark events of interest / noninterest
///
/// Used to mark events occurring +/- 12 hours from current time.
///
/// @param context Pointer to computation context
/// @param jd The curren Julian Date
/// @return void
void events_prune(Context *context, double jd) {
   for (int i = 0; i < event_spot; i++) {
      events[i].prune = (fabs(events[i].jd - jd) > 0.5) ? 1 : 0;
   }
}

/// @brief For debugging, print the events list
///
/// @param context Pointer to computation context
void events_dump(Context *context) {
   struct ln_zonedate zonedate;

   for (int i = 0; i < event_spot; i++) {
      my_get_local_date(events[i].jd, &zonedate);
      printf("[%05d] %6.12f %02d:%02d %d %s %s\n", i, events[i].jd,
            zonedate.hours, zonedate.minutes,
            events[i].prune, typenames[events[i].type],
            categorynames[events[i].category]);
   }
}

/// @brief One minute of Julian Date.
#define ONE_MINUTE_JD (1.0/(24.0*60.0))

/// @brief Typedef for functions returning Equ coordinates of object
typedef void (*Get_Equ_Coords)(double, struct ln_equ_posn *);

void ln_get_aries_equ_coords(double JD, struct ln_equ_posn *posn) {
   posn->dec = 0.0;
   posn->ra = 0.0;
}

struct Cache *new_cache(struct ln_lnlat_posn *observer) {
   struct Cache *ret = (struct Cache *)malloc(sizeof(struct Cache));
   ret->head = NULL;
   ret->observer = *observer;
   return ret;
}

int cnc = 0;

struct CacheNode *new_cache_node(struct CacheNode *prev, struct CacheNode *next) {
   struct CacheNode *ret = (struct CacheNode *)calloc(1, sizeof(struct CacheNode));
   if (prev != NULL) {
      prev->next = ret;
   }
   if (next != NULL) {
      next->prev = ret;
   }
   ret->prev = prev;
   ret->next = next;
//cnc++;printf("cnc N=%d\n", cnc);
   return ret;
}

void delete_cache_node(struct CacheNode *p) {
   if (p->prev != NULL) {
      p->prev->next = p->next;
   }
   if (p->next != NULL) {
      p->next->prev = p->prev;
   }
   free(p);
//cnc--;printf("cnc D=%d\n", cnc);
}

/// @brief Find up and down events
///
/// @param context Pointer to computation context
/// @param JD The current Julian Date
/// @param observer latitude and logitude of observer
/// @param get_equ_coords Function pointer to get the body coordinates
/// @param horizon The horizon used for events
/// @param category The EventCategory to use
/// @return void
void events_populate_anything_updown(Context *context, double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      double horizon, EventCategory category) {
   struct ln_equ_posn posn;
   struct ln_hrz_posn hrz_posn;
   double angle;

   JD -= ((double) CACHE_LIMIT);

   get_equ_coords(JD, &posn);
   ln_get_hrz_from_equ(&posn,
         observer,
         JD, &hrz_posn);
   angle = hrz_posn.alt;

   if (angle < horizon) {
      assert(event_spot < NUM_EVENTS);
      events[event_spot++] = (Event) { JD, category, EVENT_DOWN};
   }
   else {
      assert(event_spot < NUM_EVENTS);
      events[event_spot++] = (Event) { JD, category, EVENT_UP};
   }
}

///@brief Structure used fro events_populate_anything_array on multiple horizons
struct ECH {
   EventCategory category;
   double horizon;
};

/// @brief initial rough time gap [minutes] for finding events
#define SOLUN_PRESTEP 120 // seems to be around the sweet spot
#define PLANET_PRESTEP 120 // seems to be around the sweet spot

int transit_minima(struct CacheNode *a, struct CacheNode *b, struct CacheNode *c) {
   if (b->hrz_posn.alt >= a->hrz_posn.alt && b->hrz_posn.alt >= c->hrz_posn.alt) {
      return 1;
   }
   else if (b->hrz_posn.alt <= a->hrz_posn.alt && b->hrz_posn.alt <= c->hrz_posn.alt) {
      return -1;
   }
   return 0;
}

int rise_set(struct CacheNode *a, struct CacheNode *b, double horizon) {
   if (a->hrz_posn.alt <= horizon && b->hrz_posn.alt >= horizon) {
      return 1;
   }
   else if (a->hrz_posn.alt >= horizon && b->hrz_posn.alt <= horizon) {
      return -1;
   }
   return 0;
}

struct CacheNode *insert_between(struct CacheNode *a, struct CacheNode *b,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords) {
   struct CacheNode *q = new_cache_node(a, b);
   q->JD = (a->JD + b->JD) / 2.0;
   get_equ_coords(q->JD, &(q->posn));
   ln_get_hrz_from_equ(&(q->posn), observer, q->JD, &(q->hrz_posn));
   return q;
}

struct CacheNode *insert_head(struct CacheNode *oldhead, double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords) {
   struct CacheNode *q = new_cache_node(NULL, oldhead);
   q->JD = JD;
   get_equ_coords(q->JD, &(q->posn));
   ln_get_hrz_from_equ(&(q->posn), observer, q->JD, &(q->hrz_posn));
   return q;
}

struct CacheNode *insert_tail(struct CacheNode *oldtail, double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords) {
   struct CacheNode *q = new_cache_node(oldtail, NULL);
   q->JD = JD;
   get_equ_coords(q->JD, &(q->posn));
   ln_get_hrz_from_equ(&(q->posn), observer, q->JD, &(q->hrz_posn));
   return q;
}

/// @brief given an ECH array, create events
///
/// @param context Pointer to computation context
/// @param JD The Julian Date
/// @param observer Observer position
/// @param get_equ_coords Function pointer to get body coordinates
/// @param ech_count Size of echs array
/// @param echs An array of ECH
/// @param cache Pointer to the position cache for this object
/// @return void
void events_populate_anything_array(Context *context, double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      int ech_count, struct ECH *echs,
      struct Cache *cache) {

   // trim junk from beginning
   while(cache->head != NULL && cache->head->JD < (JD - CACHE_LIMIT)) {
      struct CacheNode *tmp = cache->head;
      cache->head = cache->head->next;
//      printf("delete [%d] %p JD=%lf before JD=%lf\n", __LINE__, tmp, tmp->JD, JD-CACHE_LIMIT);
      delete_cache_node(tmp);
   }

   // affirm we have a head
   if (cache->head == NULL || cache->head->JD > (JD - CACHE_LIMIT)) {
//      printf("new head [%d] %p JD=%lf after JD=%lf\n", __LINE__, cache->head, cache->head ? cache->head->JD : -1.0, JD-CACHE_LIMIT);
      cache->head = insert_head(cache->head,
                                JD - CACHE_LIMIT,
                                observer,
                                get_equ_coords);
   }

   double step = (echs[0].category <= CAT_LUNAR) ?
      ((double)SOLUN_PRESTEP / (24.0*60.0)) :
      ((double)PLANET_PRESTEP / (24.0*60.0));

   // affirm things in the middle, and proper end
   for (struct CacheNode *p = cache->head; p != NULL; p = p->next) {
      // must have a proper tail
      if (p->next == NULL && p->JD < JD + CACHE_LIMIT) {
//         printf("new tail after [%d] %p JD=%lf after JD=%lf\n", __LINE__, p, p->JD, JD+CACHE_LIMIT);
         insert_tail(p, JD + CACHE_LIMIT, observer, get_equ_coords);
      }

      while (p->next && fabs(p->next->JD - p->JD) > step) {
//         printf("new middle between [%d] %p JD=%lf %p JD=%lf step=%lf\n", __LINE__, p, p->JD, p->next, p->next->JD, step);
         insert_between(p, p->next, observer, get_equ_coords);
      }

      while (p->next && p->next->JD > (JD + CACHE_LIMIT)) {
//         printf("delete hanger [%d] %p JD=%lf %lf\n", __LINE__, p->next, p->next->JD, JD + CACHE_LIMIT);
         delete_cache_node(p->next);
      }
   }

   // now walk the list, setting positions
   if (cache->observer.lat != observer->lat ||
       cache->observer.lng != observer->lng) {
      cache->observer = *observer;
      for (struct CacheNode *p = cache->head;
            p != NULL;
            p = p->next) {
         ln_get_hrz_from_equ(&(p->posn), observer, p->JD, &(p->hrz_posn));
      }
   }

   // create up/down events

   for (int k = 0; k < ech_count; k++) {
      EventCategory category = echs[k].category;
      double horizon = echs[k].horizon;

      events_populate_anything_updown(context,
                                      JD,
                                      observer,
                                      get_equ_coords,
                                      horizon,
                                      category);
   }

   // begin looking for rise / set / transit events

   for (int k = 0; k < ech_count; k++) {
      EventCategory category = echs[k].category;
      double horizon = echs[k].horizon;

      for (struct CacheNode *p = cache->head;
           p != NULL;
           p = p->next) {

         // detect transit and minima
         // must do this BEFORE rise/set detection
         int minmax = 0;
         while (p->next != NULL && p->next->next != NULL &&
                (minmax = /*assignment*/
                 transit_minima(p, p->next, p->next->next)) != 0) {
            int flag = 0;
            if (fabs(p->next->JD - p->next->next->JD) * (24.0 * 60.0) >= 1.0) {
//               printf("insert between minmax [%d] %p JD=%lf %p JD=%lf\n", __LINE__, p->next, p->next->JD, p->next->next, p->next->next->JD);
               insert_between(p->next, p->next->next, observer, get_equ_coords);
               flag = 1;
            }
            if (fabs(p->JD - p->next->JD) * (24.0 * 60.0) >= 1.0) {
//               printf("insert between minmax [%d] %p JD=%lf %p JD=%lf\n", __LINE__, p, p->JD, p->next, p->next->JD);
               insert_between(p, p->next, observer, get_equ_coords);
               flag = 1;
            }
            if (flag == 0) {
               p->critical = true;
               p->next->critical = true;
               p->next->next->critical = true;
               if (minmax > 0 && (p->next->hrz_posn.alt >= horizon || category < CAT_LUNAR)) {
                  assert(event_spot < NUM_EVENTS);
                  events[event_spot++] =
                     (Event) { p->next->JD, category, EVENT_TRANSIT };
               }
               break;
            }
         }

         // detect rise/set
         int riseset = 0;
         while (p->next != NULL &&
                (riseset = /*assignment*/ rise_set(p, p->next, horizon)) != 0) {
            if (fabs(p->JD - p->next->JD) * (24.0 * 60.0) >= 1.0) {
//               printf("insert between riseset[%d] %p JD=%lf %p JD=%lf\n", __LINE__, p, p->JD, p->next, p->next->JD);
               insert_between(p, p->next, observer, get_equ_coords);
            }
            else {
               p->critical = true;
               p->next->critical = true;
               assert(event_spot < NUM_EVENTS);
               if (riseset > 0) {
                  events[event_spot++] =
                     (Event) { p->JD, category, EVENT_RISE };
               }
               else if (riseset < 0) {
                  events[event_spot++] =
                     (Event) { p->JD, category, EVENT_SET };
               }
               else {
                  assert(0);
               }
               break;
            }
         }
      }
   }

   for (struct CacheNode *p = cache->head;
        p != NULL;
        p = p->next) {

      while (p->next != NULL && p->next->next != NULL && p->next->critical == 0 &&
             (p->next->next->JD - p->JD) < step) {
//         printf("delete noncritical [%d] %p JD=%lf\n", __LINE__, p->next, p->next->JD);
         delete_cache_node(p->next);
      }
   }
}

/// @brief Like events_populate_anything_array, but for single category / horizon
///
/// This just creates a tmp array and calls events_populate_anything_array
///
/// @param context Pointer to computation context
/// @param JD The Julian Date
/// @param observer Observer position
/// @param get_equ_coords Function pointer to get body coordinates
/// @param horizon Horizon to use
/// @param category EventCategory to use
/// @param cache Pointer to the position cache for this object
/// @return void
void events_populate_anything(Context *context, double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      double horizon, EventCategory category,
      struct Cache *cache) {
   struct ECH ech = (struct ECH) { category, horizon };
   events_populate_anything_array(context, JD, observer, get_equ_coords, 1, &ech, cache);
}

/// @brief Create rise/transit/set events for all things solar
///
/// This just creates a tmp array and calls events_populate_anything_array
///
/// @param context Pointer to computation context
/// @param JD The Julian Date
/// @param observer Observer position
/// @return void
void events_populate_solar(Context *context, double JD, struct ln_lnlat_posn *observer) {
   struct ECH echs[] = {
      (struct ECH) { CAT_ASTRONOMICAL, LN_SOLAR_ASTRONOMICAL_HORIZON },
      (struct ECH) { CAT_NAUTICAL,     LN_SOLAR_NAUTIC_HORIZON },
      (struct ECH) { CAT_CIVIL,        LN_SOLAR_CIVIL_HORIZON },
      (struct ECH) { CAT_SOLAR,        LN_SOLAR_STANDART_HORIZON },
   };
   if (sun_cache == NULL) {
      sun_cache = new_cache(observer);
   }
   events_populate_anything_array(context, JD, observer, ln_get_solar_equ_coords,
         4, echs, sun_cache);
}

/// @brief Create rise/transit/set events for all things lunar
///
/// This just events_populate_anything
///
/// @param context Pointer to computation context
/// @param JD The Julian Date
/// @param observer Observer position
/// @return void
void events_populate_lunar(Context *context, double JD, struct ln_lnlat_posn *observer) {
   if (moon_cache == NULL) {
      moon_cache = new_cache(observer);
   }
   events_populate_anything(context, JD, observer, ln_get_lunar_equ_coords,
         LN_LUNAR_STANDART_HORIZON, CAT_LUNAR, moon_cache);
}

/// @brief Create rise/transit/set events for all planets
///
/// This just events_populate_anything for each planet
///
/// @param context Pointer to computation context
/// @param JD The Julian Date
/// @param observer Observer position
/// @return void
void events_populate_planets(Context *context, double JD, struct ln_lnlat_posn *observer) {
   if (mercury_cache == NULL) {
      mercury_cache = new_cache(observer);
   }
   events_populate_anything(context, JD, observer,
         ln_get_mercury_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_MERCURY, mercury_cache);

   if (venus_cache == NULL) {
      venus_cache = new_cache(observer);
   }
   events_populate_anything(context, JD, observer,
         ln_get_venus_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_VENUS, venus_cache);

   if (mars_cache == NULL) {
      mars_cache = new_cache(observer);
   }
   events_populate_anything(context, JD, observer,
         ln_get_mars_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_MARS, mars_cache);

   if (jupiter_cache == NULL) {
      jupiter_cache = new_cache(observer);
   }
   events_populate_anything(context, JD, observer,
         ln_get_jupiter_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_JUPITER, jupiter_cache);

   if (saturn_cache == NULL) {
      saturn_cache = new_cache(observer);
   }
   events_populate_anything(context, JD, observer,
         ln_get_saturn_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_SATURN, saturn_cache);

   if (aries_cache == NULL) {
      aries_cache = new_cache(observer);
   }
   events_populate_anything(context, JD, observer,
         ln_get_aries_equ_coords, 0.0,
         CAT_ARIES, aries_cache);
}

/// @brief Populate the event list
///
/// @param context Pointer to computation context
/// @param JD The current Julian Date
/// @param observer The observer's lat/lon coordinates
/// @return void
void events_populate(Context *context, double JD, struct ln_lnlat_posn *observer) {
   events_populate_solar(context, JD, observer);
   events_populate_lunar(context, JD, observer);
   events_populate_planets(context, JD, observer);
}

/// @brief Figure out which way is "up"
///
/// @param context Pointer to computation context
/// @param jd The current Julian Date
/// @return A Julian Date useful as "up", usually a solar transit
double events_transit(Context *context, double jd) {
   // any non pruned, non lunar, will do...
   for (int i = 0; i < event_spot; i++) {
      if (!events[i].prune &&
            events[i].type == EVENT_TRANSIT && events[i].category < CAT_LUNAR) {
         return frac(events[i].jd);
      }
   }
   // hrm, how about a pruned solar transit?
   for (int i = 0; i < event_spot; i++) {
      if (events[i].type == EVENT_TRANSIT && events[i].category < CAT_LUNAR) {
         return frac(events[i].jd);
      }
   }
   // non?  how about a lunar transit?
   for (int i = 0; i < event_spot; i++) {
      if (!events[i].prune &&
            events[i].type == EVENT_TRANSIT && events[i].category == CAT_LUNAR) {
         return frac(events[i].jd);
      }
   }
   // still nothing? LOCAL noon it is i guess
   struct ln_zonedate now;
   my_get_local_date(jd, &now);
   double ret = jd;
   if (now.seconds != 0) {
      ret += (60.0 - now.seconds) / (24.0 * 60.0 * 60.0);
      now.seconds = 0;
      now.minutes++;
   }
   if (now.minutes != 0) {
      ret += (60.0 - now.minutes) / (24.0 * 60.0);
      now.minutes = 0;
      now.hours++;
   }
   if (now.hours < 12) {
      ret += ((double)(12 - now.hours)) / 24.0;
   }
   else if (now.hours > 12) {
      ret -= ((double)(now.hours - 12)) / 24.0;
   }
   return frac(ret);
}

/// @brief Redraw stored accumulator times
///
/// This fixes some drawing oddities on very small screens
///
/// @param context Pointer to computation context
/// @param canvas Pointer to the canvas object
/// @returns void
void replay_accum_memory(Context *context, Canvas * canvas) {
   for (int i = 0; i < accumdrawnspot; i++) {
      text_canvas(canvas, FONT_BOLD_MED, accumdrawnmemory[i].x,
            accumdrawnmemory[i].y, accumdrawnmemory[i].fg,
            accumdrawnmemory[i].bg, accumdrawnmemory[i].str, 1, 3);
      free(accumdrawnmemory[i].str);
   }
   accumdrawnspot = 0;
}

// fwd declaration to appease the compiler gods
int check(Context *context, int x, int y, int w, int h);

/// @brief Helper function to accumulate total sunlight/night/whatever hours
///
/// @param context Pointer to computation context
/// @param canvas The Canvas to draw on
/// @param angle The angle used to derive total time
/// @param label The string label to draw
/// @param draw_angle Where to draw it
/// @param fore The foreground color
/// @param back The background color
/// @return void
void
accum_helper(Context *context, Canvas * canvas,
      double angle,
      char *label,
      double draw_angle, unsigned int fore, unsigned int back) {

printf("%s %08X %08X\n", __FUNCTION__, fore, back);
   angle = angle * 24.0 / 360.0;
   int hours = (int)angle;
   int minutes = (angle - (double)hours) * 60.0;
   char buffer[4096];
   if (minutes != 0) {
      if (hours != 0) {
         sprintf(buffer, "%dh %dm\n%s", hours, minutes, label);
      }
      else {
         sprintf(buffer, "%dm\n%s", minutes, label);
      }
   }
   else {
      sprintf(buffer, "%dh\n%s", hours, label);
   }

   double x;
   double y;
   int wh =
      text_canvas(canvas, FONT_BOLD_MED, -1000, -1000, fore, back, buffer, 1, 3);
   int w = wh >> 16;
   int h = wh & 0xFFFF;

   int collision;
   int radius = (canvas->w / 4);
   do {
      x = (canvas->w / 2) + radius * cos(DEG2RAD(draw_angle));
      y = (canvas->h / 2) + radius * sin(DEG2RAD(draw_angle));

      collision = check(context, x, y, w, h);

      radius--;
   }
   while (collision);

   assert(accumdrawnspot < NUM_ACCUM);
   accumdrawnmemory[accumdrawnspot++] = (AccumDrawnMemory) {
      x, y, fore, back, strdup(buffer)};
}

/// @brief A helper function to determine when labels collide
///
/// @param context Pointer to computation context
/// @param x X coordinate of desired label
/// @param y Y coordinate of desired label
/// @param w Width of desired label
/// @param h height of desired label
/// @return 1 if there is a collision, 0 otherwise
int check(Context *context, int x, int y, int w, int h) {
   w += 4;
   h += 4;
   x -= w / 2;
   y -= h / 2;

   for (int i = 0; i < timedrawnspot; i++) {
      int rw = timedrawnmemory[i].w + 4;
      int rh = timedrawnmemory[i].h + 4;
      int rx = timedrawnmemory[i].x - rw / 2;
      int ry = timedrawnmemory[i].y - rh / 2;

      if (rx > x + w) {
         continue;
      }
      if (x > rx + rw) {
         continue;
      }
      if (ry > y + h) {
         continue;
      }
      if (y > ry + rh) {
         continue;
      }

      return 1;
   }
   return 0;
}

/// @brief Draw a time for the sun band given theta,radius
///
/// @param context Pointer to computation context
/// @param canvas The Canvas to draw on
/// @param now The current Julian Date
/// @param jd The Julian Date for the event of interest
/// @param theta The desired angle
/// @param radius The desired radius
/// @param fg The foreground color for text
/// @param bg The background color for text
/// @return void
void
do_tr_time_sun(Context *context, Canvas * canvas, double now, double jd, double theta,
      double radius, unsigned int fg, unsigned int bg) {

printf("%s %08X %08X\n", __FUNCTION__, fg, bg);
   // get width and height by drawing offscreen
   int wh = do_xy_time(canvas, now, jd, -300, -300, fg, bg);
   int w = wh >> 16;
   int h = wh & 0xFFFF;

   int collision;
   int x, y;

   do {
      x = (canvas->w / 2) + radius * cos(DEG2RAD(theta));
      y = (canvas->h / 2) + radius * sin(DEG2RAD(theta));

      collision = check(context, x, y, w, h);

      radius--;
   }
   while (collision);

   assert(timedrawnspot < NUM_TIMEDRAWN);
   timedrawnmemory[timedrawnspot++] = (TimeDrawnMemory) {
      now, jd, x, y, w, h, fg, bg};

   // don't draw here, we'll get it when we replay
   // do_xy_time(canvas, now, jd, x, y, fg, bg);
}

/// @brief Redraw stored sun rise/set times
///
/// This fixes some drawing oddities on very small screens
///
/// @param context Pointer to computation context
/// @param canvas Pointer to the canvas object
/// @returns void
void replayTimeDrawnMemory(Context *context, Canvas * canvas) {
   for (int i = 0; i < timedrawnspot; i++) {
      do_xy_time(canvas,
            timedrawnmemory[i].now,
            timedrawnmemory[i].jd,
            timedrawnmemory[i].x,
            timedrawnmemory[i].y,
            timedrawnmemory[i].fg,
            timedrawnmemory[i].bg);
   }
}

/// @brief Draw colored bands for solar position
///
/// @param context Pointer to computation context
/// @param canvas The Canvas to draw on
/// @param up The Julian Date for "up"
/// @param now The Julian Date for the current time
/// @param lightdark Controls what is considered "light" and "dark"
/// @return void
void do_sun_bands(Context *context, Canvas * canvas, double up, double now, int lightdark) {
   static const double one_minute = 360.0 / 24.0 / 60.0;

   double last = now - 0.5;
   double base = (double)((int)(last));

   int light = lightdark >> 8;
   int dark = lightdark & 0xFF;

   // we begin in darkness...
   unsigned int color = COLOR_DARKNESS;
   unsigned int fore = COLOR_WHITE;
   unsigned int back = COLOR_DARKNESS;
   unsigned int ld_color = COLOR_DARK;

   unsigned int lightdark_civil = COLOR_TWILIGHT;
   unsigned int lightdark_nautical = COLOR_TWILIGHT;
   unsigned int lightdark_astronomical = COLOR_TWILIGHT;

   unsigned int transit_fore = COLOR_GREEN;
   unsigned int transit_back = COLOR_RED;

   double angle_daylight = 0.0;
   double angle_civil = 0.0;
   double angle_nautical = 0.0;
   double angle_astronomical = 0.0;
   double angle_darkness = 0.0;

   double up_angle = frac(up) * 360.0;

   int started = 0;
   int lockstart = 0;
   double transited = 0.0;

   int times_written = 0;

   bool arcd = false;

   if (lightdark != 0) {
      int light = lightdark >> 8;
      int dark = lightdark & 0xff;

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

   // big, messy state machine...
   for (int i = 0; i < event_spot; i++) {
      if (events[i].prune == 0) {
         lockstart = 1;
      }

      if (events[i].category < CAT_LUNAR) {
         if (events[i].type != EVENT_TRANSIT) {
            if (events[i].prune == 0) {
               double here = events[i].jd;
               double start_angle = (last-base) * 360.0 - up_angle + 270.0;
               double stop_angle = (here-base) * 360.0 - up_angle + 270.0;

               arcd = true;
               arc_canvas_shaded(canvas, canvas->w / 2,
                     canvas->h / 2,
                     canvas->w / 2 / 2,
                     canvas->h / 2 / 2, color, start_angle,
                     stop_angle);
               arc_canvas(canvas, canvas->w/2, canvas->w/2, canvas->w/2 / 2 + SCALE(126), 7, ld_color,
                     start_angle, stop_angle);

               // accumulate angle_daylight and angle_darkness
               start_angle = normalize_angle(start_angle);
               stop_angle = normalize_angle(stop_angle);
               while (stop_angle < start_angle) {
                  stop_angle += 360.0;
               }
               switch (color) {
                  case COLOR_DAYLIGHT:
                     angle_daylight += stop_angle - start_angle;
                     break;
                  case COLOR_CIVIL:
                     angle_civil += stop_angle - start_angle;
                     break;
                  case COLOR_NAUTICAL:
                     angle_nautical += stop_angle - start_angle;
                     break;
                  case COLOR_ASTRONOMICAL:
                     angle_astronomical += stop_angle - start_angle;
                     break;
                  case COLOR_DARKNESS:
                     angle_darkness += stop_angle - start_angle;
                     break;
               }

               if (started) {
                  times_written++;
                  // TODO FIX check size
                  do_tr_time_sun(context, canvas, now, last, start_angle,
                        (canvas->w / 3 - SCALE(32)), LOCK(fore), COLOR_LOCK); //fore, back);
               }

               last = here;
               started = 1;
            }
            else if (started) {
               break;
            }
         }
         else {
            if (!events[i].prune) {
               if (transited == 0.0) {
                  transited = events[i].jd;
                  transit_fore = fore;
                  transit_back = back;
               }
            }
         }

         if (!lockstart || !events[i].prune) {
            if (events[i].type == EVENT_UP || events[i].type == EVENT_RISE) {
               switch (events[i].category) {
                  case CAT_ASTRONOMICAL:
                     color = back = COLOR_ASTRONOMICAL;
                     ld_color = lightdark_astronomical;
                     fore = COLOR_WHITE;
                     break;
                  case CAT_NAUTICAL:
                     color = back = COLOR_NAUTICAL;
                     ld_color = lightdark_nautical;
                     fore = COLOR_WHITE;
                     break;
                  case CAT_CIVIL:
                     color = back = COLOR_CIVIL;
                     ld_color = lightdark_civil;
                     fore = COLOR_BLACK;
                     break;
                  case CAT_SOLAR:
                     color = back = COLOR_DAYLIGHT;
                     ld_color = COLOR_LIGHT;
                     fore = COLOR_BLACK;
                     break;
                  case CAT_LUNAR:
                  case CAT_MERCURY:
                  case CAT_VENUS:
                  case CAT_MARS:
                  case CAT_JUPITER:
                  case CAT_SATURN:
                  case CAT_ARIES:
                     // do nothing
                     break;
               }
            }

            if (events[i].type == EVENT_SET) {
               switch (events[i].category) {
                  case CAT_ASTRONOMICAL:
                     back = color;
                     color = COLOR_DARKNESS;
                     ld_color = COLOR_DARK;
                     fore = COLOR_WHITE;
                     break;
                  case CAT_NAUTICAL:
                     back = color;
                     color = COLOR_ASTRONOMICAL;
                     ld_color = lightdark_astronomical;
                     fore = COLOR_WHITE;
                     break;
                  case CAT_CIVIL:
                     back = color;
                     color = COLOR_NAUTICAL;
                     ld_color = lightdark_nautical;
                     fore = COLOR_BLACK;
                     break;
                  case CAT_SOLAR:
                     back = color;
                     color = COLOR_CIVIL;
                     ld_color = lightdark_civil;
                     fore = COLOR_BLACK;
                     break;
                  case CAT_LUNAR:
                  case CAT_MERCURY:
                  case CAT_VENUS:
                  case CAT_MARS:
                  case CAT_JUPITER:
                  case CAT_SATURN:
                  case CAT_ARIES:
                     // do nothing
                     break;
               }
            }
         }
      }
   }

   double here = now + 0.5;
   double start_angle = (last-base) * 360.0 - up_angle + 270.0;
   double stop_angle = (here-base) * 360.0 - up_angle + 270.0;

   if (!arcd || start_angle != stop_angle) {
      arc_canvas_shaded(canvas,
            canvas->w / 2, canvas->h / 2,
            canvas->w / 2 / 2,
            canvas->h / 2 / 2,
            color, start_angle, stop_angle);
      arc_canvas(canvas, canvas->w/2, canvas->w/2, canvas->w/2 / 2 + SCALE(126), 7, ld_color,
            start_angle, stop_angle);
   }

   // accumulate angle_daylight and angle_darkness
   start_angle = normalize_angle(start_angle);
   stop_angle = normalize_angle(stop_angle);
   while (stop_angle <= start_angle) {
      stop_angle += 360.0;
   }
   switch (color) {
      case COLOR_DAYLIGHT:
         angle_daylight += stop_angle - start_angle;
         break;
      case COLOR_CIVIL:
         angle_civil += stop_angle - start_angle;
         break;
      case COLOR_NAUTICAL:
         angle_nautical += stop_angle - start_angle;
         break;
      case COLOR_ASTRONOMICAL:
         angle_astronomical += stop_angle - start_angle;
         break;
      case COLOR_DARKNESS:
         angle_darkness += stop_angle - start_angle;
         break;
   }

   if (times_written) {
      do_tr_time_sun(context, canvas, now, last, start_angle,
            canvas->w / 3 - SCALE(32), LOCK(fore), COLOR_LOCK); //fore, back);
   }

   if (transited != 0.0) {
      double angle = (transited-base) * 360.0 - up_angle + 270.0;
      do_tr_time_sun(context, canvas, now, transited, angle,
            canvas->w / 3 - SCALE(32), LOCK(transit_fore), COLOR_LOCK); //transit_fore, transit_back);
   }

   if (lightdark == 0x0000) {
      if (angle_daylight > one_minute) {
         accum_helper(context, canvas, angle_daylight, "daylight", 270.0, COLOR_BLACK,
               COLOR_DAYLIGHT);
      }
      else if (angle_civil > one_minute) {
         accum_helper(context, canvas, angle_civil, "civil", 270.0, COLOR_BLACK,
               COLOR_CIVIL);
      }
      else if (angle_nautical > one_minute) {
         accum_helper(context, canvas, angle_nautical, "nautical", 270.0, COLOR_WHITE,
               COLOR_NAUTICAL);
      }
      else if (angle_astronomical > one_minute) {
         accum_helper(context, canvas, angle_astronomical, "astronomical", 270.0,
               COLOR_WHITE, COLOR_ASTRONOMICAL);
      }
      else if (angle_darkness > one_minute) {
         accum_helper(context, canvas, angle_darkness, "darkness", 270.0, COLOR_WHITE,
               COLOR_DARKNESS);
      }

      if (angle_darkness > one_minute) {
         if (angle_astronomical > one_minute) {
            accum_helper(context, canvas, angle_darkness, "darkness", 90.0,
                  COLOR_WHITE, COLOR_DARKNESS);
         }
      }
      else if (angle_astronomical > one_minute) {
         if (angle_nautical > one_minute) {
            accum_helper(context, canvas, angle_astronomical, "astronomical", 90.0,
                  COLOR_WHITE, COLOR_ASTRONOMICAL);
         }
      }
      else if (angle_nautical > one_minute) {
         if (angle_civil > one_minute) {
            accum_helper(context, canvas, angle_nautical, "nautical", 90.0,
                  COLOR_WHITE, COLOR_NAUTICAL);
         }
      }
      else if (angle_civil > one_minute) {
         if (angle_daylight > one_minute) {
            accum_helper(context, canvas, angle_civil, "civil", 90.0, COLOR_BLACK,
                  COLOR_CIVIL);
         }
      }
   }
   else {
      double angle_light = angle_daylight;
      double angle_dark = angle_darkness;

      switch (light) {
         case 3: angle_light += angle_astronomical; // fallthrough
         case 2: angle_light += angle_nautical;     // fallthrough
         case 1: angle_light += angle_civil;        // fallthrough
      }

      switch (dark) {
         case 3: angle_dark += angle_civil;        // fallthrough
         case 2: angle_dark += angle_nautical;     // fallthrough
         case 1: angle_dark += angle_astronomical; // fallthrough
      }

      if (angle_light > one_minute) {
         unsigned int fg = COLOR_GREEN, bg = COLOR_RED;

         if (angle_daylight > one_minute) {
            fg = COLOR_BLACK;
            bg = COLOR_DAYLIGHT;
         }
         else if (angle_civil > one_minute) {
            fg = COLOR_BLACK;
            bg = COLOR_CIVIL;
         }
         else if (angle_nautical > one_minute) {
            fg = COLOR_WHITE;
            bg = COLOR_NAUTICAL;
         }
         else if (angle_astronomical > one_minute) {
            fg = COLOR_WHITE;
            bg = COLOR_ASTRONOMICAL;
         }

         accum_helper(context, canvas, angle_light, "light", 270.0, LOCK(fg), COLOR_LOCK);
      }

      if (angle_dark > one_minute) {
         unsigned int fg = COLOR_GREEN, bg = COLOR_RED;

         if (angle_darkness > one_minute) {
            fg = COLOR_WHITE;
            bg = COLOR_DARKNESS;
         }
         else if (angle_astronomical > one_minute) {
            fg = COLOR_WHITE;
            bg = COLOR_ASTRONOMICAL;
         }
         else if (angle_nautical > one_minute) {
            fg = COLOR_WHITE;
            bg = COLOR_NAUTICAL;
         }
         else if (angle_civil > one_minute) {
            fg = COLOR_BLACK;
            bg = COLOR_CIVIL;
         }

         //accum_helper(context, canvas, angle_dark, "dark", 90.0, fg, bg);
         accum_helper(context, canvas, angle_dark, "dark", 90.0, LOCK(fg), COLOR_LOCK);
      }

   }
}

/// @brief Get a Julian Date for the last New Moon
///
/// @param now The current Julian Date
/// @return The Julian Date for the New Moon preceeding today
double get_lunar_new(double now) {
   double min = ln_get_lunar_disk(now);
   double when = now;

   for (int i = 0; i < 45; i++) {
      double lunar_disk = ln_get_lunar_disk(now + (double)i);
      if (lunar_disk < min) {
         min = lunar_disk;
         when = now + i;
      }
   }

   now = when;

   for (double i = -1.0; i < 1.0; i += (1.0 / 48.0)) {
      double lunar_disk = ln_get_lunar_disk(now + i);
      if (lunar_disk < min) {
         min = lunar_disk;
         when = now + i;
      }
   }

   return when;
}

/// @brief Get the angle at which to draw the moon
///
/// @param JD The current Julian Date
/// @param lunar_new The Julian Date of the last new moon
/// @return An angle representation of the current phase
double get_moon_angle(double JD, double lunar_new) {
   const double synodic_month = 29.530588;      // lunar synodic month
   while (lunar_new > JD) {
      lunar_new -= synodic_month;
   }
   double percent = (JD - lunar_new) / synodic_month;
   double angle = normalize_angle(270.0 + percent * 360.0);
   return angle;
}

/// @brief Draw the perimeter bands for all planets
///
/// @param context Pointer to computation context
/// @param canvas The Canvas to draw on
/// @param JD The current Julian Date
/// @param up The Julian Date used as "up" on the clock
/// @return void
void do_planet_bands(Context *context, Canvas * canvas, double JD, double up) {
   double r = canvas->w / 2 / 2 + SCALE(128 + 16 + 5);

   do_planet_band(context, canvas, up, JD, COLOR_MOONBAND, r, CAT_LUNAR, 0);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_MERCURY, r, CAT_MERCURY, 0);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_VENUS, r, CAT_VENUS, 0);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_MARS, r, CAT_MARS, 0);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_JUPITER, r, CAT_JUPITER, 0);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_SATURN, r, CAT_SATURN, 0);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_ARIES, r, CAT_ARIES, 0);

   r = canvas->w / 2 / 2 + SCALE(128 + 16 + 5);

   do_planet_band(context, canvas, up, JD, COLOR_MOONBAND, r, CAT_LUNAR, 1);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_MERCURY, r, CAT_MERCURY, 1);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_VENUS, r, CAT_VENUS, 1);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_MARS, r, CAT_MARS, 1);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_JUPITER, r, CAT_JUPITER, 1);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_SATURN, r, CAT_SATURN, 1);
   r += SCALE(15);
   do_planet_band(context, canvas, up, JD, COLOR_ARIES, r, CAT_ARIES, 1);
}

/// @brief Draw debugging information
///
/// Timezone in lower left, Julian Date in lower right
///
/// @param canvas The Canvas to draw on
/// @param JD The current Julian Date
/// @param offset The offset passed into do_all
/// @param tzProvider Name of the timezone provider
/// @return void
void do_debug_info(Canvas * canvas, double JD, double offset,
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
   time_t present;
   ln_get_timet_from_julian(JD, &present);
   struct tm *tm = localtime(&present);
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
/// @param provider The current location provider
/// @return void
void do_provider_info(Canvas * canvas, const char *provider) {
   int wh = text_canvas(canvas, FONT_BOLD_MED, -1000, -1000,
         COLOR_WHITE, COLOR_BLACK, provider, 1, 3);
   int w = wh >> 16;
   int h = wh & 0xFFFF;
   text_canvas(canvas, FONT_BOLD_MED, w / 2 + 20,
         h / 2 + 20, COLOR_WHITE, COLOR_BLACK, provider, 1, 3);
}

void initialize_all(Context *context) {
   event_spot = 0;
   timedrawnspot = 0;
   accumdrawnspot = 0;
}

void set_font(uint8_t ** target, uint8_t ** choices, int desire, int width) {
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

void set_astro_font(int width) {
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

void set_bold_big(int width) {
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

void set_bold_med(int width) {
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
   set_font(&FONT_BOLD_MED, choices, djsmb_20_bdf[1], width);
}

void set_bold_small(int width) {
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
   set_font(&FONT_BOLD_SMALL, choices, djsmb_16_bdf[1], width);
}

void set_italic_med(int width) {
   uint8_t *choices[] = {
      djsmo_8_bdf,
      djsmo_10_bdf,
      djsmo_16_bdf,
      djsmo_20_bdf,
      djsmo_32_bdf,
      djsmo_40_bdf,
      djsmo_50_bdf,
      djsmo_60_bdf,
      NULL
   };
   set_font(&FONT_ITALIC_MED, choices, djsmo_20_bdf[1], width);
}

double to_the_minute(double jd) {
   double whole = (double) ((int) jd);
   double fraction = jd - whole;
   fraction *= 24*60;
   fraction = (double)((int) fraction);
   fraction /= 24*60;

   return whole + fraction + (1.0/(24.0*60.0*2.0));
}

/// @brief Do all of the things
///
/// @param lat The observer's Latitude in degrees, South is negative
/// @param lon The observer's Longitude in degrees, West is negative
/// @param offset An offset from the current Julian Date
/// @param width Width of canvas to draw
/// @param provider Name of the location provider to be displayed
/// @param tzprovider Name of the timezone provider to be displayed
/// @param tz Name of timezone to be used
/// @param lightdark controls which regions are considered light and dark
/// @return A canvas that has been drawn upon
Canvas *do_all(double lat, double lon, double offset, int width,
      const char *provider, const char *tzprovider, const char *tz,
      int lightdark) {

   static Context *context = NULL;
   if (context == NULL) {
      context = (Context *)calloc(1, sizeof(Context));
   }

   struct ln_zonedate now;
   struct ln_lnlat_posn observer;

   if (tz != NULL && tz[0] != 0) {
      setenv("TZ", tz, 1);
      tzset();
   }

   double JD;
   double up;

   SIZE = width;
   set_astro_font(width);
   set_bold_big(width);
   set_bold_med(width);
   set_bold_small(width);
   set_italic_med(width);
   /*
#define ASTRO_FONT astro_32_bdf
#define FONT_BOLD_BIG djsmb_50_bdf
#define FONT_BOLD_MED djsmb_20_bdf
#define FONT_BOLD_SMALL djsmb_16_bdf
#define FONT_ITALIC_MED djsmo_20_bdf
    */

   initialize_all(context);

   // observer's location
   observer.lat = lat;          // degrees, North is positive
   observer.lng = lon;          // degrees, East is positive

   // get Julian day from local time
   JD = ln_get_julian_from_sys() + offset;
   if (offset > 2000000.0) {
      JD = offset;
   }

   JD = to_the_minute(JD);

   //printf("JD=%lf\n", JD);

   // local time
   my_get_local_date(JD, &now);

   // all of the data
   events_clear(context);
   events_populate(context, JD, &observer);
   events_sort(context);
   events_uniq(context);
   events_prune(context, JD);
   //events_dump();

   // get the transit time
   up = to_the_minute(events_transit(context, JD)); // rounding added to reduce jitter

   // lunar disk, phase and bright limb
   float lunar_phase = ln_get_lunar_phase(JD);  // 0 to 180
   float lunar_bright_limb = ln_get_lunar_bright_limb(JD);      // 0 to 360

   double lunar_new = get_lunar_new(JD);

   //// drawing begins here
   Canvas *canvas = new_canvas(SIZE, SIZE, COLOR_BLACK);

   int mid = canvas->w / 2;

   int goodloc = 1;
   if (lat > 90.0 || lon > 180.0 || lat < -90.0 || lon < -180.0) {
      goodloc = 0;

      time_t present;
      ln_get_timet_from_julian(JD, &present);
      struct tm *tm;
      tm = localtime(&present);
      int local_hour = tm->tm_hour;
      tm = gmtime(&present);
      int gmt_hour = tm->tm_hour;

      up += ((24 + gmt_hour - local_hour) % 24) / 24.0;
   }

   // colored bands for planets
   if (goodloc) {
      do_planet_bands(context, canvas, JD, up);
   }

   // colored bands for the sun
   if (goodloc) {
      do_sun_bands(context, canvas, up, JD, lightdark);
   }

   // draw accumulated times
   replay_accum_memory(context, canvas);

   // redraw some text, to make things pretty
   replayTimeDrawnMemory(context, canvas);

   // our rotating "now" hand
   do_now_hand(canvas, up, JD);

   // draw the moon
   double moon_angle;
   if (goodloc) {
      moon_angle = get_moon_angle(JD, lunar_new);
      do_moon_draw(context, canvas, JD, lunar_phase, lunar_bright_limb, moon_angle);
   }

   // hour ticks
   do_hour_ticks(canvas, JD, mid, mid, mid / 2 + SCALE(128), up);

   // border bands
   arc_canvas(canvas, mid, mid, mid / 2 - SCALE(128), 1, COLOR_WHITE, 0, 360.0);
   //arc_canvas(canvas, mid, mid, mid / 2 + SCALE(128), 1, COLOR_WHITE, 0, 360.0);

   // information in the center
   do_now_time(canvas, JD);
   do_now_weekday(canvas, JD);
   do_now_date(canvas, JD);
   do_location(canvas, &observer);
   //do_timezone();
   do_now_smalldate(canvas, JD);

   // embedded watch won't have weather info
   //do_weather(canvas);

   do_debug_info(canvas, JD, offset, tzprovider);

   do_provider_info(canvas, provider);

   return canvas;
}

int do_when_is_it(double lat, double lon, int category, int type, int delayMinutes) {
   static Context *context = NULL;
   if (context == NULL) {
      context = (Context *)calloc(1, sizeof(Context));
   }

   struct ln_lnlat_posn observer;
   observer.lat = lat;
   observer.lng = lon;
   double JD = ln_get_julian_from_sys();
   double oJD = JD;

   initialize_all(context);

   for (int halfdays = 0; halfdays < 14; halfdays++) {
      events_populate(context, JD, &observer);
      events_sort(context);
      events_uniq(context);

      for (int i = 0; i < event_spot; i++) {
         if (events[i].category == category && events[i].type == type) {
            double event_jd = events[i].jd + ((double)delayMinutes * ONE_MINUTE_JD);
            if (event_jd > oJD) {
               // in the future
               return (int) ((event_jd - oJD) * 24.0 * 60.0 * 60.0);
            }
         }
      }

      JD += 0.5;
   }

   // hrm, not found...
   return -1;
}

// vim: expandtab:noai:ts=3:sw=3
