/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2003 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __SCROLL_H
#define __SCROLL_H

typedef struct adv_scroll_coord_struct {
	int x;
	int y;
} adv_scroll_coord;

typedef struct adv_scroll_info_struct {
	adv_scroll_coord* map; /**< Vector of scrolling shift. */
	unsigned mac; /**< Space used in the vector. */
	unsigned max; /**< Space allocated in the vector. */
	int x; /**< Start position of the first image in the scroll buffer. */
	int y;
	unsigned width; /**< Additional size of the scroll buffer. */
	unsigned height;
} adv_scroll_info;

typedef struct adv_scroll_struct  {
	adv_scroll_info* info; /**< Scrolling information. */
	unsigned char* pre_ptr; /**< Previous image. */
	unsigned char* cur_ptr; /**< Current image. */
	int range_dx; /**< Horizontal range to search. */
	int range_dy; /**< Vertical range to search. */
	int range_limit; /**< Sum range to search. */
} adv_scroll;

adv_scroll* scroll_init(int dx, int dy, int limit);
void scroll_done(adv_scroll* scroll);
void scroll_analyze(adv_scroll* scroll, unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline);
void scroll_last_get(adv_scroll* scroll, int* x, int* y);
adv_scroll_info* scroll_info_init(adv_scroll* scroll);
void scroll_info_done(adv_scroll_info* info);

#endif

