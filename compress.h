/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998-2002 Andrea Mazzoleni
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

#ifndef __COMPRESS_H
#define __COMPRESS_H

#ifdef USE_7Z
#include "7z/7z.h"
#endif

#ifdef USE_BZIP2
#include <bzlib.h>
#endif

#include <zlib.h>

// --------------------------------------------------------------------------
// Compression

bool decompress_deflate_zlib(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned out_size);
bool compress_deflate_zlib(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int compression_level, int strategy, int mem_level);

bool decompress_bzip2(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned out_size);
bool compress_bzip2(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int blocksize, int workfactor);

bool decompress_rfc1950_zlib(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned out_size);
bool compress_rfc1950_zlib(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int compression_level, int strategy, int mem_level);

// -------------------------------------------------------------------------
// Level

enum shrink_t {
	shrink_none,
	shrink_fast,
	shrink_normal,
	shrink_extra,
	shrink_extreme
};

#endif
