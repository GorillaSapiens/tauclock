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
#ifdef ORIGx2
#define SIZE 2048
#define ASTRO_FONT astro_50_bdf
#define FONT_BOLD_BIG djsmb_60_bdf
#define FONT_BOLD_MED djsmb_40_bdf
#define FONT_BOLD_SMALL djsmb_32_bdf
#define FONT_ITALIC_MED djsmo_40_bdf
#endif
#ifdef SMALL
#define SIZE 400
#define ASTRO_FONT astro_24_bdf
#define FONT_BOLD_BIG djsmb_24_bdf
#define FONT_BOLD_MED djsmb_10_bdf
#define FONT_BOLD_SMALL djsmb_8_bdf
#define FONT_ITALIC_MED djsmo_10_bdf
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

typedef enum EventType {
   EVENT_DOWN,
   EVENT_RISE,
   EVENT_TRANSIT,
   EVENT_SET,
   EVENT_UP
} EventType;

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
   CAT_SATURN
} EventCategory;

/// @brief Human readable names for the CAT_* defines
const char *categorynames[] = { "sun", "nautical", "civil", "astronomical",
   "lunar", "mercury", "venus", "mars", "jupiter", "saturn"
};

#define COLOR_MERCURY   COLOR_GRAY
#define COLOR_VENUS     COLOR_WHITE
#define COLOR_MARS      COLOR_RED
#define COLOR_JUPITER   COLOR_ORANGE
#define COLOR_SATURN    COLOR_LIGHTBLUE // RGB(0xea, 0xd6, 0xb8)

/// @brief Human readable names for the EVENT_* defines
const char *typenames[] = { "down", "rise", "transit", "set", "up" };

/// @brief A structure denoting a up/down/rise/transit/set event.
typedef struct {
   double jd;
   EventCategory category;
   EventType type;
   int prune;
} Event;

/// @brief The size of the events array
#define NUM_EVENTS 4096

/// @brief An array containing events
Event events[NUM_EVENTS];

/// @brief An integer denoting the next free slot in the events array
int event_spot = 0;

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

