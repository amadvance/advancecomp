/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004 Andrea Mazzoleni
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

/** \file
 * PNG file support.
 */

#ifndef __PNG_H
#define __PNG_H

#include "extra.h"
#include "fz.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \name PNG_CHUNK */
/*@{*/
#define ADV_PNG_CN_IHDR 0x49484452
#define ADV_PNG_CN_PLTE 0x504C5445
#define ADV_PNG_CN_IDAT 0x49444154
#define ADV_PNG_CN_IEND 0x49454E44
#define ADV_PNG_CN_tRNS 0x74524e53
/*@}*/

adv_error adv_png_read_chunk(adv_fz* f, unsigned char** data, unsigned* size, unsigned* type);
adv_error adv_png_write_chunk(adv_fz* f, unsigned type, const unsigned char* data, unsigned size, unsigned* count);

adv_error adv_png_read_signature(adv_fz* f);
adv_error adv_png_write_signature(adv_fz* f, unsigned* count);

adv_error adv_png_read_iend(adv_fz* f, const unsigned char* data, unsigned data_size, unsigned type);
adv_error adv_png_write_iend(adv_fz* f, unsigned* count);

adv_error adv_png_read_ihdr(
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned char** rns_ptr, unsigned* rns_size,
	adv_fz* f, const unsigned char* data, unsigned data_size
);

adv_error adv_png_write_ihdr(
	unsigned pix_width, unsigned pix_height,
	unsigned pix_depth, unsigned pix_type,
	adv_fz* f, unsigned* count
);

adv_error adv_png_write_idat(
	unsigned pix_width, unsigned pix_height, unsigned pix_pixel,
	const uint8* pix_ptr, int pix_pixel_pitch, int pix_scanline_pitch,
	adv_bool fast,
	adv_fz* f, unsigned* count
);

adv_error adv_png_write_raw(
	unsigned pix_width, unsigned pix_height, unsigned pix_pixel,
	const unsigned char* pix_ptr, int pix_pixel_pitch, int pix_scanline_pitch,
	const unsigned char* pal_ptr, unsigned pal_size,
	const unsigned char* rns_ptr, unsigned rns_size,
	adv_bool fast,
	adv_fz* f, unsigned* count
);

void adv_png_expand_4(unsigned width, unsigned height, unsigned char* ptr);
void adv_png_expand_2(unsigned width, unsigned height, unsigned char* ptr);
void adv_png_expand_1(unsigned width, unsigned height, unsigned char* ptr);
void adv_png_unfilter_8(unsigned width, unsigned height, unsigned char* ptr, unsigned line);
void adv_png_unfilter_24(unsigned width, unsigned height, unsigned char* ptr, unsigned line);
void adv_png_unfilter_32(unsigned width, unsigned height, unsigned char* ptr, unsigned line);

/** \addtogroup VideoFile */
/*@{*/

adv_error adv_png_read(
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	adv_fz* f
);

adv_error adv_png_read_rns(
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned char** rns_ptr, unsigned* rns_size,
	adv_fz* f
);

adv_error adv_png_write(
	unsigned pix_width, unsigned pix_height, unsigned pix_pixel,
	const unsigned char* pix_ptr, int pix_pixel_pitch, int pix_scanline_pitch,
	const unsigned char* pal_ptr, unsigned pal_size,
	adv_bool fast,
	adv_fz* f, unsigned* count
);

adv_error adv_png_write_rns(
	unsigned pix_width, unsigned pix_height, unsigned pix_pixel,
	const unsigned char* pix_ptr, int pix_pixel_pitch, int pix_scanline_pitch,
	const unsigned char* pal_ptr, unsigned pal_size,
	const unsigned char* rns_ptr, unsigned rns_size,
	adv_bool fast,
	adv_fz* f, unsigned* count
);

/*@}*/

#ifdef __cplusplus
};
#endif

#endif

