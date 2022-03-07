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

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <math.h>

#include <libnova/solar.h>
#include <libnova/lunar.h>
#include <libnova/julian_day.h>
#include <libnova/rise_set.h>
#include <libnova/transform.h>

#include "draw.h"

#define DEG2RAD(x) ((x) * M_PI / 180.0)
#define RAD2DEG(x) ((x) * 180.0 / M_PI)

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

const char *weekdays[] = {
   "Sunday",
   "Monday",
   "Tuesday",
   "Wednesday",
   "Thursday",
   "Friday",
   "Saturday"
};

const char *signs[] = {
   "Aries", "Taurus", "Gemini",
   "Cancer", "Leo", "Virgo",
   "Libra", "Scorpio", "Sagitarius",
   "Capricorn", "Aquarius", "Pisces"
};

#define EVENT_DOWN 0
#define EVENT_RISE 1
#define EVENT_TRANSIT 2
#define EVENT_SET 3
#define EVENT_UP 4
#define EVENT_NONE 5
const char *typenames[] = { "down", "rise", "transit", "set", "up", "none" };

typedef struct {
   double jd;
   const char *fn_name;
   int event_type;
   int prune;
} Event;

#define NUM_EVENTS 4096
Event events[NUM_EVENTS];
int event_spot = 0;

#define SIZE 1024

/// @brief Normalize an angle in the range 0.0 .. 360.0
///
/// @param angle The angle to normalize, in degrees
/// @returns The same angle in the range 0.0 .. 360.0
double normalize_angle(double angle) {
   if (angle < 0.0) {
      angle += 360.0 * (1.0 - (int)(angle / 360.0));
   }
   if (angle >= 360.0) {
      angle -= 360.0 * (int)(angle / 360.0);
   }
   return angle;
}

double angle_between(double a, double b) {
   return fabs(RAD2DEG(atan2(sin(DEG2RAD(a - b)), cos(DEG2RAD(a - b)))));
}

double frac(double arg) {
   double integer;
   return modf(arg, &integer);
}

void remove_newlines(char *p) {
   while (*p) {
      if (((unsigned char)*p) < ' ') {
         *p = 0;
         break;
      }
      p++;
   }
}

void do_stars(Canvas * canvas) {
   for (int i = 0; i < 1000; i++) {
      int x = rand() % canvas->w;
      int y = rand() % canvas->h;
      int dx = rand() % 2 + 1;
      int dy = rand() % 2 + 1;

      for (int jx = -dx; jx <= dx; jx++) {
         conditional_poke_canvas(canvas, x + jx, y, COLOR_WHITE,
                                 COLOR_DARKBLUE);
      }
      for (int jy = -dy; jy <= dy; jy++) {
         conditional_poke_canvas(canvas, x, y + jy, COLOR_WHITE,
                                 COLOR_DARKBLUE);
      }
      if (rand() % 2) {
         conditional_poke_canvas(canvas, x + 1, y + 1,
                                 COLOR_WHITE, COLOR_DARKBLUE);
         conditional_poke_canvas(canvas, x - 1, y + 1,
                                 COLOR_WHITE, COLOR_DARKBLUE);
         conditional_poke_canvas(canvas, x + 1, y - 1,
                                 COLOR_WHITE, COLOR_DARKBLUE);
         conditional_poke_canvas(canvas, x - 1, y - 1,
                                 COLOR_WHITE, COLOR_DARKBLUE);
      }
   }
}

int
do_xy_time(Canvas * canvas, double now, double jd, int x, int y,
           unsigned int fg, unsigned int bg) {
   char time[32];
   struct ln_zonedate zonedate;
   ln_get_local_date(jd, &zonedate);
   sprintf(time, "%02d:%02d", zonedate.hours, zonedate.minutes);
   return text_canvas(canvas, jd < now ? djsmo_20_bdf : djsmb_20_bdf, x, y,
                      fg, bg, time, 1, 3);
}

void do_hour_ticks(Canvas * canvas, int x, int y, int r, double up) {
   double angle = -frac(up) * 360.0;

   double xa, ya;
   double xc, yc;

   Canvas *shadow = new_canvas(canvas->w, canvas->h, 0);

   for (int i = 0; i < 24; i++) {
      xa = x + (r - 8) * cos(DEG2RAD(angle + i * (360.0 / 24.0)));
      ya = y + (r - 8) * sin(DEG2RAD(angle + i * (360.0 / 24.0)));
      xc = x + r * cos(DEG2RAD(angle + i * (360.0 / 24.0)));
      yc = y + r * sin(DEG2RAD(angle + i * (360.0 / 24.0)));
      line_canvas(shadow, xa, ya, xc, yc, COLOR_BLACK);
   }
   xor_canvas(shadow, canvas);

   delete_canvas(shadow);
}

void do_now_hand(Canvas * canvas, double up, double now) {

   Canvas *shadow = new_canvas(canvas->w, canvas->h, 0);

   double up_angle = frac(up) * 360.0;
   double now_angle = frac(now) * 360.0 - up_angle + 270.0;
   double xc =
      canvas->w / 2 + (canvas->w / 2 / 2 + 128) * cos(DEG2RAD(now_angle));
   double yc =
      canvas->h / 2 + (canvas->h / 2 / 2 + 128) * sin(DEG2RAD(now_angle));
   double xc2 =
      canvas->w / 2 + (canvas->w / 2 / 2 - 128) * cos(DEG2RAD(now_angle));
   double yc2 =
      canvas->h / 2 + (canvas->h / 2 / 2 - 128) * sin(DEG2RAD(now_angle));

   thick_line_canvas(shadow, xc2, yc2, xc, yc, COLOR_WHITE, 3);
   xor_canvas(shadow, canvas);

   delete_canvas(shadow);
}