/// @brief Remove newlines in a string.
///
/// This routine is dumb, and simply terminates any string with nonprinting chars.
///
/// @param p Pointer to string
/// @return void, the string is modified in place.
void remove_newlines(char *p) {
   while (*p) {
      if (((unsigned char)*p) < ' ') {
         *p = 0;
         break;
      }
      p++;
   }
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
/// @param canvas The Canvas to draw on
/// @param now The current Julian Date
/// @param lunar_phase The current lunar_phase as returned by libnova
/// @param lunar_bright_limb The current lunar_bright_limb as returned by libnova
/// @param where_angle The clock angle at which to draw the moon
/// @return void
void
do_moon_draw(Canvas * canvas,
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

   // outline
   arc_canvas(canvas, cx, cy, SCALE(40), 1, COLOR_DARKGRAY, 0, 360.0);

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
   if (is_up == 1) {
      arc_canvas(canvas, cx, cy, SCALE(40), 1, COLOR_MOONBAND, 0, 360.0);
      arc_canvas(canvas, cx, cy, SCALE(43), 1, COLOR_MOONBAND, 0, 360.0);
   }
}

/// @brief Draw the perimeter planet band
///
/// @param canvas The Canvas to draw on
/// @param up The Julian Date used as "up" on the clock
/// @param now The current Julian Date
/// @param color The color to use when drawing
/// @param radius The radius at which to draw the band
/// @param category The planet category
/// @param mode 1=draw band, 2=draw tick, 3=draw both
/// @return void
void
do_planet_band(Canvas * canvas, double up, double now,
      unsigned int color, double radius, EventCategory category,
      int mode) {
   double up_angle = frac(up) * 360.0;
   int need_character = mode & 2;       // draw characters at ticks

   bool arcd = false;

   char sym[2] = { 0, 0 };
   sym[0] = 'B' + (category - CAT_LUNAR);

   double last = now - .5;
   int is_up = -1;
   for (int i = 0; i < event_spot; i++) {
      if (events[i].jd > now + .5) {
         break;
      }
      if (events[i].category == category) {
         switch (events[i].type) {
            case EVENT_UP:
            case EVENT_RISE:
            case EVENT_TRANSIT:
               if (is_up == 0) {
                  if (events[i].jd > last) {
                     last = events[i].jd;
                  }
               }
               is_up = 1;
               break;
            case EVENT_DOWN:
            case EVENT_SET:
               if (events[i].jd > last && is_up == 1) {
                  double start_angle = frac(last) * 360.0 - up_angle + 270.0;
                  double stop_angle =
                     frac(events[i].jd) * 360.0 - up_angle + 270.0;
                  if (mode & 1) {
                     arcd = true;
                     arc_canvas_shaded(canvas, canvas->w / 2, canvas->h / 2,
                           radius, SCALE(5), color, start_angle,
                           stop_angle);
                  }

                  last = events[i].jd;
               }
               is_up = 0;
               break;
         }
      }
   }
   if (is_up == 1) {
      double start_angle = frac(last) * 360.0 - up_angle + 270.0;
      double stop_angle = frac(now + .5) * 360.0 - up_angle + 270.0;
      if (mode & 1) {
         if (!arcd || start_angle != stop_angle) {
            arc_canvas_shaded(canvas, canvas->w / 2, canvas->h / 2,
                  radius, SCALE(5), color, start_angle, stop_angle);
         }
      }
   }

   double transit_x = 0.0;
   double transit_y = 0.0;

   for (int i = 0; i < event_spot; i++) {
      if (events[i].category == category) {
         if (!events[i].prune) {
            switch (events[i].type) {
               case EVENT_UP:
               case EVENT_DOWN:
                  // do nothing
                  break;
               case EVENT_RISE:
                  {
                     double x, y;
                     double angle =
                        frac(events[i].jd) * 360.0 - up_angle + 270.0;

                     x = (canvas->w / 2) + radius * cos(DEG2RAD(angle - 3));
                     y = (canvas->h / 2) + radius * sin(DEG2RAD(angle - 3));
                     text_canvas(canvas, ASTRO_FONT, x, y, color,
                           COLOR_BLACK, sym, 1, 1);
                     need_character = 0;
                  }
                  // fallthrough
               case EVENT_TRANSIT:
               case EVENT_SET:
                  {
                     double x1, y1, x2, y2;
                     double angle =
                        frac(events[i].jd) * 360.0 - up_angle + 270.0;

                     x1 = canvas->w / 2 + (radius - 16) * cos(DEG2RAD(angle));
                     y1 = canvas->h / 2 + (radius - 16) * sin(DEG2RAD(angle));
                     x2 = canvas->w / 2 + (radius + 16) * cos(DEG2RAD(angle));
                     y2 = canvas->h / 2 + (radius + 16) * sin(DEG2RAD(angle));

                     double x =
                        (canvas->w / 2) + radius * cos(DEG2RAD(angle + 3));
                     double y =
                        (canvas->h / 2) + radius * sin(DEG2RAD(angle + 3));

                     if (mode & 2) {
                        line_canvas(canvas, x1, y1, x2, y2, color);
                     }
                     if (events[i].type == EVENT_SET) {
                        text_canvas(canvas, ASTRO_FONT, x, y, color,
                              COLOR_BLACK, sym, 1, 1);
                        need_character = 0;
                     }
                     else {
                        transit_x = x;
                        transit_y = y;
                     }
                  }
                  break;
            }
         }
      }
   }
   if (need_character && is_up) {
      // shazbat!
      double x =
         (canvas->w / 2) + (radius +
               15) * cos(DEG2RAD((int)(category - CAT_LUNAR) *
                     (360 / 6)));
      double y =
         (canvas->h / 2) + (radius +
               15) * sin(DEG2RAD((int)(category - CAT_LUNAR) *
                     (360 / 6)));
      if (transit_x != 0.0) {
         x = transit_x;
         y = transit_y;
      }
      text_canvas(canvas, ASTRO_FONT, x, y, color, COLOR_BLACK, sym, 1, 1);
   }
}

/// @brief Clear the events list
void events_clear(void) {
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
         CAT_SATURN
      };

      static const EventCategory order_down[] = {       //"lscna"
         CAT_LUNAR,
         CAT_MERCURY,
         CAT_VENUS,
         CAT_MARS,
         CAT_JUPITER,
         CAT_SATURN,
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
void events_sort(void) {
   qsort(events, event_spot, sizeof(Event), event_compar);
}

/// @brief Uniq the events list
///
/// Uses the event_compar function
void events_uniq(void) {
   int i = 0;
   while (i < event_spot - 1) {
      if (!event_compar(events + i, events + (i + 1))) {
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
/// @param jd The curren Julian Date
/// @return void
void events_prune(double jd) {
   for (int i = 0; i < event_spot; i++) {
      events[i].prune = (fabs(events[i].jd - jd) > 0.5) ? 1 : 0;
   }
}

/// @brief For debugging, print the events list
void events_dump(void) {
   struct ln_zonedate zonedate;

   for (int i = 0; i < event_spot; i++) {
      my_get_local_date(events[i].jd, &zonedate);
      printf("%6.12f %02d:%02d %d %s %s\n", events[i].jd,
            zonedate.hours, zonedate.minutes,
            events[i].prune, typenames[events[i].type],
            categorynames[events[i].category]);
   }
}

/// @brief One minute of Julian Date.
#define ONE_MINUTE_JD (1.0/(24.0*60.0))

/// @brief One half minute of Julian Date.
#define HALF_MINUTE_JD (1.0/(24.0*60.0*2.0))

/// @brief Ten seconds of Julian Date.
#define TEN_SECONDS_JD (1.0/(24.0*60.0*6.0))

/// @brief Typedef for functions returning Equ coordinates of object
typedef void (*Get_Equ_Coords)(double, struct ln_equ_posn *);

void events_populate_anything_updown(double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      double horizon, EventCategory category) {
   struct ln_equ_posn posn;
   struct ln_hrz_posn hrz_posn;
   double angle;

   JD -= 2.0 + HALF_MINUTE_JD;

   get_equ_coords(JD, &posn);
   ln_get_hrz_from_equ(&posn,
         observer,
         JD, &hrz_posn);
   angle = hrz_posn.alt;

   if (angle < horizon) {
      events[event_spot++] = (Event) { JD, category, EVENT_DOWN};
   }
   else {
      events[event_spot++] = (Event) { JD, category, EVENT_UP};
   }
}

#define CLOSEENOUGH(a,b) (fabs((a)-(b)) < (ONE_MINUTE_JD * 1.5))
#define WITHINHALF(a,b) (fabs((a)-(b)) < 0.5)

void events_populate_anything_rst_firstpass(double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      double horizon, EventCategory category,
      Event *tentative,
      int *tent_spotp) {
   struct ln_equ_posn posn;
   struct ln_hrz_posn hrz_posn;
   double angles[3];

   get_equ_coords(JD, &posn);

   ln_get_hrz_from_equ(&posn,
         observer,
         JD - 2.0 - ONE_MINUTE_JD, &hrz_posn);
   angles[1] = hrz_posn.alt;

   ln_get_hrz_from_equ(&posn,
         observer,
         JD - 2.0 - 2.0 * ONE_MINUTE_JD, &hrz_posn);
   angles[2] = hrz_posn.alt;

   for (int i = 0; i < (1440*3)+10; i++) {
      double when = JD - 2.0 + (double) i / 1440.0;
      ln_get_hrz_from_equ(&posn, observer, when, &hrz_posn);
      angles[0] = hrz_posn.alt;

      if (angles[1] < horizon && angles[0] >= horizon) {
         tentative[(*tent_spotp)++] = (Event) { when, category, EVENT_RISE };
      }
      if (angles[1] > angles[0] && angles[1] > angles[2]) {
         if (angles[1] > horizon || category == CAT_SOLAR) {
            tentative[(*tent_spotp)++] = (Event) { when - ONE_MINUTE_JD, category, EVENT_TRANSIT };
         }
      }
      if (angles[0] < horizon && angles[1] >= horizon) {
         tentative[(*tent_spotp)++] = (Event) { when, category, EVENT_SET };
      }
      angles[2] = angles[1];
      angles[1] = angles[0];
   }
}

void events_populate_anything_rst_secondpass(double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      double horizon, EventCategory category,
      Event *tentative,
      int *tent_spotp) {
   struct ln_equ_posn posn;
   struct ln_hrz_posn hrz_posn;
   double angles[3];

   for (int j = 0; j < *tent_spotp; j++) {
again:
      get_equ_coords(tentative[j].jd, &posn);

      ln_get_hrz_from_equ(&posn,
            observer,
            JD - 2.0 - ONE_MINUTE_JD, &hrz_posn);
      angles[1] = hrz_posn.alt;

      ln_get_hrz_from_equ(&posn,
            observer,
            JD - 2.0 - 2.0 * ONE_MINUTE_JD, &hrz_posn);
      angles[2] = hrz_posn.alt;

      for (int i = 0; i < (1440*3)+10; i++) {
         double when = JD - 2.0 + (double) i / 1440.0;
         ln_get_hrz_from_equ(&posn, observer, when, &hrz_posn);
         angles[0] = hrz_posn.alt;

         if (tentative[j].type == EVENT_RISE) {
            if (angles[1] < horizon && angles[0] >= horizon) {
               if (WITHINHALF(when, tentative[j].jd)) {
                  if (CLOSEENOUGH(when, tentative[j].jd)) {
                     goto carry_on;
                  }
                  else {
                     tentative[j].jd = when;
                     goto again;
                  }
               }
            }
         }
         if (tentative[j].type == EVENT_TRANSIT) {
            if (angles[1] > angles[0] && angles[1] > angles[2]) {
               if (angles[1] > horizon || category == CAT_SOLAR) {
                  if (WITHINHALF(when, tentative[j].jd)) {
                     if (CLOSEENOUGH(when, tentative[j].jd)) {
                        goto carry_on;
                     }
                     else {
                        tentative[j].jd = when;
                        goto again;
                     }
                  }
               }
            }
         }
         if (tentative[j].type == EVENT_SET) {
            if (angles[0] < horizon && angles[1] >= horizon) {
               if (WITHINHALF(when, tentative[j].jd)) {
                  if (CLOSEENOUGH(when, tentative[j].jd)) {
                     goto carry_on;
                  }
                  else {
                     tentative[j].jd = when;
                     goto again;
                  }
               }
            }
         }
         angles[2] = angles[1];
         angles[1] = angles[0];
      }
carry_on:;
   }
}

void events_populate_anything_rst_thirdpass(double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      double horizon, EventCategory category,
      Event *tentative,
      int *tent_spotp) {
   struct ln_equ_posn posn;
   struct ln_hrz_posn hrz_posn;
   double angles[3];

   for (int i = 0; i < *tent_spotp; i++) {
      if (tentative[i].type == EVENT_RISE) {
         get_equ_coords(tentative[i].jd - TEN_SECONDS_JD, &posn);
         ln_get_hrz_from_equ(&posn,
             observer,
             tentative[i].jd - TEN_SECONDS_JD, &hrz_posn);
         angles[1] = hrz_posn.alt;

         get_equ_coords(tentative[i].jd, &posn);
         ln_get_hrz_from_equ(&posn,
             observer,
             tentative[i].jd, &hrz_posn);
         angles[0] = hrz_posn.alt;

         while (angles[0] >= horizon && angles[1] >= horizon) {
            angles[0] = angles[1];
            tentative[i].jd -= TEN_SECONDS_JD;

            get_equ_coords(tentative[i].jd - TEN_SECONDS_JD, &posn);
            ln_get_hrz_from_equ(&posn,
                  observer,
                  tentative[i].jd - TEN_SECONDS_JD, &hrz_posn);
            angles[1] = hrz_posn.alt;
         }
         while (angles[0] < horizon && angles[1] < horizon) {
            angles[1] = angles[0];
            tentative[i].jd += TEN_SECONDS_JD;

            get_equ_coords(tentative[i].jd, &posn);
            ln_get_hrz_from_equ(&posn,
                  observer,
                  tentative[i].jd, &hrz_posn);
            angles[0] = hrz_posn.alt;
         }
      }
      else if (tentative[i].type == EVENT_TRANSIT) {
         get_equ_coords(tentative[i].jd - TEN_SECONDS_JD, &posn);
         ln_get_hrz_from_equ(&posn,
             observer,
             tentative[i].jd - TEN_SECONDS_JD, &hrz_posn);
         angles[2] = hrz_posn.alt;

         get_equ_coords(tentative[i].jd, &posn);
         ln_get_hrz_from_equ(&posn,
             observer,
             tentative[i].jd, &hrz_posn);
         angles[1] = hrz_posn.alt;

         get_equ_coords(tentative[i].jd + TEN_SECONDS_JD, &posn);
         ln_get_hrz_from_equ(&posn,
             observer,
             tentative[i].jd + TEN_SECONDS_JD, &hrz_posn);
         angles[0] = hrz_posn.alt;

         while (angles[2] > angles[1]) {
            angles[0] = angles[1];
            angles[1] = angles[2];
            tentative[i].jd -= TEN_SECONDS_JD;

            get_equ_coords(tentative[i].jd - TEN_SECONDS_JD, &posn);
            ln_get_hrz_from_equ(&posn,
                  observer,
                  tentative[i].jd - TEN_SECONDS_JD, &hrz_posn);
            angles[2] = hrz_posn.alt;
         }
         while (angles[0] > angles[1]) {
            angles[2] = angles[1];
            angles[1] = angles[0];
            tentative[i].jd += TEN_SECONDS_JD;

            get_equ_coords(tentative[i].jd, &posn);
            ln_get_hrz_from_equ(&posn,
                  observer,
                  tentative[i].jd, &hrz_posn);
            angles[0] = hrz_posn.alt;
         }
      }
      else if (tentative[i].type == EVENT_SET) {
         get_equ_coords(tentative[i].jd - TEN_SECONDS_JD, &posn);
         ln_get_hrz_from_equ(&posn,
             observer,
             tentative[i].jd - TEN_SECONDS_JD, &hrz_posn);
         angles[1] = hrz_posn.alt;

         get_equ_coords(tentative[i].jd, &posn);
         ln_get_hrz_from_equ(&posn,
             observer,
             tentative[i].jd, &hrz_posn);
         angles[0] = hrz_posn.alt;

         while (angles[0] < horizon && angles[1] < horizon) {
            angles[0] = angles[1];
            tentative[i].jd -= TEN_SECONDS_JD;

            get_equ_coords(tentative[i].jd - TEN_SECONDS_JD, &posn);
            ln_get_hrz_from_equ(&posn,
                  observer,
                  tentative[i].jd - TEN_SECONDS_JD, &hrz_posn);
            angles[1] = hrz_posn.alt;
         }
         while (angles[0] >= horizon && angles[1] >= horizon) {
            angles[1] = angles[0];
            tentative[i].jd += TEN_SECONDS_JD;

            get_equ_coords(tentative[i].jd, &posn);
            ln_get_hrz_from_equ(&posn,
                  observer,
                  tentative[i].jd, &hrz_posn);
            angles[0] = hrz_posn.alt;
         }
      }
   }
}

void events_populate_anything_rst(double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      double horizon, EventCategory category) {
   Event tentative[64];
   int tent_spot = 0;

   events_populate_anything_rst_firstpass(JD,
                                          observer,
                                          get_equ_coords,
                                          horizon,
                                          category,
                                          tentative,
                                          &tent_spot);

   events_populate_anything_rst_secondpass(JD,
                                           observer,
                                           get_equ_coords,
                                           horizon,
                                           category,
                                           tentative,
                                           &tent_spot);

   events_populate_anything_rst_thirdpass(JD,
                                          observer,
                                          get_equ_coords,
                                          horizon,
                                          category,
                                          tentative,
                                          &tent_spot);

   for (int i = 0; i < tent_spot; i++) {
      events[event_spot++] = tentative[i];
   }
}

void events_populate_anything(double JD,
      struct ln_lnlat_posn *observer,
      Get_Equ_Coords get_equ_coords,
      double horizon, EventCategory category) {

   events_populate_anything_updown(JD, observer, get_equ_coords, horizon, category);
   events_populate_anything_rst(JD, observer, get_equ_coords, horizon, category);
}

void events_populate_solar(double JD, struct ln_lnlat_posn *observer) {
   events_populate_anything(JD, observer, ln_get_solar_equ_coords,
         LN_SOLAR_ASTRONOMICAL_HORIZON, CAT_ASTRONOMICAL);
   events_populate_anything(JD, observer, ln_get_solar_equ_coords,
         LN_SOLAR_NAUTIC_HORIZON, CAT_NAUTICAL);
   events_populate_anything(JD, observer, ln_get_solar_equ_coords,
         LN_SOLAR_CIVIL_HORIZON, CAT_CIVIL);
   events_populate_anything(JD, observer, ln_get_solar_equ_coords,
         LN_SOLAR_STANDART_HORIZON, CAT_SOLAR);
}

void events_populate_lunar(double JD, struct ln_lnlat_posn *observer) {
   events_populate_anything(JD, observer, ln_get_lunar_equ_coords,
         LN_LUNAR_STANDART_HORIZON, CAT_LUNAR);
}

void events_populate_planets(double JD, struct ln_lnlat_posn *observer) {
   events_populate_anything(JD, observer,
         ln_get_mercury_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_MERCURY);
   events_populate_anything(JD, observer,
         ln_get_venus_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_VENUS);
   events_populate_anything(JD, observer,
         ln_get_mars_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_MARS);
   events_populate_anything(JD, observer,
         ln_get_jupiter_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_JUPITER);
   events_populate_anything(JD, observer,
         ln_get_saturn_equ_coords, LN_STAR_STANDART_HORIZON,
         CAT_SATURN);
}

/// @brief Populate the event list
///
/// @param JD The current Julian Date
/// @param observer The observer's lat/lon coordinates
/// @return void
void events_populate(double JD, struct ln_lnlat_posn *observer) {
   events_populate_solar(JD, observer);
   events_populate_lunar(JD, observer);
   events_populate_planets(JD, observer);
}

/// @brief Figure out which way is "up"
///
/// @param jd The current Julian Date
/// @return A Julian Date useful as "up", usually a solar transit
double events_transit(double jd) {
   // any non pruned, non lunar, will do...
   for (int i = 0; i < event_spot; i++) {
      if (!events[i].prune &&
            events[i].type == EVENT_TRANSIT && events[i].category < CAT_LUNAR) {
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

#if 0
/// @brief Read the weather.txt file, and add details to the clock
///
/// This is dead code.
///
/// @param canvas The Canvas to draw on
/// @return void
void do_weather(Canvas * canvas) {
   FILE *f = fopen("weather.txt", "r");
   if (f) {
      char buffer[128];
      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         //location

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // desc
      remove_newlines(buffer);
      text_canvas(canvas, FONT_BOLD_MED, SIZE / 2, SIZE / 2 / 2 + SCALE(64),
            COLOR_BLACK, COLOR_YELLOW, buffer, 1, 3);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // icon
      remove_newlines(buffer);
      buffer[1] = 0;
      text_canvas(canvas, icons_128x128, SIZE / 2, SIZE / 2 / 2, COLOR_BLACK,
            COLOR_YELLOW, buffer, 1, 1);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // temp
      remove_newlines(buffer);
      text_canvas(canvas, FONT_BOLD_MED, SIZE / 2 - SCALE(104), SIZE / 2 / 2,
            COLOR_BLACK, COLOR_YELLOW, buffer, 1, 3);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // wind
      remove_newlines(buffer);
      text_canvas(canvas, FONT_BOLD_MED, SIZE / 2 + SCALE(128), SIZE / 2 / 2,
            COLOR_BLACK, COLOR_YELLOW, buffer, 1, 3);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // desc
      remove_newlines(buffer);
      text_canvas(canvas, FONT_BOLD_MED, SIZE / 2, SIZE * 3 / 4 + SCALE(64),
            COLOR_WHITE, COLOR_DARKBLUE, buffer, 1, 3);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // icon
      remove_newlines(buffer);
      buffer[1] = 0;
      text_canvas(canvas, icons_128x128, SIZE / 2, SIZE * 3 / 4, COLOR_WHITE,
            COLOR_DARKBLUE, buffer, 1, 1);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // temp
      remove_newlines(buffer);
      text_canvas(canvas, FONT_BOLD_MED, SIZE / 2 - 104, SIZE * 3 / 4,
            COLOR_WHITE, COLOR_DARKBLUE, buffer, 1, 3);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // wind
      remove_newlines(buffer);
      text_canvas(canvas, FONT_BOLD_MED, SIZE / 2 + 128, SIZE * 3 / 4,
            COLOR_WHITE, COLOR_DARKBLUE, buffer, 1, 3);

      fclose(f);
   }
}
#endif

/// @brief A struct used to remember where something is drawn.
typedef struct AccumDrawnMemory {
   int x;
   int y;
   unsigned int fg;
   unsigned int bg;
   char *str;
} AccumDrawnMemory;

/// @brief An array of things drawn
AccumDrawnMemory accumdrawnmemory[8];

/// @brief The next open spot in the array
int accumdrawnspot = 0;

/// @brief Redraw stored accumulator times
///
/// This fixes some drawing oddities on very small screens
///
/// @param canvas Pointer to the canvas object
/// @returns void
void replay_accum_memory(Canvas * canvas) {
   for (int i = 0; i < accumdrawnspot; i++) {
      text_canvas(canvas, FONT_BOLD_MED, accumdrawnmemory[i].x,
            accumdrawnmemory[i].y, accumdrawnmemory[i].fg,
            accumdrawnmemory[i].bg, accumdrawnmemory[i].str, 1, 3);
      free(accumdrawnmemory[i].str);
   }
   accumdrawnspot = 0;
}

/// @brief Helper function to accumulate total sunlight/night/whatever hours
///
/// @param canvas The Canvas to draw on
/// @param angle The angle used to derive total time
/// @param label The string label to draw
/// @param draw_angle Where to draw it
/// @param fore The foreground color
/// @param back The background color
/// @return void
void
accum_helper(Canvas * canvas,
      double angle,
      char *label,
      double draw_angle, unsigned int fore, unsigned int back) {

   angle = angle * 24.0 / 360.0;
   int hours = (int)angle;
   int minutes = (angle - (double)hours) * 60.0;
   char buffer[64];
   if (minutes != 0) {
      if (hours != 0) {
         sprintf(buffer, "%dh %dm", hours, minutes);
      }
      else {
         sprintf(buffer, "%dm", minutes);
      }
   }
   else {
      sprintf(buffer, "%dh", hours);
   }
   double x = (canvas->w / 2) + (canvas->w * 4 / 16) * cos(DEG2RAD(draw_angle));
   double y = (canvas->h / 2) + (canvas->h * 4 / 16) * sin(DEG2RAD(draw_angle));
   //   text_canvas(canvas, FONT_BOLD_MED, x, y - SCALE(16), fore, back, buffer, 1, 3);
   //   text_canvas(canvas, FONT_BOLD_MED, x, y + SCALE(16), fore, back, label, 1, 3);
   accumdrawnmemory[accumdrawnspot++] = (AccumDrawnMemory) {
      x, y - SCALE(16), fore, back, strdup(buffer)};
   accumdrawnmemory[accumdrawnspot++] = (AccumDrawnMemory) {
      x, y + SCALE(16), fore, back, strdup(label)};
}

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

/// @brief An array of things drawn
TimeDrawnMemory timedrawnmemory[32];

/// @brief The next open spot in the array
int timedrawnspot = 0;

/// @brief A helper function toe determine when labels collide
///
/// @param x X coordinate of desired label
/// @param y Y coordinate of desired label
/// @param w Width of desired label
/// @param h height of desired label
/// @return 1 if there is a collision, 0 otherwise
int check(int x, int y, int w, int h) {
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
/// @param canvas The Canvas to draw on
/// @param now The current Julian Date
/// @param jd The Julian Date for the event of interest
/// @param theta The desired angle
/// @param radius The desired radius
/// @param fg The foreground color for text
/// @param bg The background color for text
/// @return void
void
do_tr_time_sun(Canvas * canvas, double now, double jd, double theta,
      double radius, unsigned int fg, unsigned int bg) {

   // get width and height by drawing offscreen
   int wh = do_xy_time(canvas, now, jd, -300, -300, fg, bg);
   int w = wh >> 16;
   int h = wh & 0xFFFF;

   int collision;
   int x, y;

   do {
      x = (canvas->w / 2) + radius * cos(DEG2RAD(theta));
      y = (canvas->h / 2) + radius * sin(DEG2RAD(theta));

      collision = check(x, y, w, h);

      radius--;
   }
   while (collision);

   timedrawnmemory[timedrawnspot++] = (TimeDrawnMemory) {
      now, jd, x, y, w, h, fg, bg};

   // don't draw here, we'll get it when we replay
   // do_xy_time(canvas, now, jd, x, y, fg, bg);
}

/// @brief Redraw stored sun rise/set times
///
/// This fixes some drawing oddities on very small screens
///
/// @param canvas Pointer to the canvas object
/// @returns void
void replayTimeDrawnMemory(Canvas * canvas) {
   for (int i = 0; i < timedrawnspot; i++) {
      do_xy_time(canvas,
            timedrawnmemory[i].now,
            timedrawnmemory[i].jd,
            timedrawnmemory[i].x,
            timedrawnmemory[i].y,
            timedrawnmemory[i].fg, timedrawnmemory[i].bg);
   }
}

/// @brief Draw colored bands for solar position
///
/// @param canvas The Canvas to draw on
/// @param up The Julian Date for "up"
/// @param now The Julian Date for the current time
/// @return void
void do_sun_bands(Canvas * canvas, double up, double now) {
   static const double one_minute = 360.0 / 24.0 / 60.0;
   double last = now - 0.5;
   unsigned int color = COLOR_DARKBLUE;
   unsigned int fore = COLOR_WHITE;
   unsigned int back = COLOR_DARKBLUE;

   unsigned int transit_fore = COLOR_GREEN;
   unsigned int transit_back = COLOR_RED;

   double angle_daylight = 0.0;
   double angle_civil = 0.0;
   double angle_nautical = 0.0;
   double angle_astronomical = 0.0;
   double angle_darkness = 0.0;

   double up_angle = frac(up) * 360.0;

   int started = 0;
   double transited = 0.0;

   int times_written = 0;

   bool arcd = false;

   // big, messy state machine...
   for (int i = 0; i < event_spot; i++) {
      if (events[i].category < CAT_LUNAR) {
         if (events[i].type != EVENT_TRANSIT) {
            if (events[i].prune == 0) {
               double here = events[i].jd;
               double start_angle = frac(last) * 360.0 - up_angle + 270.0;
               double stop_angle = frac(here) * 360.0 - up_angle + 270.0;

               arcd = true;
               arc_canvas_shaded(canvas, canvas->w / 2,
                     canvas->h / 2,
                     canvas->w / 2 / 2,
                     canvas->h / 2 / 2, color, start_angle,
                     stop_angle);

               // accumulate angle_daylight and angle_darkness
               start_angle = normalize_angle(start_angle);
               stop_angle = normalize_angle(stop_angle);
               while (stop_angle < start_angle) {
                  stop_angle += 360.0;
               }
               switch (color) {
                  case COLOR_YELLOW:
                     angle_daylight += stop_angle - start_angle;
                     break;
                  case COLOR_ORANGE:
                     angle_civil += stop_angle - start_angle;
                     break;
                  case COLOR_LIGHTBLUE:
                     angle_nautical += stop_angle - start_angle;
                     break;
                  case COLOR_BLUE:
                     angle_astronomical += stop_angle - start_angle;
                     break;
                  case COLOR_DARKBLUE:
                     angle_darkness += stop_angle - start_angle;
                     break;
               }

               if (started) {
                  times_written++;
                  // TODO FIX check size
                  do_tr_time_sun(canvas, now, last, start_angle,
                        (canvas->w / 3 - SCALE(32)), fore, back);
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

         if (events[i].type == EVENT_UP || events[i].type == EVENT_RISE) {
            switch (events[i].category) {
               case CAT_ASTRONOMICAL:
                  color = back = COLOR_BLUE;
                  fore = COLOR_WHITE;
                  break;
               case CAT_NAUTICAL:
                  color = back = COLOR_LIGHTBLUE;
                  fore = COLOR_WHITE;
                  break;
               case CAT_CIVIL:
                  color = back = COLOR_ORANGE;
                  fore = COLOR_BLACK;
                  break;
               case CAT_SOLAR:
                  color = back = COLOR_YELLOW;
                  fore = COLOR_BLACK;
                  break;
               case CAT_LUNAR:
               case CAT_MERCURY:
               case CAT_VENUS:
               case CAT_MARS:
               case CAT_JUPITER:
               case CAT_SATURN:
                  // do nothing
                  break;
            }
         }

         if (events[i].type == EVENT_SET) {
            switch (events[i].category) {
               case CAT_ASTRONOMICAL:
                  back = color;
                  color = COLOR_DARKBLUE;
                  fore = COLOR_WHITE;
                  break;
               case CAT_NAUTICAL:
                  back = color;
                  color = COLOR_BLUE;
                  fore = COLOR_WHITE;
                  break;
               case CAT_CIVIL:
                  back = color;
                  color = COLOR_LIGHTBLUE;
                  fore = COLOR_BLACK;
                  break;
               case CAT_SOLAR:
                  back = color;
                  color = COLOR_ORANGE;
                  fore = COLOR_BLACK;
                  break;
               case CAT_LUNAR:
               case CAT_MERCURY:
               case CAT_VENUS:
               case CAT_MARS:
               case CAT_JUPITER:
               case CAT_SATURN:
                  // do nothing
                  break;
            }
         }
      }
   }

   double here = now + 0.5;
   double start_angle = frac(last) * 360.0 - up_angle + 270.0;
   double stop_angle = frac(here) * 360.0 - up_angle + 270.0;

   if (!arcd || start_angle != stop_angle) {
      arc_canvas_shaded(canvas,
            canvas->w / 2, canvas->h / 2,
            canvas->w / 2 / 2,
            canvas->h / 2 / 2,
            color, start_angle, stop_angle);
   }

   // accumulate angle_daylight and angle_darkness
   start_angle = normalize_angle(start_angle);
   stop_angle = normalize_angle(stop_angle);
   while (stop_angle <= start_angle) {
      stop_angle += 360.0;
   }
   switch (color) {
      case COLOR_YELLOW:
         angle_daylight += stop_angle - start_angle;
         break;
      case COLOR_ORANGE:
         angle_civil += stop_angle - start_angle;
         break;
      case COLOR_LIGHTBLUE:
         angle_nautical += stop_angle - start_angle;
         break;
      case COLOR_BLUE:
         angle_astronomical += stop_angle - start_angle;
         break;
      case COLOR_DARKBLUE:
         angle_darkness += stop_angle - start_angle;
         break;
   }

   if (times_written) {
      do_tr_time_sun(canvas, now, last, start_angle,
            canvas->w / 3 - SCALE(32), fore, back);
   }

   if (transited != 0.0) {
      double angle = frac(transited) * 360.0 - up_angle + 270.0;
      do_tr_time_sun(canvas, now, transited, angle,
            canvas->w / 3 - SCALE(32), transit_fore, transit_back);
   }

   if (angle_daylight > one_minute) {
      accum_helper(canvas, angle_daylight, "daylight", 270.0, COLOR_BLACK,
            COLOR_YELLOW);
   }
   else if (angle_civil > one_minute) {
      accum_helper(canvas, angle_civil, "civil", 270.0, COLOR_BLACK,
            COLOR_ORANGE);
   }
   else if (angle_nautical > one_minute) {
      accum_helper(canvas, angle_nautical, "nautical", 270.0, COLOR_WHITE,
            COLOR_LIGHTBLUE);
   }
   else if (angle_astronomical > one_minute) {
      accum_helper(canvas, angle_astronomical, "astronomical", 270.0,
            COLOR_WHITE, COLOR_BLUE);
   }
   else if (angle_darkness > one_minute) {
      accum_helper(canvas, angle_darkness, "darkness", 270.0, COLOR_WHITE,
            COLOR_DARKBLUE);
   }

   if (angle_darkness > one_minute) {
      if (angle_astronomical > one_minute) {
         accum_helper(canvas, angle_darkness, "darkness", 90.0,
               COLOR_WHITE, COLOR_DARKBLUE);
      }
   }
   else if (angle_astronomical > one_minute) {
      if (angle_nautical > one_minute) {
         accum_helper(canvas, angle_astronomical, "astronomical", 90.0,
               COLOR_WHITE, COLOR_BLUE);
      }
   }
   else if (angle_nautical > one_minute) {
      if (angle_civil > one_minute) {
         accum_helper(canvas, angle_nautical, "nautical", 90.0,
               COLOR_WHITE, COLOR_LIGHTBLUE);
      }
   }
   else if (angle_civil > one_minute) {
      if (angle_daylight > one_minute) {
         accum_helper(canvas, angle_civil, "civil", 90.0, COLOR_BLACK,
               COLOR_ORANGE);
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
/// @param canvas The Canvas to draw on
/// @param JD The current Julian Date
/// @param up The Julian Date used as "up" on the clock
/// @return void
void do_planet_bands(Canvas * canvas, double JD, double up) {
   double r = canvas->w / 2 / 2 + SCALE(128 + 16 + 5);

   do_planet_band(canvas, up, JD, COLOR_MOONBAND, r, CAT_LUNAR, 1);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_MERCURY, r, CAT_MERCURY, 1);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_VENUS, r, CAT_VENUS, 1);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_MARS, r, CAT_MARS, 1);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_JUPITER, r, CAT_JUPITER, 1);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_SATURN, r, CAT_SATURN, 1);

   r -= SCALE(100);
   do_planet_band(canvas, up, JD, COLOR_MOONBAND, r, CAT_LUNAR, 2);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_MERCURY, r, CAT_MERCURY, 2);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_VENUS, r, CAT_VENUS, 2);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_MARS, r, CAT_MARS, 2);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_JUPITER, r, CAT_JUPITER, 2);
   r += SCALE(20);
   do_planet_band(canvas, up, JD, COLOR_SATURN, r, CAT_SATURN, 2);
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

void initialize_all(void) {
   event_spot = 0;
   timedrawnspot = 0;
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

   return whole + fraction;
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
/// @return A canvas that has been drawn upon
Canvas *do_all(double lat, double lon, double offset, int width,
      const char *provider, const char *tzprovider, const char *tz) {
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

   initialize_all();

   // observer's location
   observer.lat = lat;          // degrees, North is positive
   observer.lng = lon;          // degrees, East is positive

   // get Julian day from local time
   JD = ln_get_julian_from_sys() + offset;
   if (offset > 2000000.0) {
      JD = offset;
   }

   JD = to_the_minute(JD);

   printf("JD=%lf\n", JD);

   // local time
   my_get_local_date(JD, &now);

   // all of the data
   events_clear();
   events_populate(JD, &observer);
   events_sort();
   events_uniq();
   events_prune(JD);
   events_dump();

   // get the transit time
   up = events_transit(JD);

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
      do_planet_bands(canvas, JD, up);
   }

   // colored bands for the sun
   if (goodloc) {
      do_sun_bands(canvas, up, JD);
   }

   // our rotating "now" hand
   do_now_hand(canvas, up, JD);

   // draw the moon
   double moon_angle;
   if (goodloc) {
      moon_angle = get_moon_angle(JD, lunar_new);
      do_moon_draw(canvas, JD, lunar_phase, lunar_bright_limb, moon_angle);
   }

   // draw accumulated times
   replay_accum_memory(canvas);

   // hour ticks
   do_hour_ticks(canvas, JD, mid, mid, mid / 2 + SCALE(128), up);

   // redraw some text, to make things pretty
   replayTimeDrawnMemory(canvas);

   // border bands
   arc_canvas(canvas, mid, mid, mid / 2 - SCALE(128), 1, COLOR_WHITE, 0, 360.0);
   arc_canvas(canvas, mid, mid, mid / 2 + SCALE(128), 1, COLOR_WHITE, 0, 360.0);

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

// vim: expandtab:noai:ts=3:sw=3
