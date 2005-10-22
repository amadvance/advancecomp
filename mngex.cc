/*
 * This file is part of the AdvanceSCAN project.
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

#include "portable.h"

#include "mngex.h"

#include "lib/mng.h"
#include "lib/endianrw.h"

#include <iostream>
#include <iomanip>

using namespace std;

static bool mng_write_reduce(adv_mng_write* mng, data_ptr& out_ptr, unsigned& out_scanline, unsigned char* ovr_ptr, unsigned char* img_ptr, unsigned img_scanline)
{
	unsigned char col_ptr[256*3];
	unsigned col_mapped[256];
	unsigned col_count;
	adv_bool ovr_used[256];
	unsigned i, j, k;
	unsigned char* new_ptr;
	unsigned new_scanline;

	/* build the new palette */
	col_count = 0;
	for(i=0;i<mng->height;++i) {
		unsigned char* p0 = img_ptr + i * img_scanline;
		for(j=0;j<mng->width;++j) {
			for(k=0;k<col_count;++k) {
				if (col_ptr[k*3] == p0[0] && col_ptr[k*3+1] == p0[1] && col_ptr[k*3+2] == p0[2])
					break;
			}
			if (k == col_count) {
				if (col_count == 256)
					return false; /* too many colors */
				col_ptr[col_count*3] = p0[0];
				col_ptr[col_count*3+1] = p0[1];
				col_ptr[col_count*3+2] = p0[2];
				++col_count;
			}
			p0 += 3;
		}
	}

	for(i=0;i<256;++i) {
		ovr_used[i] = 0;
		col_mapped[i] = 0;
	}

	/* start from the last valid palette */
	memcpy(ovr_ptr, mng->pal_ptr, 256*3);

	/* map colors already present in the old palette */
	for(i=0;i<col_count;++i) {
		for(k=0;k<256;++k) {
			if (!ovr_used[k] && ovr_ptr[k*3]==col_ptr[i*3] && ovr_ptr[k*3+1]==col_ptr[i*3+1] && ovr_ptr[k*3+2]==col_ptr[i*3+2])
				break;
		}
		if (k<256) {
			ovr_used[k] = 1;
			col_mapped[i] = 1;
		}
	}

	/* map colors not present in the old palette */
	for(i=0;i<col_count;++i) {
		if (!col_mapped[i]) {
			/* search the first free space */
			for(k=0;k<256;++k)
				if (!ovr_used[k])
					break;
			ovr_used[k] = 1;
			ovr_ptr[k*3] = col_ptr[i*3];
			ovr_ptr[k*3+1] = col_ptr[i*3+1];
			ovr_ptr[k*3+2] = col_ptr[i*3+2];
		}
	}

	// create the new bitmap
	new_scanline = mng->width;
	new_ptr = data_alloc(mng->height * new_scanline);
	for(i=0;i<mng->height;++i) {
		unsigned char* p0 = new_ptr + i*new_scanline;
		unsigned char* p1 = img_ptr + i*img_scanline;
		for(j=0;j<mng->width;++j) {
			for(k=0;;++k)
				if (ovr_ptr[k*3]==p1[0] && ovr_ptr[k*3+1]==p1[1] && ovr_ptr[k*3+2]==p1[2])
					break;
			*p0 = k;
			++p0;
			p1 += 3;
		}
	}

	out_ptr = new_ptr;
	out_scanline = new_scanline;

	return true;
}

static void mng_write_expand(adv_mng_write* mng, data_ptr& out_ptr, unsigned& out_scanline, const unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr)
{
	unsigned char* new_ptr;
	unsigned new_scanline;
	unsigned i, j;

	/* create the new bitmap */
	new_scanline = mng->width * 3;
	new_ptr = data_alloc(mng->height * new_scanline);

	for(i=0;i<mng->height;++i) {
		unsigned char* p0 = new_ptr + i*new_scanline;
		const unsigned char* p1 = img_ptr + i*img_scanline;
		for(j=0;j<mng->width;++j) {
			unsigned k = 3 * *p1;
			p0[0] = pal_ptr[k];
			p0[1] = pal_ptr[k+1];
			p0[2] = pal_ptr[k+2];
			p0 += 3;
			++p1;
		}
	}

	out_ptr = new_ptr;
	out_scanline = new_scanline;
}