void do_now_time(Canvas * canvas, double now) {
   char time[32];
   struct ln_zonedate zonedate;
   ln_get_local_date(now, &zonedate);
   sprintf(time, "%02d:%02d", zonedate.hours, zonedate.minutes);
   text_canvas(canvas, djsmb_50_bdf, canvas->w / 2, canvas->h / 2,
               COLOR_BLACK, COLOR_WHITE, time, 1, 12);
}

void do_now_weekday(Canvas * canvas, double now) {
   struct ln_zonedate zonedate;
   ln_get_local_date(now, &zonedate);

   struct ln_date date;
   date.years = zonedate.years;
   date.months = zonedate.months;
   date.days = zonedate.days;
   date.hours = date.minutes = date.seconds = 0;

   int dow = ln_get_day_of_week(&date);

   char text[32];
   sprintf(text, "%s", weekdays[dow]);
   text_canvas(canvas, djsmb_20_bdf, canvas->w / 2, canvas->h / 2 - 90,
               COLOR_BLACK, COLOR_WHITE, text, 1, 2);
}

void do_now_date(Canvas * canvas, double now) {
   char time[32];
   struct ln_zonedate zonedate;
   ln_get_local_date(now, &zonedate);
   sprintf(time, "%s-%d", months[zonedate.months - 1], zonedate.days);
   text_canvas(canvas, djsmb_20_bdf, canvas->w / 2, canvas->h / 2 - 60,
               COLOR_BLACK, COLOR_WHITE, time, 1, 2);
}

void do_location(Canvas * canvas, struct ln_lnlat_posn *observer) {
   char location[128];

   double lat = observer->lat;
   char NS = 'N';
   if (lat < 0.0) {
      lat = -lat;
      NS = 'S';
   }

   double lng = observer->lng;
   char EW = 'E';
   if (lng < 0.0) {
      lng = -lng;
      EW = 'W';
   }
   char *degree = "\u00B0";     // in utf8, degree symbol

   sprintf(location, "%0.4f%s%c,%0.4f%s%c", lat, degree, NS, lng, degree, EW);
   text_canvas(canvas, djsmb_16_bdf, canvas->w / 2, canvas->h / 2 + 48,
               COLOR_BLACK, COLOR_WHITE, location, 1, 2);
}

void do_now_smalldate(Canvas * canvas, double now) {
   char time[32];
   struct ln_zonedate zonedate;
   ln_get_local_date(now, &zonedate);
   sprintf(time, "%04d-%02d-%02d", zonedate.years, zonedate.months,
           zonedate.days);
   text_canvas(canvas, djsmb_16_bdf, canvas->w / 2, canvas->h / 2 + 72,
               COLOR_BLACK, COLOR_WHITE, time, 1, 2);
}

void
do_moon_draw(Canvas * canvas,
             double up,
             double now,
             float lunar_phase,
             float lunar_bright_limb, float lunar_disk, double where_angle) {

   // this is, quite tedious.  here we go...
   int cx, cy;

   cx = canvas->w / 2 + (canvas->w / 2 / 2 + 128 + 16 +
                         64) * cos(DEG2RAD(where_angle));
   cy = canvas->h / 2 + (canvas->h / 2 / 2 + 128 + 16 +
                         64) * sin(DEG2RAD(where_angle));

   unsigned int interior_color;
   unsigned int chunk_color;
   int cxm = 1;
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
   int b = abs(lunar_phase - 90) * 40 / 90;
   if (b == 0) {
      b++;
   }
   int chunk_x = (b * b - 40 * 40) / (2 * b);
   int chunk_r = abs(chunk_x - b);

   // this won't make sense, because it's derived from earlier code...
   // but it really does draw the moon...
   for (int dx = -40; dx <= 40; dx++) {
      for (int dy = -40; dy <= 40; dy++) {
         double d_interior = sqrt(dx * dx + dy * dy);
         double d_chunk = sqrt((dx - chunk_x * cxm) * (dx - chunk_x * cxm) +
                               dy * dy);

         if (d_interior <= 40.0) {
            if (d_chunk < chunk_r) {
               poke_canvas(canvas, cx + dx, cy + dy, chunk_color);
            }
            else {
               poke_canvas(canvas, cx + dx, cy + dy, interior_color);
            }
         }
      }
   }

   // outline
   arc_canvas(canvas, cx, cy, 40, 1, COLOR_DARKGRAY, COLOR_NONE, 0, 360.0);

   int is_up = -1;
   for (int i = 0; i < event_spot; i++) {
      if (events[i].jd > now) {
         break;
      }
      if (events[i].fn_name[0] == 'l') {
         switch (events[i].event_type) {
         case EVENT_UP:
         case EVENT_RISE:
            is_up = 1;
            break;
         case EVENT_DOWN:
         case EVENT_SET:
            is_up = 0;
            break;
         }
      }
   }
   if (is_up == 1) {
      arc_canvas(canvas, cx, cy, 40, 1, COLOR_MOONBAND, COLOR_NONE, 0, 360.0);
      arc_canvas(canvas, cx, cy, 43, 1, COLOR_MOONBAND, COLOR_NONE, 0, 360.0);
   }
}

