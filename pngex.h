/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002 Andrea Mazzoleni
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

#ifndef __PNGEX_H
#define __PNGEX_H

#include "except.h"
#include "compress.h"
#include "file.h"
#include "data.h"

#include "lib/png.h"
#include "lib/error.h"

inline void throw_png_error()
{
	if (error_unsupported_get())
		throw error_unsupported() << error_get();
	else
		throw error() << error_get();
}

void png_print_chunk(unsigned type, unsigned char* data, unsigned size);

void png_compress(
	shrink_t level,
	data_ptr& out_ptr, unsigned& out_size,
	const unsigned char* img_ptr, unsigned img_scanline, unsigned img_pixel,
	unsigned x, unsigned y, unsigned dx, unsigned dy
);
void png_compress_delta(
	shrink_t level,
	data_ptr& out_ptr, unsigned& out_size,
	const unsigned char* img_ptr, unsigned img_scanline, unsigned img_pixel,
	const unsigned char* prev_ptr, unsigned prev_scanline,
	unsigned x, unsigned y, unsigned dx, unsigned dy
);
void png_compress_palette_delta(
	data_ptr& out_ptr, unsigned& out_size,
	const unsigned char* pal_ptr, unsigned pal_size,
	const unsigned char* prev_ptr
);
void png_write(
	adv_fz* f,
	unsigned pix_width, unsigned pix_height, unsigned pix_pixel,
	unsigned char* pix_ptr, unsigned pix_scanline,
	unsigned char* pal_ptr, unsigned pal_size,
	unsigned char* rns_ptr, unsigned rns_size,
	shrink_t level
);
void png_convert_4(
	unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline,
	unsigned char* pal_ptr, unsigned pal_size,
	unsigned char** dst_ptr, unsigned* dst_pixel, unsigned* dst_scanline
);
void png_convert_3(
	unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline,
	unsigned char* pal_ptr, unsigned pal_size,
	unsigned char** dst_ptr, unsigned* dst_pixel, unsigned* dst_scanline
);

#endif