void mng_write_header(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned width, unsigned height, unsigned frequency, int scroll_x, int scroll_y, unsigned scroll_width, unsigned scroll_height, adv_bool alpha)
{
	unsigned simplicity;
	unsigned char mhdr[28];

	if (adv_mng_write_signature(f, fc) != 0) {
		throw_png_error();
	}

	switch (mng->type) {
	case mng_vlc :
		simplicity = (1 << 0) /* Enable flags */
			| (1 << 6); /* Enable flags */
		break;
	case mng_lc :
		simplicity = (1 << 0) /* Enable flags */
			| (1 << 1) /* Basic features */
			| (1 << 6); /* Enable flags */
		break;
	default:
	case mng_std :
		simplicity = (1 << 0) /* Enable flags */
			| (1 << 1) /* Basic features */
			| (1 << 2) /* Extended features */
			| (1 << 5) /* Delta PNG */
			| (1 << 6) /* Enable flags */
			| (1 << 9); /* Object buffers must be stored */
		break;
	}

	if (alpha) {
		simplicity |= (1 << 3) /* Internal transparency */
			| (1 << 8); /* Semi-transparency */
	}

	be_uint32_write(mhdr + 0, width); /* width */
	be_uint32_write(mhdr + 4, height); /* height */
	be_uint32_write(mhdr + 8, frequency); /* ticks per second */
	be_uint32_write(mhdr + 12, 0); /* nominal layer count */
	be_uint32_write(mhdr + 16, 0); /* nominal frame count */
	be_uint32_write(mhdr + 20, 0); /* nominal play time */
	be_uint32_write(mhdr + 24, simplicity); /* simplicity profile */

	if (adv_png_write_chunk(f, ADV_MNG_CN_MHDR, mhdr, sizeof(mhdr), fc) != 0) {
		throw_png_error();
	}

	mng->first = 1;

	mng->width = width;
	mng->height = height;
	mng->pixel = 0;
	mng->line = 0;

	mng->scroll_x = scroll_x;
	mng->scroll_y = scroll_y;
	mng->scroll_width = scroll_width;
	mng->scroll_height = scroll_height;

	mng->scroll_ptr = 0;

	mng->pal_size = 0;

	mng->tick = 1;

	mng->header_written = 1;
	mng->header_simplicity = simplicity;
}

static bool row_equal(adv_mng_write* mng, unsigned y, unsigned char* img_ptr, unsigned img_scanline)
{
	unsigned char* p0;
	unsigned char* p1;

	p0 = mng->current_ptr + y * mng->line;
	p1 = img_ptr + y * img_scanline;

	return memcmp(p0, p1, mng->width * mng->pixel) == 0;
}

static bool col_equal(adv_mng_write* mng, unsigned x, unsigned char* img_ptr, unsigned img_scanline)
{
	unsigned char* p0;
	unsigned char* p1;
	unsigned i;

	p0 = mng->current_ptr + x * mng->pixel;
	p1 = img_ptr + x * mng->pixel;

	if (mng->pixel == 1) {
		for(i=0;i<mng->height;++i) {
			if (p0[0] != p1[0])
				return false;
			p0 += mng->line;
			p1 += img_scanline;
		}
	} else if (mng->pixel == 3) {
		for(i=0;i<mng->height;++i) {
			if (p0[0] != p1[0] || p0[1] != p1[1] || p0[2] != p1[2])
				return false;
			p0 += mng->line;
			p1 += img_scanline;
		}
	} else if (mng->pixel == 4) {
		for(i=0;i<mng->height;++i) {
			if (p0[0] != p1[0] || p0[1] != p1[1] || p0[2] != p1[2] || p0[3] != p1[3])
				return false;
			p0 += mng->line;
			p1 += img_scanline;
		}
	} else {
		throw error() << "Unsupported bit depth";
	}

	return true;
}