void
do_moon_band(Canvas * canvas, double up, double now, double moon_angle,
             unsigned int color) {
   double up_angle = frac(up) * 360.0;

   double last = now - .5;
   int is_up = -1;
   for (int i = 0; i < event_spot; i++) {
      if (events[i].jd > now + .5) {
         break;
      }
      if (events[i].fn_name[0] == 'l') {
         switch (events[i].event_type) {
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
               arc_canvas(canvas, canvas->w / 2, canvas->h / 2,
                          canvas->w / 2 / 2 + 128 + 16, 5, color, COLOR_NONE,
                          start_angle, stop_angle);

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
      arc_canvas(canvas, canvas->w / 2, canvas->h / 2,
                 canvas->w / 2 / 2 + 128 + 16, 5, color, COLOR_NONE,
                 start_angle, stop_angle);
   }

   for (int i = 0; i < event_spot; i++) {
      if (events[i].fn_name[0] == 'l') {
         if (!events[i].prune) {
            switch (events[i].event_type) {
            case EVENT_RISE:
            case EVENT_TRANSIT:
            case EVENT_SET:
               {
                  double x1, y1, x2, y2;
                  double x, y;
                  double angle = frac(events[i].jd) * 360.0 - up_angle + 270.0;

                  x1 = canvas->w / 2 +
                     (canvas->w / 2 / 2 + 128 + 16 - 16) * cos(DEG2RAD(angle));
                  y1 = canvas->h / 2 +
                     (canvas->h / 2 / 2 + 128 + 16 - 16) * sin(DEG2RAD(angle));
                  x2 = canvas->w / 2 +
                     (canvas->w / 2 / 2 + 128 + 16 + 16) * cos(DEG2RAD(angle));
                  y2 = canvas->h / 2 +
                     (canvas->h / 2 / 2 + 128 + 16 + 16) * sin(DEG2RAD(angle));
                  thick_line_canvas(canvas, x1, y1, x2, y2, color, 3);

                  if (angle_between(angle, moon_angle) < 10.0) {
                     angle = moon_angle - 10.0;
                  }
                  x = (canvas->w / 2) +
                     (canvas->w / 2 / 2 + 128 + 16 + 50) * cos(DEG2RAD(angle));
                  y = (canvas->h / 2) +
                     (canvas->h / 2 / 2 + 128 + 16 + 50) * sin(DEG2RAD(angle));
                  do_xy_time(canvas, now,
                             events[i].jd, x, y, COLOR_BLACK, COLOR_WHITE);
               }
               break;
            }
         }
      }
   }
}

void events_clear(void) {
   event_spot = 0;
}

int event_compar(const void *a, const void *b) {
   Event *pa = (Event *) a;
   Event *pb = (Event *) b;

   if (pa->jd < pb->jd) {
      return -1;
   }
   else if (pa->jd > pb->jd) {
      return 1;
   }
   else if (pa->event_type < pb->event_type) {
      return -1;
   }
   else if (pa->event_type > pb->event_type) {
      return 1;
   }
   else {
      if (pa->fn_name[0] == pb->fn_name[0]) {
         return 0;
      }
      // same type, different names
      static const char *order_up = "ancsl";
      static const char *order_down = "lscna";

      const char *order = order_up;

      if (pa->event_type == EVENT_DOWN) {
         order = order_down;
      }

      for (const char *p = order; *p; p++) {
         if (pa->fn_name[0] == *p) {
            return -1;
         }
         if (pb->fn_name[0] == *p) {
            return 1;
         }
      }

      // huh???
      return strcmp(pa->fn_name, pb->fn_name);
   }
}

void events_sort(void) {
   qsort(events, event_spot, sizeof(Event), event_compar);
}

void events_prune(double jd) {
   for (int i = 0; i < event_spot; i++) {
      events[i].prune = (fabs(events[i].jd - jd) > 0.5) ? 1 : 0;
   }
}

void events_dump(void) {
   struct ln_zonedate zonedate;

   for (int i = 0; i < event_spot; i++) {
      ln_get_local_date(events[i].jd, &zonedate);
      printf("%6.12f %02d:%02d %d %s %s\n", events[i].jd,
             zonedate.hours, zonedate.minutes,
             events[i].prune, typenames[events[i].event_type],
             events[i].fn_name);
   }
}

#define ONE_MINUTE_JD (1.0/(24.0*60.0))

typedef void (*Get_Equ_Coords)(double, struct ln_equ_posn *);

void
my_get_everything_helper(double JD,
                         struct ln_lnlat_posn *observer,
                         Get_Equ_Coords get_equ_coords,
                         double horizon, const char *fn_name, int event_type) {

   struct ln_equ_posn posn;
   struct ln_hrz_posn hrz_posn;
   double angle;
   double angles[3];

   int iterations = 0;

   get_equ_coords(JD, &posn);
   ln_get_hrz_from_equ(&posn, observer, JD, &hrz_posn);
   angle = hrz_posn.alt;

   switch (event_type) {
   case EVENT_RISE:
      while (angle >= horizon) {
         iterations++;
         JD -= ONE_MINUTE_JD;
         get_equ_coords(JD, &posn);
         ln_get_hrz_from_equ(&posn, observer, JD, &hrz_posn);
         angle = hrz_posn.alt;
         if (iterations > 120) {
            goto abort;
         }
      }
      while (angle < horizon) {
         iterations++;
         JD += ONE_MINUTE_JD;
         get_equ_coords(JD, &posn);
         ln_get_hrz_from_equ(&posn, observer, JD, &hrz_posn);
         angle = hrz_posn.alt;
         if (iterations > 120) {
            goto abort;
         }
      }
      events[event_spot++] = (Event) {
      JD, fn_name, EVENT_RISE};
      break;
   case EVENT_SET:
      while (angle >= horizon) {
         iterations++;
         JD += ONE_MINUTE_JD;
         get_equ_coords(JD, &posn);
         ln_get_hrz_from_equ(&posn, observer, JD, &hrz_posn);
         angle = hrz_posn.alt;
         if (iterations > 120) {
            goto abort;
         }
      }
      while (angle < horizon) {
         iterations++;
         JD -= ONE_MINUTE_JD;
         get_equ_coords(JD, &posn);
         ln_get_hrz_from_equ(&posn, observer, JD, &hrz_posn);
         angle = hrz_posn.alt;
         if (iterations > 120) {
            goto abort;
         }
      }
      events[event_spot++] = (Event) {
      JD, fn_name, EVENT_SET};
      break;
   case EVENT_TRANSIT:
      angles[1] = angle;
      ln_get_hrz_from_equ(&posn, observer, JD - ONE_MINUTE_JD, &hrz_posn);
      angles[0] = hrz_posn.alt;
      ln_get_hrz_from_equ(&posn, observer, JD + ONE_MINUTE_JD, &hrz_posn);
      angles[2] = hrz_posn.alt;
      while (angles[0] < angles[1] && angles[1] < angles[2]) {
         iterations++;
         angles[0] = angles[1];
         angles[1] = angles[2];
         JD += ONE_MINUTE_JD;
         ln_get_hrz_from_equ(&posn, observer, JD + ONE_MINUTE_JD, &hrz_posn);
         angles[2] = hrz_posn.alt;
         if (iterations > 120) {
            goto abort;
         }
      }
      while (angles[0] > angles[1] && angles[1] > angles[2]) {
         iterations++;
         angles[2] = angles[1];
         angles[1] = angles[0];
         JD -= ONE_MINUTE_JD;
         ln_get_hrz_from_equ(&posn, observer, JD - ONE_MINUTE_JD, &hrz_posn);
         angles[0] = hrz_posn.alt;
         if (iterations > 120) {
            goto abort;
         }
      }
      // only maxima above horizon count as "transit"
      if (angles[1] > horizon) {
         events[event_spot++] = (Event) {
         JD, fn_name, EVENT_TRANSIT};
      }
      break;
   }
 abort:
   return;
}

// collect all events
//
// one may ask, why not use ln_get_solar_rst() and various related functions?
// turns out these functions are buggy near the poles. so instead we
// roll our own, and look for horizon crossings (for rise/set) and local
// maxima (for transits).  we do this first with an approximated position
// for the object (to save computation time) which we then refine in the
// my_get_everything_helper() above.
void
my_get_everything(double JDstart,
                  double JDend,
                  struct ln_lnlat_posn *observer,
                  Get_Equ_Coords get_equ_sun_coords,
                  Get_Equ_Coords get_equ_moon_coords) {

   struct ln_equ_posn sun_posn;
   struct ln_equ_posn moon_posn;

   double sun_angle_2 = 0.0;
   double sun_angle_1 = 0.0;

   double moon_angle_2 = 0.0;
   double moon_angle_1 = 0.0;

   // the sun doesn't move much, calculate it once

   get_equ_sun_coords((JDstart + JDend) / 2.0, &sun_posn);

   for (double i = JDstart - (ONE_MINUTE_JD * 2.0); i < JDend;
        i += ONE_MINUTE_JD) {
      struct ln_hrz_posn sun_hrz_posn;
      ln_get_hrz_from_equ(&sun_posn, observer, i, &sun_hrz_posn);
      double sun_angle = sun_hrz_posn.alt;

      if (i >= JDstart) {
         if (sun_angle_1 < LN_SOLAR_ASTRONOMICAL_HORIZON &&
             sun_angle >= LN_SOLAR_ASTRONOMICAL_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_sun_coords,
                                     LN_SOLAR_ASTRONOMICAL_HORIZON,
                                     "astronomical", EVENT_RISE);
         }
         if (sun_angle_1 < LN_SOLAR_NAUTIC_HORIZON &&
             sun_angle >= LN_SOLAR_NAUTIC_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_sun_coords,
                                     LN_SOLAR_NAUTIC_HORIZON,
                                     "nautical", EVENT_RISE);
         }
         if (sun_angle_1 < LN_SOLAR_CIVIL_HORIZON &&
             sun_angle >= LN_SOLAR_CIVIL_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_sun_coords,
                                     LN_SOLAR_CIVIL_HORIZON,
                                     "civil", EVENT_RISE);
         }
         if (sun_angle_1 < LN_SOLAR_STANDART_HORIZON &&
             sun_angle >= LN_SOLAR_STANDART_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_sun_coords,
                                     LN_SOLAR_STANDART_HORIZON,
                                     "solar", EVENT_RISE);
         }

         if (sun_angle_1 >= LN_SOLAR_ASTRONOMICAL_HORIZON &&
             sun_angle < LN_SOLAR_ASTRONOMICAL_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_sun_coords,
                                     LN_SOLAR_ASTRONOMICAL_HORIZON,
                                     "astronomical", EVENT_SET);
         }
         if (sun_angle_1 >= LN_SOLAR_NAUTIC_HORIZON &&
             sun_angle < LN_SOLAR_NAUTIC_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_sun_coords,
                                     LN_SOLAR_NAUTIC_HORIZON,
                                     "nautical", EVENT_SET);
         }
         if (sun_angle_1 >= LN_SOLAR_CIVIL_HORIZON &&
             sun_angle < LN_SOLAR_CIVIL_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_sun_coords,
                                     LN_SOLAR_CIVIL_HORIZON,
                                     "civil", EVENT_SET);
         }
         if (sun_angle_1 >= LN_SOLAR_STANDART_HORIZON &&
             sun_angle < LN_SOLAR_STANDART_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_sun_coords,
                                     LN_SOLAR_STANDART_HORIZON,
                                     "solar", EVENT_SET);
         }

         if (sun_angle_2 < sun_angle_1 && sun_angle < sun_angle_1) {
            my_get_everything_helper(i, observer, get_equ_sun_coords, -90.1,    // a fake horizon
                                     "solar", EVENT_TRANSIT);
         }
      }

      sun_angle_2 = sun_angle_1;
      sun_angle_1 = sun_angle;
   }

   // the moon moves a lot more, so it is more complicated.

   double moon_angle;
   struct ln_hrz_posn moon_hrz_posn;

   // now do a quick pass for rough times,
   // and call a helper for refinement

   struct ln_equ_posn moon_posn_start;
   double moon_jd_start = JDstart - (ONE_MINUTE_JD * 2.0);
   get_equ_moon_coords(moon_jd_start, &moon_posn_start);
   float moon_xyz_start[3];
   moon_xyz_start[0] =
      cos(DEG2RAD(moon_posn_start.dec)) * cos(DEG2RAD(moon_posn_start.ra));
   moon_xyz_start[1] =
      cos(DEG2RAD(moon_posn_start.dec)) * sin(DEG2RAD(moon_posn_start.ra));
   moon_xyz_start[2] = sin(DEG2RAD(moon_posn_start.dec));

   struct ln_equ_posn moon_posn_end;
   double moon_jd_end = JDend;
   get_equ_moon_coords(moon_jd_end, &moon_posn_end);
   float moon_xyz_end[3];
   moon_xyz_end[0] =
      cos(DEG2RAD(moon_posn_end.dec)) * cos(DEG2RAD(moon_posn_end.ra));
   moon_xyz_end[1] =
      cos(DEG2RAD(moon_posn_end.dec)) * sin(DEG2RAD(moon_posn_end.ra));
   moon_xyz_end[2] = sin(DEG2RAD(moon_posn_end.dec));

   for (double i = JDstart - (ONE_MINUTE_JD * 2.0); i < JDend;
        i += ONE_MINUTE_JD) {

      // === INTERPOLATE_MOON POSITION (because it is faster)
      float fraction = (i - JDstart) / (JDend - JDstart);
      float xyz[3];
      xyz[0] =
         moon_xyz_start[0] + fraction * (moon_xyz_end[0] - moon_xyz_start[0]);
      xyz[1] =
         moon_xyz_start[1] + fraction * (moon_xyz_end[1] - moon_xyz_start[1]);
      xyz[2] =
         moon_xyz_start[2] + fraction * (moon_xyz_end[2] - moon_xyz_start[2]);
      float d = sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
      xyz[0] /= d;
      xyz[1] /= d;
      xyz[2] /= d;
      moon_posn.ra = RAD2DEG(atan2(xyz[1], xyz[0]));
      moon_posn.dec = RAD2DEG(asin(xyz[2]));
      // === END

      ln_get_hrz_from_equ(&moon_posn, observer, i, &moon_hrz_posn);
      moon_angle = moon_hrz_posn.alt;

      if (i >= JDstart) {
         if (moon_angle_1 < LN_LUNAR_STANDART_HORIZON &&
             moon_angle >= LN_LUNAR_STANDART_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_moon_coords,
                                     LN_LUNAR_STANDART_HORIZON,
                                     "lunar", EVENT_RISE);
         }

         if (moon_angle_1 >= LN_LUNAR_STANDART_HORIZON &&
             moon_angle < LN_LUNAR_STANDART_HORIZON) {
            my_get_everything_helper(i,
                                     observer,
                                     get_equ_moon_coords,
                                     LN_LUNAR_STANDART_HORIZON,
                                     "lunar", EVENT_SET);
         }

         if (moon_angle_2 < moon_angle_1 && moon_angle < moon_angle_1) {
            my_get_everything_helper(i, observer,
                                     get_equ_moon_coords,
                                     LN_LUNAR_STANDART_HORIZON,
                                     "lunar", EVENT_TRANSIT);
         }
      }

      moon_angle_2 = moon_angle_1;
      moon_angle_1 = moon_angle;
   }

   // find the earliest time

   double earliest = events[0].jd;
   for (int i = 1; i < event_spot; i++) {
      if (events[i].jd < earliest) {
         earliest = events[i].jd;
      }
   }
   earliest -= ONE_MINUTE_JD;

   // now do all the up downs for the earliest time

   // solar

   get_equ_sun_coords(earliest, &sun_posn);
   struct ln_hrz_posn sun_hrz_posn;
   ln_get_hrz_from_equ(&sun_posn, observer, earliest, &sun_hrz_posn);
   double sun_angle = sun_hrz_posn.alt;

   if (sun_angle < LN_SOLAR_STANDART_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "solar", EVENT_DOWN};
   }
   if (sun_angle < LN_SOLAR_CIVIL_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "civil", EVENT_DOWN};
   }
   if (sun_angle < LN_SOLAR_NAUTIC_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "nautical", EVENT_DOWN};
   }
   if (sun_angle < LN_SOLAR_ASTRONOMICAL_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "astronomical", EVENT_DOWN};
   }

   if (sun_angle >= LN_SOLAR_ASTRONOMICAL_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "astronomical", EVENT_UP};
   }
   if (sun_angle >= LN_SOLAR_NAUTIC_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "nautical", EVENT_UP};
   }
   if (sun_angle >= LN_SOLAR_CIVIL_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "civil", EVENT_UP};
   }
   if (sun_angle >= LN_SOLAR_STANDART_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "solar", EVENT_UP};
   }

   // lunar

   get_equ_moon_coords(earliest, &moon_posn);
   ln_get_hrz_from_equ(&moon_posn, observer, earliest, &moon_hrz_posn);
   moon_angle = moon_hrz_posn.alt;

   if (moon_angle >= LN_LUNAR_STANDART_HORIZON) {
      events[event_spot++] = (Event) {
      earliest, "lunar", EVENT_UP};
   }
   else {
      events[event_spot++] = (Event) {
      earliest, "lunar", EVENT_DOWN};
   }
}

