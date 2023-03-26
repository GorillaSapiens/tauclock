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

#ifndef _INCLUDE_DRAW_H_
#define _INCLUDE_DRAW_H_

#include "fonts/djsmb_60_bdf.h"
#include "fonts/djsmb_50_bdf.h"
#include "fonts/djsmb_40_bdf.h"
#include "fonts/djsmb_32_bdf.h"
#include "fonts/djsmb_24_bdf.h"
#include "fonts/djsmb_20_bdf.h"
#include "fonts/djsmb_16_bdf.h"
#include "fonts/djsmb_10_bdf.h"
#include "fonts/djsmb_8_bdf.h"

#include "fonts/djsmo_60_bdf.h"
#include "fonts/djsmo_50_bdf.h"
#include "fonts/djsmo_40_bdf.h"
#include "fonts/djsmo_32_bdf.h"
#include "fonts/djsmo_24_bdf.h"
#include "fonts/djsmo_20_bdf.h"
#include "fonts/djsmo_16_bdf.h"
#include "fonts/djsmo_10_bdf.h"
#include "fonts/djsmo_8_bdf.h"

#include "fonts/astro_50_bdf.h"
#include "fonts/astro_32_bdf.h"
#include "fonts/astro_24_bdf.h"
#include "fonts/astro_20_bdf.h"
#include "fonts/astro_16_bdf.h"

#include "fonts/icons_128x128.h"

#define COLOR_NONE       0x00000000
#define COLOR_XOR        0x00000001

#ifdef STANDALONE
#define RGB(r,g,b) (0xFF000000 | ((b) << 16) | ((g) << 8) | (r))
#else
#define RGB(r,g,b) (0xFF000000 | ((r) << 16) | ((g) << 8) | (b))
#endif

#define COLOR_BLACK      RGB(  0,   0,   0)
#define COLOR_BLUE       RGB(  0,   0, 255)
#define COLOR_CYAN       RGB(  0, 255, 255)
#define COLOR_DARKBLUE   RGB(  0,   0, 128)
#define COLOR_DARKGRAY   RGB( 64,  64,  64)
#define COLOR_GRAY       RGB(128, 128, 128)
#define COLOR_GREEN      RGB(  0, 255,   0)
#define COLOR_LIGHTBLUE  RGB( 76, 171, 244)
#define COLOR_LIGHTGRAY  RGB(192, 192, 192)
#define COLOR_MAGENTA    RGB(255,   0, 255)
#define COLOR_MOONBAND   RGB(136,  88, 222)
#define COLOR_ORANGE     RGB(255, 165,   0)
#define COLOR_PURPLE     RGB(128,   0, 128)
#define COLOR_RED        RGB(255,   0,   0)
#define COLOR_WHITE      RGB(255, 255, 255)
#define COLOR_YELLOW     RGB(255, 255,   0)

#define COLOR_SILVER     RGB(190, 194, 203)

#define COLOR_LOCK       0x80000000
#define LOCK(x)          ((x) & 0xFEFFFFFF)
#define ISLOCKED(x)      (!((x) & 0x01000000))


/// @brief A struct defining an area to draw on.
typedef struct Canvas {
   int w;                       ///< The width of the Canvas
   int h;                       ///< The height of the Canvas
   unsigned int *data;          ///< Pointer to Canvas data
} Canvas;

Canvas *new_canvas(int w, int h, unsigned int fill);
void delete_canvas(Canvas * canvas);
void dump_canvas(Canvas * canvas, const char *fname);

void xor_canvas(Canvas * mask, Canvas * target);
void poke_canvas(Canvas * canvas, int x, int y, unsigned int c);
void conditional_poke_canvas(Canvas * canvas, int x, int y,
      unsigned int color, unsigned int old);
unsigned int peek_canvas(Canvas * canvas, int x, int y);
int text_canvas(Canvas * canvas, uint8_t * font, int x, int y,
      unsigned int fg, unsigned int bg, const char *p, int mult,
      int gap);
void blur_poke_canvas(Canvas * canvas, int x, int y, unsigned int color,
      int blur);
void line_canvas(Canvas * canvas, int x1, int y1, int x2, int y2,
      unsigned int color);
void thick_line_canvas(Canvas * canvas, int x1, int y1, int x2, int y2,
      unsigned int color, int thickness);
void arc_canvas(Canvas * canvas, int center_x, int center_y, int radius,
      int strokewidth, unsigned int strokecolor,
      double begin_deg, double end_deg);
#endif