static void compute_image_range(adv_mng_write* mng, unsigned* out_x, unsigned* out_y, unsigned* out_dx, unsigned* out_dy, unsigned char* img_ptr, unsigned img_scanline)
{
	unsigned x;
	unsigned dx;
	unsigned y;
	unsigned dy;

	x = 0;
	while (x < mng->width && col_equal(mng, x, img_ptr, img_scanline))
		++x;

	dx = mng->width - x;
	while (dx > 0 && col_equal(mng, x + dx - 1, img_ptr, img_scanline))
		--dx;

	y = 0;
	while (y < mng->height && row_equal(mng, y, img_ptr, img_scanline))
		++y;

	dy = mng->height - y;
	while (dy > 0 && row_equal(mng, y + dy - 1, img_ptr, img_scanline))
		--dy;

	*out_x = x;
	*out_y = y;
	*out_dx = dx;
	*out_dy = dy;
}

static void mng_write_store(adv_mng_write* mng, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size)
{
	unsigned i;

	if (pal_size) {
		memcpy(mng->pal_ptr, pal_ptr, pal_size);
		memset(mng->pal_ptr + pal_size, 0, 256*3 - pal_size);
		if (pal_size > mng->pal_size)
			mng->pal_size = pal_size;
	}

	for(i=0;i<mng->height;++i) {
		memcpy(&mng->current_ptr[i * mng->line], &img_ptr[i * img_scanline], mng->width * mng->pixel);
	}
}

static void mng_write_first(adv_mng_write* mng, adv_fz* f, unsigned* fc)
{
	unsigned char defi[12];
	unsigned defi_size;
	unsigned char ihdr[13];
	data_ptr z_ptr;
	unsigned z_size;

	if (mng->type == mng_std) {
		be_uint16_write(defi + 0, 1); /* object id */
		defi[2] = 0; /* visible */
		defi[3] = 1; /* concrete */
		if (mng->scroll_x!=0 || mng->scroll_y!=0) {
			be_uint32_write(defi + 4, - mng->scroll_x);
			be_uint32_write(defi + 8, - mng->scroll_y);
			defi_size = 12;
		} else
			defi_size = 4;
		if (adv_png_write_chunk(f, ADV_MNG_CN_DEFI, defi, defi_size, fc) != 0) {
			throw_png_error();
		}
	}

	be_uint32_write(ihdr + 0, mng->width + mng->scroll_width);
	be_uint32_write(ihdr + 4, mng->height + mng->scroll_height);
	ihdr[8] = 8; /* bit depth */
	if (mng->pixel == 1)
		ihdr[9] = 3; /* color type */
	else if (mng->pixel == 3)
		ihdr[9] = 2; /* color type */
	else if (mng->pixel == 4)
		ihdr[9] = 6; /* color type */
	ihdr[10] = 0; /* compression */
	ihdr[11] = 0; /* filter */
	ihdr[12] = 0; /* interlace */

	if (adv_png_write_chunk(f, ADV_PNG_CN_IHDR, ihdr, sizeof(ihdr), fc) != 0) {
		throw_png_error();
	}

	if (mng->pal_size) {
		if (adv_png_write_chunk(f, ADV_PNG_CN_PLTE, mng->pal_ptr, mng->pal_size, fc) != 0) {
			throw_png_error();
		}
	}

	png_compress(mng->level, z_ptr, z_size, mng->scroll_ptr, mng->line, mng->pixel, 0, 0, mng->width + mng->scroll_width, mng->height + mng->scroll_height);

	if (adv_png_write_chunk(f, ADV_PNG_CN_IDAT, z_ptr, z_size, fc) != 0) {
		throw_png_error();
	}

	if (adv_png_write_chunk(f, ADV_PNG_CN_IEND, 0, 0, fc) != 0) {
		throw_png_error();
	}
}