void events_populate(double JD, struct ln_lnlat_posn *observer) {
   my_get_everything(JD - 2.0, JD + 1.0, observer,
                     ln_get_solar_equ_coords, ln_get_lunar_equ_coords);
}

double events_transit(double jd) {
   // any non pruned, non lunar, will do...
   for (int i = 0; i < event_spot; i++) {
      if (!events[i].prune &&
          events[i].event_type == EVENT_TRANSIT &&
          strcmp(events[i].fn_name, "lunar")) {
         return frac(events[i].jd);
      }
   }
   // non?  how about a lunar transit?
   for (int i = 0; i < event_spot; i++) {
      if (!events[i].prune &&
          events[i].event_type == EVENT_TRANSIT &&
          !strcmp(events[i].fn_name, "lunar")) {
         return frac(events[i].jd);
      }
   }
   // still nothing? LOCAL noon it is i guess
   struct ln_zonedate now;
   ln_get_local_date(jd, &now);
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
      text_canvas(canvas, djsmb_20_bdf, SIZE / 2, SIZE / 2 / 2 + 64,
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
      text_canvas(canvas, djsmb_20_bdf, SIZE / 2 - 104, SIZE / 2 / 2,
                  COLOR_BLACK, COLOR_YELLOW, buffer, 1, 3);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // wind
      remove_newlines(buffer);
      text_canvas(canvas, djsmb_20_bdf, SIZE / 2 + 128, SIZE / 2 / 2,
                  COLOR_BLACK, COLOR_YELLOW, buffer, 1, 3);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // desc
      remove_newlines(buffer);
      text_canvas(canvas, djsmb_20_bdf, SIZE / 2, SIZE * 3 / 4 + 64,
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
      text_canvas(canvas, djsmb_20_bdf, SIZE / 2 - 104, SIZE * 3 / 4,
                  COLOR_WHITE, COLOR_DARKBLUE, buffer, 1, 3);

      if (!fgets(buffer, sizeof(buffer) - 1, f)) {
         return;
      }                         // wind
      remove_newlines(buffer);
      text_canvas(canvas, djsmb_20_bdf, SIZE / 2 + 128, SIZE * 3 / 4,
                  COLOR_WHITE, COLOR_DARKBLUE, buffer, 1, 3);

      fclose(f);
   }
}

void do_zodiac(Canvas * canvas, double JD) {
   struct ln_equ_posn sun_equatorial;
   ln_get_solar_equ_coords(JD, &sun_equatorial);
   double ra = sun_equatorial.ra;

   Canvas *shadow = new_canvas(canvas->w, canvas->h, 0);
   char buf[2] = { 0, 0 };
   for (int sign = 0; sign < 12; sign++) {
      // aries is at 0 + (360.0/12.0/2.0)
      int angle = sign * (360.0 / 12.0);
      int theta = 270.0 - ra + (360.0 / 12.0 / 2.0) + angle;
      int cx =
         canvas->w / 2 + (canvas->w / 2 / 2 - 128 + 20) * cos(DEG2RAD(theta));
      int cy =
         canvas->h / 2 + (canvas->h / 2 / 2 - 128 + 20) * sin(DEG2RAD(theta));
      buf[0] = 'a' + sign;
      text_canvas(shadow, astro_32_bdf, cx, cy, COLOR_BLACK, COLOR_NONE, buf, 1,
                  3);

      theta += (360.0 / 12.0 / 2.0);
      cx = canvas->w / 2 + (canvas->w / 2 / 2 - 128 + 20 +
                            15) * cos(DEG2RAD(theta));
      cy = canvas->h / 2 + (canvas->h / 2 / 2 - 128 + 20 +
                            15) * sin(DEG2RAD(theta));
      int cx2 =
         canvas->w / 2 + (canvas->w / 2 / 2 - 128 + 20 -
                          15) * cos(DEG2RAD(theta));
      int cy2 =
         canvas->h / 2 + (canvas->h / 2 / 2 - 128 + 20 -
                          15) * sin(DEG2RAD(theta));
      line_canvas(shadow, cx, cy, cx2, cy2, COLOR_BLACK);
   }
   xor_canvas(shadow, canvas);
   delete_canvas(shadow);
}

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
   double x =
      (canvas->w / 2) +
      ((canvas->w / 3 - 24) * 5 / 8) * cos(DEG2RAD(draw_angle));
   double y =
      (canvas->h / 2) +
      ((canvas->h / 3 - 24) * 5 / 8) * sin(DEG2RAD(draw_angle));
   text_canvas(canvas, djsmb_20_bdf, x, y - 16, fore, back, buffer, 1, 3);
   text_canvas(canvas, djsmb_20_bdf, x, y + 16, fore, back, label, 1, 3);
}

typedef struct TimeDrawnMemory {
   int x;
   int y;
   int w;
   int h;
} TimeDrawnMemory;
TimeDrawnMemory timedrawnmemory[32];
int timedrawnspot = 0;

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

void
do_tr_time_sun(Canvas * canvas, double now, double jd, double theta,
               double r, unsigned int fg, unsigned int bg) {

   // get width and height by drawing offscreen
   int wh = do_xy_time(canvas, now, jd, -300, -300, fg, bg);
   int w = wh >> 16;
   int h = wh & 0xFFFF;

   int collision;
   int x, y;

   do {
      x = (canvas->w / 2) + r * cos(DEG2RAD(theta));
      y = (canvas->h / 2) + r * sin(DEG2RAD(theta));

      collision = check(x, y, w, h);

      r--;
   }
   while (collision);

   timedrawnmemory[timedrawnspot++] = (TimeDrawnMemory) {
   x, y, w, h};

   do_xy_time(canvas, now, jd, x, y, fg, bg);
}

void do_sun_bands(Canvas * canvas, double up, double now) {
   static const double one_minute = 360.0 / 24.0 / 60.0;
   double last = now - 0.5;
   unsigned int color = COLOR_DARKBLUE;
   unsigned int fore = COLOR_WHITE;
   unsigned int back = COLOR_DARKBLUE;

   unsigned int transit_fore = COLOR_GREEN;
   unsigned int transit_back = COLOR_RED;

   double daylight = 0.0;
   double civil = 0.0;
   double nautical = 0.0;
   double astronomical = 0.0;
   double darkness = 0.0;

   double up_angle = frac(up) * 360.0;

   int started = 0;
   double transited = 0.0;

   int times_written = 0;

   // big, messy state machine...
   for (int i = 0; i < event_spot; i++) {
      if (strcmp(events[i].fn_name, "lunar")) {
         if (events[i].event_type != EVENT_TRANSIT) {
            if (events[i].prune == 0) {
               double here = events[i].jd;
               double start_angle = frac(last) * 360.0 - up_angle + 270.0;
               double stop_angle = frac(here) * 360.0 - up_angle + 270.0;

               arc_canvas(canvas, canvas->w / 2,
                          canvas->h / 2,
                          canvas->w / 2 / 2,
                          canvas->h / 2 / 2, color,
                          COLOR_NONE, start_angle, stop_angle);

               // accumulate daylight and darkness
               start_angle = normalize_angle(start_angle);
               stop_angle = normalize_angle(stop_angle);
               while (stop_angle < start_angle) {
                  stop_angle += 360.0;
               }
               switch (color) {
               case COLOR_YELLOW:
                  daylight += stop_angle - start_angle;
                  break;
               case COLOR_ORANGE:
                  civil += stop_angle - start_angle;
                  break;
               case COLOR_LIGHTBLUE:
                  nautical += stop_angle - start_angle;
                  break;
               case COLOR_BLUE:
                  astronomical += stop_angle - start_angle;
                  break;
               case COLOR_DARKBLUE:
                  darkness += stop_angle - start_angle;
                  break;
               }

               if (started) {
                  times_written++;
                  // TODO FIX check size
                  do_tr_time_sun(canvas, now, last, start_angle,
                                 (canvas->w / 3 - 24), fore, back);
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

         if (events[i].event_type == EVENT_UP
             || events[i].event_type == EVENT_RISE) {
            switch (events[i].fn_name[0]) {
            case 'a':          // astronomical
               color = back = COLOR_BLUE;
               fore = COLOR_WHITE;
               break;
            case 'n':          // nautical
               color = back = COLOR_LIGHTBLUE;
               fore = COLOR_WHITE;
               break;
            case 'c':          // civil
               color = back = COLOR_ORANGE;
               fore = COLOR_BLACK;
               break;
            case 's':          // solar
               color = back = COLOR_YELLOW;
               fore = COLOR_BLACK;
               break;
            }
         }

         if (events[i].event_type == EVENT_SET) {
            switch (events[i].fn_name[0]) {
            case 'a':          // astronomical
               back = color;
               color = COLOR_DARKBLUE;
               fore = COLOR_WHITE;
               break;
            case 'n':          // nautical
               back = color;
               color = COLOR_BLUE;
               fore = COLOR_WHITE;
               break;
            case 'c':          // civil
               back = color;
               color = COLOR_LIGHTBLUE;
               fore = COLOR_BLACK;
               break;
            case 's':          // solar
               back = color;
               color = COLOR_ORANGE;
               fore = COLOR_BLACK;
               break;
            }
         }
      }
   }

   double here = now + 0.5;
   double start_angle = frac(last) * 360.0 - up_angle + 270.0;
   double stop_angle = frac(here) * 360.0 - up_angle + 270.0;
   arc_canvas(canvas, canvas->w / 2, canvas->h / 2, canvas->w / 2 / 2,
              canvas->h / 2 / 2, color, COLOR_NONE, start_angle, stop_angle);

   // accumulate daylight and darkness
   start_angle = normalize_angle(start_angle);
   stop_angle = normalize_angle(stop_angle);
   while (stop_angle <= start_angle) {
      stop_angle += 360.0;
   }
   switch (color) {
   case COLOR_YELLOW:
      daylight += stop_angle - start_angle;
      break;
   case COLOR_ORANGE:
      civil += stop_angle - start_angle;
      break;
   case COLOR_LIGHTBLUE:
      nautical += stop_angle - start_angle;
      break;
   case COLOR_BLUE:
      astronomical += stop_angle - start_angle;
      break;
   case COLOR_DARKBLUE:
      darkness += stop_angle - start_angle;
      break;
   }

   if (times_written) {
      do_tr_time_sun(canvas, now, last, start_angle,
                     canvas->w / 3 - 24, fore, back);
   }

   if (transited != 0.0) {
      double angle = frac(transited) * 360.0 - up_angle + 270.0;
      do_tr_time_sun(canvas, now, transited, angle,
                     canvas->w / 3 - 24, transit_fore, transit_back);
   }

   if (daylight > one_minute) {
      accum_helper(canvas, daylight, "daylight", 270.0, COLOR_BLACK,
                   COLOR_YELLOW);
   }
   else if (civil > one_minute) {
      accum_helper(canvas, civil, "civil", 270.0, COLOR_BLACK, COLOR_ORANGE);
   }
   else if (nautical > one_minute) {
      accum_helper(canvas, nautical, "nautical", 270.0, COLOR_WHITE,
                   COLOR_LIGHTBLUE);
   }
   else if (astronomical > one_minute) {
      accum_helper(canvas, astronomical, "astronomical", 270.0,
                   COLOR_WHITE, COLOR_BLUE);
   }
   else if (darkness > one_minute) {
      accum_helper(canvas, darkness, "darkness", 270.0, COLOR_WHITE,
                   COLOR_DARKBLUE);
   }

   if (darkness > one_minute) {
      if (astronomical > one_minute) {
         accum_helper(canvas, darkness, "darkness", 90.0,
                      COLOR_WHITE, COLOR_DARKBLUE);
      }
   }
   else if (astronomical > one_minute) {
      if (nautical > one_minute) {
         accum_helper(canvas, astronomical, "astronomical", 90.0,
                      COLOR_WHITE, COLOR_BLUE);
      }
   }
   else if (nautical > one_minute) {
      if (civil > one_minute) {
         accum_helper(canvas, nautical, "nautical", 90.0,
                      COLOR_WHITE, COLOR_LIGHTBLUE);
      }
   }
   else if (civil > one_minute) {
      if (daylight > one_minute) {
         accum_helper(canvas, civil, "civil", 90.0, COLOR_BLACK, COLOR_ORANGE);
      }
   }
}

double get_lunar_new(double now) {
   double min = ln_get_lunar_disk(now);
   double when = now;

   for (double i = 1.0; i < 45.0; i += 1.0) {
      double lunar_disk = ln_get_lunar_disk(now + i);
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

double get_moon_angle(double JD, double lunar_new) {
   const double synodic_month = 29.530588;      // lunar synodic month
   while (lunar_new > JD) {
      lunar_new -= synodic_month;
   }
   double percent = (JD - lunar_new) / synodic_month;
   double angle = normalize_angle(270.0 + percent * 360.0);
   return angle;
}

Canvas *do_all(double lat, double lng, double offset) {
   struct ln_zonedate now;
   struct ln_lnlat_posn observer;

   double JD;
   double up;

   // observer's location
   observer.lat = lat;          // degrees, North is positive
   observer.lng = lng;          // degrees, East is positive

   // get Julian day from local time
   JD = ln_get_julian_from_sys() + offset;

   // local time
   ln_get_local_date(JD, &now);

   // all of the data
   events_clear();
   events_populate(JD, &observer);
   events_sort();
   events_prune(JD);

   // get the transit time
   up = events_transit(JD);

   // lunar disk, phase and bright limb
   float lunar_disk = ln_get_lunar_disk(JD);    // 0 to 1
   float lunar_phase = ln_get_lunar_phase(JD);  // 0 to 180
   float lunar_bright_limb = ln_get_lunar_bright_limb(JD);      // 0 to 360

   double lunar_new = get_lunar_new(JD);

   //// drawing begins here
   Canvas *canvas = new_canvas(SIZE, SIZE, 0xFF);

   int mid = canvas->w / 2;

   // draw the moon
   double moon_angle = get_moon_angle(JD, lunar_new);
   do_moon_draw(canvas, up, JD, lunar_phase, lunar_bright_limb, lunar_disk,
                moon_angle);

   // colored bands for the sun
   do_sun_bands(canvas, up, JD);

   // hour ticks
   do_hour_ticks(canvas, mid, mid, mid / 2 + 128, up);

   // black border bands
   arc_canvas(canvas, mid, mid, mid / 2 - 128, 1, COLOR_BLACK, COLOR_NONE,
              0, 360.0);
   arc_canvas(canvas, mid, mid, mid / 2 + 128, 1, COLOR_BLACK, COLOR_NONE,
              0, 360.0);

   // our rotating "now" hand
   do_now_hand(canvas, up, JD);

   // zodiac, skip because woo-woo
   // do_zodiac(canvas, JD);

   // colored band for the moon
   do_moon_band(canvas, up, JD, moon_angle, COLOR_MOONBAND);

   // information in the center
   do_now_time(canvas, JD);
   do_now_weekday(canvas, JD);
   do_now_date(canvas, JD);
   do_location(canvas, &observer);
   //do_timezone();
   do_now_smalldate(canvas, JD);

   // embedded watch won't have weather info
   //do_weather(canvas);

   // for debugging, put the Julian date in the lower right
   char buf[64];
   sprintf(buf, "JD=%f", JD);
   text_canvas(canvas, djsmb_20_bdf, canvas->w * 27 / 32, canvas->h - 16,
               COLOR_BLACK, COLOR_WHITE, buf, 1, 3);

   return canvas;
}
