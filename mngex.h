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

#ifndef __MNGEX_H
#define __MNGEX_H

#include "pngex.h"

typedef enum adv_mng_type_enum {
	mng_vlc,
	mng_lc,
	mng_std
} adv_mng_type;

typedef struct adv_mng_write_struct {
	adv_bool first; /**< First image flag. */

	unsigned width; /**< Frame size in pixel. */
	unsigned height; /**< Frame size in pixel. */
	unsigned pixel; /**< Pixel size in bytes. */
	unsigned line; /**< Scanline size in bytes. */

	int scroll_x; /**< Initial position of the first image in the scroll buffer. */
	int scroll_y; /**< Initial position of the first image in the scroll buffer. */
	unsigned scroll_width; /**< Extra scroll buffer size in pixel. */
	unsigned scroll_height; /**< Extra scroll buffer size in pixel. */

	unsigned char* scroll_ptr; /**< Global image data. */

	unsigned char* current_ptr; /**< Pointer at the current frame in the scroll buffer. */
	int current_x; /**< Current scroll position. */
	int current_y;

	unsigned pal_size; /**< Palette size in bytes. */
	unsigned char pal_ptr[256*3]; /**< Palette. */

	unsigned tick; /**< Last tick used. */

	adv_mng_type type; /**< Type of the MNG stream. */
	shrink_t level; /**< Compression level of the MNG stream. */

	adv_bool reduce; /**< Try to reduce the images to 256 color. */
	adv_bool expand; /**< Expand the images to 24 bit color. */

	adv_bool header_written; /**< If the header was written. */
	unsigned header_simplicity; /**< Simplicity written in the header. */
} adv_mng_write;

adv_bool mng_write_has_header(adv_mng_write* mng);
void mng_write_header(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned width, unsigned height, unsigned frequency, int scroll_x, int scroll_y, unsigned scroll_width, unsigned scroll_height, adv_bool alpha);
void mng_write_image(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned width, unsigned height, unsigned pixel, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size, int shift_x, int shift_y);
void mng_write_frame(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned tick);
void mng_write_footer(adv_mng_write* mng, adv_fz* f, unsigned* fc);
adv_mng_write* mng_write_init(adv_mng_type type, shrink_t level, adv_bool reduce, adv_bool expand);
void mng_write_done(adv_mng_write* mng);

#endif