static void mng_write_move(adv_mng_write* mng, adv_fz* f, unsigned* fc, int shift_x, int shift_y)
{
	unsigned char move[13];

	if (shift_x!=0 || shift_y!=0) {

		be_uint16_write(move + 0, 1); /* id 1 */
		be_uint16_write(move + 2, 1); /* id 1 */
		move[4] = 1; /* adding */
		be_uint32_write(move + 5, - shift_x);
		be_uint32_write(move + 9, - shift_y);

		if (adv_png_write_chunk(f, ADV_MNG_CN_MOVE, move, sizeof(move), fc)!=0) {
			throw_png_error();
		}
	}
}

static void mng_write_delta_image(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size)
{
	unsigned x, y, dx, dy;
	data_ptr pal_d_ptr;
	unsigned pal_d_size;
	data_ptr pal_r_ptr;
	unsigned pal_r_size;
	data_ptr z_d_ptr;
	unsigned z_d_size;
	data_ptr z_r_ptr;
	unsigned z_r_size;
	unsigned char dhdr[20];
	unsigned dhdr_size;

	if (pal_ptr && pal_size) {
		png_compress_palette_delta(pal_d_ptr, pal_d_size, pal_ptr, pal_size, mng->pal_ptr);

		pal_r_ptr = data_dup(pal_ptr, pal_size);
                pal_r_size = pal_size;
	} else {
		pal_d_ptr = 0;
		pal_d_size = 0;
		pal_r_ptr = 0;
		pal_r_size = 0;
	}

	compute_image_range(mng, &x, &y, &dx, &dy, img_ptr, img_scanline);

	if (dx && dy) {
		png_compress_delta(mng->level, z_d_ptr, z_d_size, img_ptr, img_scanline, mng->pixel, mng->current_ptr, mng->line, x, y, dx, dy);
		png_compress(mng->level, z_r_ptr, z_r_size, img_ptr, img_scanline, mng->pixel, x, y, dx, dy);
	} else {
		z_d_ptr = 0;
		z_d_size = 0;
		z_r_ptr = 0;
		z_r_size = 0;
	}

	be_uint16_write(dhdr + 0, 1); /* object id */
	dhdr[2] = 1; /* png image */
	if (z_d_size) {
		if (z_d_size < z_r_size) {
			dhdr[3] = 1; /* block pixel addition */
			dhdr_size = 20;
		} else {
			if (dx == mng->width && dy == mng->height && mng->scroll_width == 0 && mng->scroll_height == 0) {
				dhdr[3] = 0; /* entire image replacement */
				dhdr_size = 12;
			} else {
				dhdr[3] = 4; /* block pixel replacement */
				dhdr_size = 20;
			}
		}
	} else {
		dhdr[3] = 7; /* no change to pixel data */
		dhdr_size = 4;
	}

	be_uint32_write(dhdr + 4, dx);
	be_uint32_write(dhdr + 8, dy);
	be_uint32_write(dhdr + 12, x + mng->current_x);
	be_uint32_write(dhdr + 16, y + mng->current_y);

	if (adv_png_write_chunk(f, ADV_MNG_CN_DHDR, dhdr, dhdr_size, fc) != 0) {
		throw_png_error();
	}

	if (pal_d_size) {
		if (pal_d_size < pal_r_size) {
			if (adv_png_write_chunk(f, ADV_MNG_CN_PPLT, pal_d_ptr, pal_d_size, fc) != 0) {
				throw_png_error();
			}
		} else {
			if (adv_png_write_chunk(f, ADV_PNG_CN_PLTE, pal_r_ptr, pal_r_size, fc) != 0) {
				throw_png_error();
			}
		}
	}

	if (z_d_size) {
		if (z_d_size < z_r_size) {
			if (adv_png_write_chunk(f, ADV_PNG_CN_IDAT, z_d_ptr, z_d_size, fc) != 0) {
				throw_png_error();
			}
		} else {
			if (adv_png_write_chunk(f, ADV_PNG_CN_IDAT, z_r_ptr, z_r_size, fc) != 0) {
				throw_png_error();
			}
		}
	}

	if (adv_png_write_chunk(f, ADV_PNG_CN_IEND, 0, 0, fc) != 0) {
		throw_png_error();
	}

	mng_write_store(mng, img_ptr, img_scanline, pal_ptr, pal_size);
}

static void mng_write_base_image(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size)
{
	data_ptr pal_r_ptr;
	unsigned pal_r_size;
	data_ptr z_r_ptr;
	unsigned z_r_size;

	pal_r_ptr = data_dup(pal_ptr, pal_size);
        pal_r_size = pal_size;

	png_compress(mng->level, z_r_ptr, z_r_size, img_ptr, img_scanline, mng->pixel, 0, 0, mng->width, mng->height);

	unsigned char ihdr[13];
	unsigned ihdr_size;

	be_uint32_write(ihdr + 0, mng->width);
	be_uint32_write(ihdr + 4, mng->height);
	ihdr[8] = 8; /* bit depth */
	if (mng->pixel == 1)
		ihdr[9] = 3; /* color type */
	else if (mng->pixel == 3)
		ihdr[9] = 3; /* color type */
	else if (mng->pixel == 4)
		ihdr[9] = 6; /* color type */
	else
		throw error() << "Unsupported bit depth";
	ihdr[10] = 0; /* compression */
	ihdr[11] = 0; /* filter */
	ihdr[12] = 0; /* interlace */
	ihdr_size = 13;

	if (adv_png_write_chunk(f, ADV_PNG_CN_IHDR, ihdr, ihdr_size, fc) != 0) {
		throw_png_error();
	}

	if (pal_r_size) {
		if (adv_png_write_chunk(f, ADV_PNG_CN_PLTE, pal_r_ptr, pal_r_size, fc) != 0) {
			throw_png_error();
		}
	}

	if (adv_png_write_chunk(f, ADV_PNG_CN_IDAT, z_r_ptr, z_r_size, fc) != 0) {
		throw_png_error();
	}

	if (adv_png_write_chunk(f, ADV_PNG_CN_IEND, 0, 0, fc) != 0) {
		throw_png_error();
	}

	mng_write_store(mng, img_ptr, img_scanline, pal_ptr, pal_size);
}

static void mng_write_image_setup(adv_mng_write* mng, adv_fz* f, unsigned width, unsigned height, unsigned pixel)
{
	if (!mng->header_written) {
		throw error() << "You must write an header before the first image";
	}

	if (width != mng->width) {
		throw error() << "Invalid width change";
	}

	if (height != mng->height) {
		throw error() << "Invalid height change";
	}

	if (mng->first) {
		switch (pixel) {
		case 1 :
			if (mng->expand)
				mng->pixel = 3;
			else
				mng->pixel = 1;
			break;
		case 3 :
			if (mng->reduce)
				mng->pixel = 1;
			else
				mng->pixel = 3;
			break;
		default:
			mng->pixel = pixel;
			break;
		}

		mng->line = mng->pixel * (mng->width + mng->scroll_width);
		mng->scroll_ptr = data_alloc((mng->height + mng->scroll_height) * mng->line);
		mng->current_ptr = mng->scroll_ptr + mng->scroll_x * mng->pixel + mng->scroll_y * mng->line;
		mng->current_x = mng->scroll_x;
		mng->current_y = mng->scroll_y;

		memset(mng->scroll_ptr, 0, (mng->height + mng->scroll_height) * mng->line);
		memset(mng->pal_ptr, 0, sizeof(mng->pal_ptr));
	}
}

static void mng_write_image_raw(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned width, unsigned height, unsigned pixel, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size, int shift_x, int shift_y)
{
	if (pixel != mng->pixel) {
		throw error() << "Invalid bit depth change";
	}

	if (mng->first) {
		mng->first = 0;

		if (shift_x != 0 || shift_y != 0) {
			throw error() << "Internal error";
		}

		mng_write_store(mng, img_ptr, img_scanline, pal_ptr, pal_size);
		mng_write_first(mng, f, fc);
	} else {
		// shift may be negative, caste all to int to prevent conversion to 64 bit unsigned int
		mng->current_ptr += shift_x * (int)mng->pixel + shift_y * (int)mng->line;

		mng->current_x += shift_x;
		mng->current_y += shift_y;

		if (mng->type == mng_std) {
			mng_write_move(mng, f, fc, shift_x, shift_y);
			mng_write_delta_image(mng, f, fc, img_ptr, img_scanline, pal_ptr,
 pal_size);
		} else {
			mng_write_base_image(mng, f, fc, img_ptr, img_scanline, pal_ptr, pal_size);
		}
	}
}

