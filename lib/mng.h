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
 * MNG file support.
 */

#ifndef __MNG_H
#define __MNG_H

#include "png.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \name ADV_MNG_CHUNK */
/*@{*/
#define ADV_MNG_CN_DHDR 0x44484452
#define ADV_MNG_CN_MHDR 0x4D484452
#define ADV_MNG_CN_MEND 0x4D454E44
#define ADV_MNG_CN_DEFI 0x44454649
#define ADV_MNG_CN_PPLT 0x50504c54
#define ADV_MNG_CN_MOVE 0x4d4f5645
#define ADV_MNG_CN_TERM 0x5445524d
#define ADV_MNG_CN_SAVE 0x53415645
#define ADV_MNG_CN_SEEK 0x5345454b
#define ADV_MNG_CN_LOOP 0x4c4f4f50
#define ADV_MNG_CN_ENDL 0x454e444c
#define ADV_MNG_CN_BACK 0x4241434b
#define ADV_MNG_CN_FRAM 0x4652414d
/*@}*/

/**
 * MNG context.
 */
typedef struct adv_mng_struct {
	int end_flag; /**< End flag. */
	unsigned pixel; /**< Bytes per pixel. */
	unsigned char* dat_ptr; /**< Current image buffer. */
	unsigned dat_size; /**< Size of the buffer image. */
	unsigned dat_line; /**< Bytes per scanline. */
	
	int dat_x; /**< X position of the displayed area in the working area. */
	int dat_y; /**< Y position of the displayed area in the working area. */
	unsigned dat_width; /**< Width of the working area. */
	unsigned dat_height; /**< height of the working area. */

	unsigned char* dlt_ptr; /**< Delta buffer. */
	unsigned dlt_size; /**< Delta buffer size. */
	unsigned dlt_line; /**< Delta bufer bytes per scanline. */

	unsigned char pal_ptr[256*3]; /**< Palette data. */
	unsigned pal_size; /**< Palette data size in bytes. */

	unsigned frame_frequency; /**< Base frame rate. */
	unsigned frame_tick; /**< Ticks for a generic frame. */
	unsigned frame_width; /**< Frame width. */
	unsigned frame_height; /**< Frame height. */
} adv_mng;

adv_error adv_mng_read_signature(adv_fz* f);
adv_error adv_mng_write_signature(adv_fz* f, unsigned* count);
adv_error adv_mng_write_mhdr(
	unsigned pix_width, unsigned pix_height,
	unsigned frequency, adv_bool is_lc,
	adv_fz* f, unsigned* count
);
adv_error adv_mng_write_mend(adv_fz* f, unsigned* count);
adv_error adv_mng_write_fram(unsigned tick, adv_fz* f, unsigned* count);

/** \addtogroup VideoFile */
/*@{*/

adv_mng* adv_mng_init(adv_fz* f);
void adv_mng_done(adv_mng* mng);
adv_error adv_mng_read(
	adv_mng* mng,
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned* tick,
	adv_fz* f
);
adv_error adv_mng_read_done(
	adv_mng* mng,
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned* tick,
	adv_fz* f
);
unsigned adv_mng_frequency_get(adv_mng* mng);
unsigned adv_mng_width_get(adv_mng* mng);
unsigned adv_mng_height_get(adv_mng* mng);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif

