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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "draw.h"
#include "clock.h"

/// @brief A useless function required by the C runtime library.
int main(int argc, char *argv[]) {
   if (argc < 3) {
      printf("Usage: %s <lat> <lon> [<date>] [<offset>]\n", argv[0]);
      printf("       for <lat>, N is positive, S is negative\n");
      printf("       for <lon>, E is positive, W is negative\n");
      printf("       <date> in YYYY/MM/DD format\n");
      printf("       <offset> additional float applied to date\n");
      exit(-1);
   }

   double offset = 0.0;
   int lightdark = 0;

   if (argv[3]) {
      if (strchr(argv[3], '/')) {
         int year, month, day;
         int ret = sscanf(argv[3], "%04d/%02d/%02d", &year, &month,
                          &day);
         if (ret == 3) {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            time_t t2 = mktime(&tm);
            offset = (t2 - t) / (60 * 60 * 24);
         }
         else {
            fprintf(stderr, "could not parse %s\n", argv[3]);
         }
      }
      else {
         offset = atof(argv[3]);
      }
   }
   if (argv[4]) {
      offset += atof(argv[4]);
   }
   if (argv[5]) {
      lightdark = atoi(argv[5]);
   }

   char *provider = getenv("PROVIDER");
   if (provider == NULL) {
      provider = "provider";
   }

   Canvas *canvas =
      do_all(atof(argv[1]), atof(argv[2]), offset, 1024, provider,
             "tzprovider", "timezone", lightdark, NULL, NULL);

   dump_canvas(canvas, "out.bin");

   printf("raw output written to out.bin\n");
   printf("`convert -size 1024x1024 -depth 8 RGBA:out.bin out.png` to view\n");
}