void mng_write_image(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned width, unsigned height, unsigned pixel, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size, int shift_x, int shift_y)
{
	mng_write_image_setup(mng, f, width, height, pixel);

	if (pixel == 1) {
		data_ptr new_ptr;
		unsigned new_scanline;

		if (!mng->expand) {
			mng_write_image_raw(mng, f, fc, width, height, pixel, img_ptr, img_scanline, pal_ptr, pal_size, shift_x, shift_y);
		} else {
			mng_write_expand(mng, new_ptr, new_scanline, img_ptr, img_scanline, pal_ptr);
			mng_write_image_raw(mng, f, fc, width, height, 3, new_ptr, new_scanline, 0, 0, shift_x, shift_y);
		}
	} else if (pixel == 3) {
		if (!mng->reduce) {
			mng_write_image_raw(mng, f, fc, width, height, pixel, img_ptr, img_scanline, 0, 0, shift_x, shift_y);
		} else {
			unsigned char ovr_ptr[256*3];
			data_ptr new_ptr;
			unsigned new_scanline;

			if (!mng_write_reduce(mng, new_ptr, new_scanline, ovr_ptr, img_ptr, img_scanline)) {
				throw error_unsupported() << "Color reduction failed";
			} else {
				mng_write_image_raw(mng, f, fc, width, height, 1, new_ptr, new_scanline, ovr_ptr, 256*3, shift_x, shift_y);

				// update the last palette used
				memcpy(mng->pal_ptr, ovr_ptr, sizeof(mng->pal_ptr));
			}
		}
	} else if (pixel == 4) {
		unsigned required = (1 << 3) | (1 << 6) | (1 << 8);
		if ((mng->header_simplicity & required) != required) {
			throw error() << "You enable transparency for alpha images";
		}

		mng_write_image_raw(mng, f, fc, width, height, pixel, img_ptr, img_scanline, 0, 0, shift_x, shift_y);
	} else {
		throw error_unsupported() << "Unsupported bit depth";
	} 
}

void mng_write_frame(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned tick)
{
	unsigned char fram[10];
	unsigned fram_size;

	if (tick == mng->tick) {
		fram[0] = 1;
		fram_size = 1;
	} else {
		fram[0] = 1;
		fram[1] = 0; /* no name */
		fram[2] = 2; /* change delay for all the next frames */
		fram[3] = 0; /* don't change timeout */
		fram[4] = 0; /* don't change clipping */
		fram[5] = 0; /* don't change sync id list */
		be_uint32_write(fram + 6, tick);
		fram_size = 10;
	}

	mng->tick = tick;

	if (adv_png_write_chunk(f, ADV_MNG_CN_FRAM, fram, fram_size, fc) != 0) {
		throw_png_error();
	}
}


void mng_write_footer(adv_mng_write* mng, adv_fz* f, unsigned* fc)
{
	if (adv_png_write_chunk(f, ADV_MNG_CN_MEND, 0, 0, fc) != 0) {
		throw_png_error();
	}
}

adv_mng_write* mng_write_init(adv_mng_type type, shrink_t level, adv_bool reduce, adv_bool expand)
{
	adv_mng_write* mng;

	mng = (adv_mng_write*)malloc(sizeof(adv_mng_write));

	mng->type = type;
	mng->level = level;
	mng->reduce = reduce;
	mng->expand = expand;
	mng->header_written = 0;
	mng->header_simplicity = 0;

	return mng;
}

void mng_write_done(adv_mng_write* mng)
{
	free(mng->scroll_ptr);
	free(mng);
}

adv_bool mng_write_has_header(adv_mng_write* mng)
{
	return mng->header_written;
}
