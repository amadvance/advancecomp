/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
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

#include "pngex.h"
#include "except.h"
#include "utility.h"
#include "compress.h"
#include "siglock.h"

#include "lib/endianrw.h"
#include "lib/mng.h"

#include <iostream>
#include <iomanip>

#include <cstdio>

#include <unistd.h>

using namespace std;

// --------------------------------------------------------------------------
// Static

int opt_dx;
int opt_dy;
bool opt_reduce;
bool opt_expand;
shrink_t opt_level;
bool opt_quiet;
bool opt_scroll;
bool opt_lc;
bool opt_vlc;
bool opt_force;

/*************************************************************************************/
/* Compare */

unsigned compare_single1(unsigned width, unsigned height, unsigned char* p0, unsigned char* p1, unsigned line) {
	unsigned i,j;
	unsigned count = 0;

	for(i=0;i<height;++i) {
#if 0
		for(j=0;j<width;++j) {
			if (p0[0] == p1[0])
				++count;
			++p0;
			++p1;
		}
#else
		assert( sizeof(unsigned) == 4 );
		j = width;
		while (j >= 4) {
			unsigned v0 = *(unsigned*)p0;
			unsigned v1 = *(unsigned*)p1;
			v0 ^= v1;
			if (v0) {
				if (v0 & 0xFF)
					++count;
				if (v0 & 0xFF00)
					++count;
				if (v0 & 0xFF0000)
					++count;
				if (v0 & 0xFF000000)
					++count;
			} else {
				count += 4;
			}
			p0 += 4;
			p1 += 4;
			j -= 4;
		}
		while (j > 0) {
			if (p0[0] == p1[0])
				++count;
			++p0;
			++p1;
			--j;
		}
#endif
		p0 += line - width;
		p1 += line - width;
	}

	return count;
}

unsigned compare_single3(unsigned width, unsigned height, unsigned char* p0, unsigned char* p1, unsigned line) {
	unsigned i,j;
	unsigned count = 0;

	for(i=0;i<height;++i) {
		for(j=0;j<width;++j) {
			if (p0[0] == p1[0] && p0[1] == p1[1] && p0[2] == p1[2])
				++count;
			p0 += 3;
			p1 += 3;
		}
		p0 += line - width * 3;
		p1 += line - width * 3;
	}

	return count;
}

unsigned compare_shift(int x, int y, unsigned width, unsigned height, unsigned char* p0, unsigned char* p1, unsigned pixel, unsigned line) {
	unsigned count;
	int dx = width - abs(x);
	int dy = height - abs(y);

	if (x < 0)
		p1 += -x * pixel;
	else
		p0 += x * pixel;

	if (y < 0)
		p1 += -y * line;
	else
		p0 += y * line;

	if (pixel == 1)
		count = compare_single1(dx, dy, p0, p1, line);
	else
		count = compare_single3(dx, dy, p0, p1, line);

	return count;
}

void compare(int* x, int* y, unsigned width, unsigned height, unsigned char* p0, unsigned char* p1, unsigned pixel, unsigned line) {
	int i,j;
	int best_x;
	int best_y;
	unsigned best_count;
	int prev_x;
	int prev_y;
	unsigned prev_count;
	unsigned center_count;
	unsigned total;

	best_x = 0;
	best_y = 0;
	best_count = compare_shift(0, 0, width, height, p0, p1, pixel, line);

	prev_count = 0;
	prev_x = 0;
	prev_y = 0;

	center_count = best_count;

	total = width * height;

	for(i=-opt_dx;i<=opt_dx;++i) {
	for(j=-opt_dy;j<=opt_dy;++j) {
	if (j || i) {
		unsigned count;

		count = compare_shift(i, j, width, height, p0, p1, pixel, line);
		if (count > best_count) {
			prev_count = best_count;
			prev_x = best_x;
			prev_y = best_y;
			best_count = count;
			best_x = i;
			best_y = j;
		} else if (count > prev_count) {
			prev_count = count;
			prev_x = i;
			prev_y = j;
		}
	}
	}
	}

	/* if too small difference than fixed image don't move */
	if (best_count < total / 3) {
		// printf("disable, best %3d %3d %6d, 2'nd %3d %3d %6d, fix %6d\n", best_x, best_y, best_count * 100 / total, prev_x, prev_y, prev_count * 100 / total, center_count * 100 / total);

		*x = 0;
		*y = 0;
	} else {
		// printf("best %3d %3d %6d, 2'nd %3d %3d %6d, fix %6d\n", best_x, best_y, best_count * 100 / total, prev_x, prev_y, prev_count * 100 / total, center_count * 100 / total);

		*x = best_x;
		*y = best_y;
	}
}

/*************************************************************************************/
/* Analyze */

struct scroll_coord {
	int x;
	int y;
};

struct scroll {
	struct scroll_coord* map; /* vector of scrolling shift */
	unsigned mac; /* space used */
	unsigned max; /* space allocated */
	int x; /* start position of the first image in the scroll buffer */
	int y;
	unsigned width; /* additional size of the scroll buffer */
	unsigned height;
};

struct analyze {
	unsigned char* pre_ptr; /* previous image */
	unsigned char* cur_ptr; /* current image */
} ANALYZE;

void analyze_insert(struct scroll* sc, int x, int y) {
	if (sc->mac == sc->max) {
		sc->max *= 2;
		if (!sc->max)
			sc->max = 64;
		sc->map = (struct scroll_coord*)realloc(sc->map, sc->max * sizeof(struct scroll_coord));
	}

	sc->map[sc->mac].x = x;
	sc->map[sc->mac].y = y;
	++sc->mac;
}

void analyze_image(adv_fz* f, struct scroll* sc, unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline) {
	unsigned char* ptr;
	unsigned scanline;
	unsigned i;

	scanline = pix_width * pix_pixel;
	ptr = data_alloc(pix_height * scanline);

	for(i=0;i<pix_height;++i) {
		memcpy(ptr + i*scanline, pix_ptr + i*pix_scanline, scanline);
	}

	data_free(ANALYZE.pre_ptr);
	ANALYZE.pre_ptr = ANALYZE.cur_ptr;
	ANALYZE.cur_ptr = ptr;

	if (ANALYZE.pre_ptr && ANALYZE.cur_ptr) {
		int x;
		int y;
		compare(&x, &y, pix_width, pix_height, ANALYZE.pre_ptr, ANALYZE.cur_ptr, pix_pixel, scanline);
		analyze_insert(sc,x,y);
	} else {
		analyze_insert(sc,0,0);
	}
}

void analyze_range(struct scroll* sc) {
	int px = 0;
	int py = 0;
	int min_x = 0;
	int min_y = 0;
	int max_x = 0;
	int max_y = 0;
	unsigned i;

	for(i=0;i<sc->mac;++i) {
		px += sc->map[i].x;
		py += sc->map[i].y;
		if (px < min_x)
			min_x = px;
		if (py < min_y)
			min_y = py;
		if (px > max_x)
			max_x = px;
		if (py > max_y)
			max_y = py;
	}

	sc->x = -min_x;
	sc->y = -min_y;
	sc->width = max_x - min_x;
	sc->height = max_y - min_y;
}

void analyze_f(adv_fz* f, struct scroll* sc) {
	data_ptr data;
	unsigned counter;
	adv_mng* mng;
	int dx = 0;
	int dy = 0;

	mng = mng_init(f);

	ANALYZE.pre_ptr = 0;
	ANALYZE.cur_ptr = 0;

	sc->map = 0;
	sc->mac = 0;
	sc->max = 0;

	counter = 0;

	while (1) {
		unsigned pix_width;
		unsigned pix_height;
		unsigned char* pix_ptr;
		unsigned pix_pixel;
		unsigned pix_scanline;
		unsigned char* dat_ptr;
		unsigned dat_size;
		unsigned char* pal_ptr;
		unsigned pal_size;
		unsigned tick;
		int r;

		r = mng_read(mng, &pix_width, &pix_height, &pix_pixel, &dat_ptr, &dat_size, &pix_ptr, &pix_scanline, &pal_ptr, &pal_size, &tick, f);
		if (r < 0) {
			throw error_png();
		}
		if (r > 0)
			break;

		try {
			analyze_image(f, sc, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline);
		} catch (...) {
			free(dat_ptr);
			free(pal_ptr);
			throw;
		}

		free(dat_ptr);
		free(pal_ptr);

		++counter;
		if (!opt_quiet) {
			int x = sc->map[sc->mac-1].x;
			int y = sc->map[sc->mac-1].y;
			if (dx < abs(x))
				dx = abs(x);
			if (dy < abs(y))
				dy = abs(y);
			cout << "Analyzing frame " << counter << ", delta " << x << "x" << y << " [" << dx << "x" << dy << "]    \r";
			cout.flush();
		}
	}

	mng_done(mng);

	if (!opt_quiet) {
		// clear
		cout << "                                                    \r";
	}

	data_free(ANALYZE.pre_ptr);
	data_free(ANALYZE.cur_ptr);

	analyze_range(sc);
}

void analyze_file(const string& path, struct scroll* sc) {
	adv_fz* f;

	f = fzopen(path.c_str(),"rb");
	if (!f) {
		throw error() << "Failed open for reading " << path;
	}

	try {
		analyze_f(f,sc);
	} catch (...) {
		fzclose(f);
		throw;
	}

	fzclose(f);
}

/*************************************************************************************/
/* Write */

struct write {
	int first; /* first image flag */

	unsigned width; /* frame size in pixel */
	unsigned height;
	unsigned pixel; /* pixel size in bytes */
	unsigned line; /* line size in bytes */

	int scroll_x; /* initial position of the first image in the scroll buffer */
	int scroll_y;
	unsigned scroll_width; /* extra scroll buffer size */
	unsigned scroll_height;

	unsigned char* scroll_ptr; /* global image data */

	unsigned char* current_ptr; /* pointer at the current frame in the scroll buffer */
	int current_x; /* current scroll position */
	int current_y;

	unsigned pal_size; /* palette size in bytes */
	unsigned char pal_ptr[256*3]; /* palette */
	int pal_used[256]; /* flag vector for the used entries in the palette */

	unsigned tick; /* last tick used */
} WRITE;

bool reduce_image(data_ptr out_ptr, unsigned& out_scanline, unsigned char* ovr_ptr, unsigned* ovr_used, unsigned char* img_ptr, unsigned img_scanline) {
	unsigned char col_ptr[256*3];
	unsigned col_mapped[256];
	unsigned col_count;
	unsigned i,j,k;
	unsigned char* new_ptr;
	unsigned new_scanline;

	/* build the new palette */
	col_count = 0;
	for(i=0;i<WRITE.height;++i) {
		unsigned char* p0 = img_ptr + i * img_scanline;
		for(j=0;j<WRITE.width;++j) {
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
	memcpy(ovr_ptr, WRITE.pal_ptr, 256*3);

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

	/* create the new bitmap */
	new_scanline = WRITE.width;
	new_ptr = data_alloc(WRITE.height * new_scanline);
	for(i=0;i<WRITE.height;++i) {
		unsigned char* p0 = new_ptr + i*new_scanline;
		unsigned char* p1 = img_ptr + i*img_scanline;
		for(j=0;j<WRITE.width;++j) {
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

void write_expand(data_ptr out_ptr, unsigned& out_scanline, const unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr) {
	unsigned char* new_ptr;
	unsigned new_scanline;
	unsigned i,j;

	/* create the new bitmap */
	new_scanline = WRITE.width * 3;
	new_ptr = data_alloc(WRITE.height * new_scanline);

	for(i=0;i<WRITE.height;++i) {
		unsigned char* p0 = new_ptr + i*new_scanline;
		const unsigned char* p1 = img_ptr + i*img_scanline;
		for(j=0;j<WRITE.width;++j) {
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

void write_header(adv_fz* f, unsigned* fc, unsigned width, unsigned height, unsigned frequency, int scroll_x, int scroll_y, unsigned scroll_width, unsigned scroll_height) {
	unsigned simplicity;
	unsigned char mhdr[28];

	if (mng_write_signature(f,fc) != 0) {
		throw error_png();
	}

	if (opt_vlc) {
		simplicity = (1 << 0) /* Enable flags */
			| (1 << 6); /* Enable flags */
	} else if (opt_lc) {
		simplicity = (1 << 0) /* Enable flags */
			| (1 << 1) /* Basic features */
			| (1 << 6); /* Enable flags */
	} else {
		simplicity = (1 << 0) /* Enable flags */
			| (1 << 1) /* Basic features */
			| (1 << 2) /* Extended features */
			| (1 << 5) /* Delta PNG */
			| (1 << 6) /* Enable flags */
			| (1 << 9); /* Object buffers must be stored */
	}

	be_uint32_write(mhdr + 0, width); /* width */
	be_uint32_write(mhdr + 4, height); /* height */
	be_uint32_write(mhdr + 8, frequency ); /* ticks per second */
	be_uint32_write(mhdr + 12, 0 ); /* nominal layer count */
	be_uint32_write(mhdr + 16, 0 ); /* nominal frame count */
	be_uint32_write(mhdr + 20, 0 ); /* nominal play time */
	be_uint32_write(mhdr + 24, simplicity); /* simplicity profile */

	if (png_write_chunk(f, MNG_CN_MHDR, mhdr, sizeof(mhdr), fc) != 0) {
		throw error_png();
	}

	WRITE.first = 1;

	WRITE.width = width;
	WRITE.height = height;
	WRITE.pixel = 0;
	WRITE.line = 0;

	WRITE.scroll_x = scroll_x;
	WRITE.scroll_y = scroll_y;
	WRITE.scroll_width = scroll_width;
	WRITE.scroll_height = scroll_height;

	WRITE.scroll_ptr = 0;

	WRITE.pal_size = 0;

	WRITE.tick = 1;
}

bool row_equal(unsigned y, unsigned char* img_ptr, unsigned img_scanline) {
	unsigned char* p0;
	unsigned char* p1;

	p0 = WRITE.current_ptr + y * WRITE.line;
	p1 = img_ptr + y * img_scanline;

	return memcmp(p0, p1, WRITE.width * WRITE.pixel) == 0;
}

bool col_equal(unsigned x, unsigned char* img_ptr, unsigned img_scanline) {
	unsigned char* p0;
	unsigned char* p1;
	unsigned i;

	p0 = WRITE.current_ptr + x * WRITE.pixel;
	p1 = img_ptr + x * WRITE.pixel;

	if (WRITE.pixel == 1) {
		for(i=0;i<WRITE.height;++i) {
			if (p0[0] != p1[0])
				return false;
			p0 += WRITE.line;
			p1 += img_scanline;
		}
	} else {
		for(i=0;i<WRITE.height;++i) {
			if (p0[0] != p1[0] || p0[1] != p1[1] || p0[2] != p1[2])
				return false;
			p0 += WRITE.line;
			p1 += img_scanline;
		}
	}

	return true;
}

void compute_image_range(unsigned* out_x, unsigned* out_y, unsigned* out_dx, unsigned* out_dy, unsigned char* img_ptr, unsigned img_scanline) {
	unsigned x;
	unsigned dx;
	unsigned y;
	unsigned dy;

	x = 0;
	while (x < WRITE.width && col_equal(x, img_ptr, img_scanline))
		++x;

	dx = WRITE.width - x;
	while (dx > 0 && col_equal(x + dx - 1, img_ptr, img_scanline))
		--dx;

	y = 0;
	while (y < WRITE.height && row_equal(y, img_ptr, img_scanline))
		++y;

	dy = WRITE.height - y;
	while (dy > 0 && row_equal(y + dy - 1, img_ptr, img_scanline))
		--dy;

	*out_x = x;
	*out_y = y;
	*out_dx = dx;
	*out_dy = dy;
}

void write_store(unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size) {
	unsigned i;

	if (pal_size) {
		memcpy(WRITE.pal_ptr, pal_ptr, pal_size);
		memset(WRITE.pal_ptr + pal_size, 0, 256*3 - pal_size);
		if (pal_size > WRITE.pal_size)
			WRITE.pal_size = pal_size;
	}

	for(i=0;i<WRITE.height;++i) {
		memcpy( &WRITE.current_ptr[i * WRITE.line], &img_ptr[i * img_scanline], WRITE.width * WRITE.pixel);
	}
}

void write_first(adv_fz* f, unsigned* fc) {
	unsigned char defi[12];
	unsigned defi_size;
	unsigned char ihdr[13];
	data_ptr z_ptr;
	unsigned z_size;

	if (!opt_lc && !opt_vlc) {
		be_uint16_write(defi + 0, 1); /* object id */
		defi[2] = 0; /* visible */
		defi[3] = 1; /* concrete */
		if (WRITE.scroll_x!=0 || WRITE.scroll_y!=0) {
			be_uint32_write(defi + 4, - WRITE.scroll_x);
			be_uint32_write(defi + 8, - WRITE.scroll_y);
			defi_size = 12;
		} else
			defi_size = 4;
		if (png_write_chunk(f, MNG_CN_DEFI, defi, defi_size, fc) != 0) {
			throw error_png();
		}
	}

	be_uint32_write(ihdr + 0, WRITE.width + WRITE.scroll_width);
	be_uint32_write(ihdr + 4, WRITE.height + WRITE.scroll_height);
	ihdr[8] = 8; /* bit depth */
	if (WRITE.pixel == 1)
		ihdr[9] = 3; /* color type */
	else
		ihdr[9] = 2; /* color type */
	ihdr[10] = 0; /* compression */
	ihdr[11] = 0; /* filter */
	ihdr[12] = 0; /* interlace */

	if (png_write_chunk(f, PNG_CN_IHDR, ihdr, sizeof(ihdr), fc) != 0) {
		throw error_png();
	}

	if (WRITE.pal_size) {
		if (png_write_chunk(f, PNG_CN_PLTE, WRITE.pal_ptr, WRITE.pal_size, fc) != 0) {
			throw error_png();
		}
	}

	png_compress(opt_level, z_ptr, z_size, WRITE.scroll_ptr, WRITE.line, WRITE.pixel, 0, 0, WRITE.width + WRITE.scroll_width, WRITE.height + WRITE.scroll_height);

	if (png_write_chunk(f, PNG_CN_IDAT, z_ptr, z_size, fc) != 0) {
		throw error_png();
	}

	if (png_write_chunk(f, PNG_CN_IEND, 0, 0, fc) != 0) {
		throw error_png();
	}
}

void write_move(adv_fz* f, unsigned* fc, int shift_x, int shift_y) {
	unsigned char move[13];

	if (shift_x!=0 || shift_y!=0) {

		be_uint16_write(move + 0, 1); /* id 1 */
		be_uint16_write(move + 2, 1); /* id 1 */
		move[4] = 1; /* adding */
		be_uint32_write(move + 5, - shift_x);
		be_uint32_write(move + 9, - shift_y);

		if (png_write_chunk(f, MNG_CN_MOVE, move, sizeof(move), fc)!=0) {
			throw error_png();
		}
	}
}

void write_delta_image(adv_fz* f, unsigned* fc, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size) {
	unsigned x,y,dx,dy;
	data_ptr pal_d_ptr;
	unsigned pal_d_size;
	data_ptr pal_r_ptr;
	unsigned pal_r_size;
	data_ptr z_d_ptr;
	unsigned z_d_size;
	data_ptr z_r_ptr;
	unsigned z_r_size;

	if (pal_ptr && pal_size) {
		png_compress_palette_delta(pal_d_ptr, pal_d_size, pal_ptr, pal_size, WRITE.pal_ptr);

		pal_r_ptr = data_dup(pal_ptr, pal_size);
                pal_r_size = pal_size;
	} else {
		pal_d_ptr = 0;
		pal_d_size = 0;
		pal_r_ptr = 0;
		pal_r_size = 0;
	}

	compute_image_range(&x, &y, &dx, &dy, img_ptr, img_scanline);

	if (dx && dy) {
		png_compress_delta(opt_level, z_d_ptr, z_d_size, img_ptr, img_scanline, WRITE.pixel, WRITE.current_ptr, WRITE.line, x, y, dx, dy);
		png_compress(opt_level, z_r_ptr, z_r_size, img_ptr, img_scanline, WRITE.pixel, x, y, dx, dy);
	} else {
		z_d_ptr = 0;
		z_d_size = 0;
		z_r_ptr = 0;
		z_r_size = 0;
	}

	unsigned char dhdr[20];
	unsigned dhdr_size;

	be_uint16_write(dhdr + 0, 1); /* object id */
	dhdr[2] = 1; /* png image */
	if (z_d_size) {
		if (z_d_size < z_r_size) {
			dhdr[3] = 1; /* block pixel addition */
			dhdr_size = 20;
		} else {
			if (dx == WRITE.width && dy == WRITE.height && WRITE.scroll_width == 0 && WRITE.scroll_height == 0) {
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
	be_uint32_write(dhdr + 12, x + WRITE.current_x);
	be_uint32_write(dhdr + 16, y + WRITE.current_y);

	if (png_write_chunk(f, MNG_CN_DHDR, dhdr, dhdr_size, fc) != 0) {
		throw error_png();
	}

	if (pal_d_size) {
		if (pal_d_size < pal_r_size) {
			if (png_write_chunk(f, MNG_CN_PPLT, pal_d_ptr, pal_d_size, fc) != 0) {
				throw error_png();
			}
		} else {
			if (png_write_chunk(f, PNG_CN_PLTE, pal_r_ptr, pal_r_size, fc) != 0) {
				throw error_png();
			}
		}
	}

	if (z_d_size) {
		if (z_d_size < z_r_size) {
			if (png_write_chunk(f, PNG_CN_IDAT, z_d_ptr, z_d_size, fc) != 0) {
				throw error_png();
			}
		} else {
			if (png_write_chunk(f, PNG_CN_IDAT, z_r_ptr, z_r_size, fc) != 0) {
				throw error_png();
			}
		}
	}

	if (png_write_chunk(f, PNG_CN_IEND, 0, 0, fc) != 0) {
		throw error_png();
	}

	write_store(img_ptr, img_scanline, pal_ptr, pal_size);
}

void write_base_image(adv_fz* f, unsigned* fc, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size) {
	data_ptr pal_r_ptr;
	unsigned pal_r_size;
	data_ptr z_r_ptr;
	unsigned z_r_size;

	pal_r_ptr = data_dup(pal_ptr, pal_size);
        pal_r_size = pal_size;

	png_compress(opt_level, z_r_ptr, z_r_size, img_ptr, img_scanline, WRITE.pixel, 0, 0, WRITE.width, WRITE.height);

	unsigned char ihdr[13];
	unsigned ihdr_size;

	be_uint32_write(ihdr + 0, WRITE.width);
	be_uint32_write(ihdr + 4, WRITE.height);
	ihdr[8] = 8; /* bit depth */
	if (WRITE.pixel == 1)
		ihdr[9] = 3; /* color type */
	else
		ihdr[9] = 2; /* color type */
	ihdr[10] = 0; /* compression */
	ihdr[11] = 0; /* filter */
	ihdr[12] = 0; /* interlace */
	ihdr_size = 13;

	if (png_write_chunk(f, PNG_CN_IHDR, ihdr, ihdr_size, fc) != 0) {
		throw error_png();
	}

	if (pal_r_size) {
		if (png_write_chunk(f, PNG_CN_PLTE, pal_r_ptr, pal_r_size, fc) != 0) {
			throw error_png();
		}
	}

	if (png_write_chunk(f, PNG_CN_IDAT, z_r_ptr, z_r_size, fc) != 0) {
		throw error_png();
	}

	if (png_write_chunk(f, PNG_CN_IEND, 0, 0, fc) != 0) {
		throw error_png();
	}

	write_store(img_ptr, img_scanline, pal_ptr, pal_size);
}

void write_image_raw(adv_fz* f, unsigned* fc, unsigned width, unsigned height, unsigned pixel, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size, int shift_x, int shift_y) {
	if (width != WRITE.width) {
		throw error() << "Internal error";
	}

	if (height != WRITE.height) {
		throw error() << "Internal error";
	}

	if (WRITE.first) {
		if (shift_x!=0 || shift_y!=0) {
			throw error() << "Internal error";
		}

		WRITE.first = 0;
		WRITE.pixel = pixel;
		WRITE.line = pixel * (WRITE.width + WRITE.scroll_width);
		WRITE.scroll_ptr = data_alloc( (WRITE.height + WRITE.scroll_height) * WRITE.line);
		WRITE.current_ptr = WRITE.scroll_ptr + WRITE.scroll_x * WRITE.pixel + WRITE.scroll_y * WRITE.line;
		WRITE.current_x = WRITE.scroll_x;
		WRITE.current_y = WRITE.scroll_y;

		memset(WRITE.scroll_ptr, 0, (WRITE.height + WRITE.scroll_height) * WRITE.line);
		memset(WRITE.pal_ptr,0, 256*3);

		write_store(img_ptr, img_scanline, pal_ptr, pal_size);
		write_first(f, fc);
	} else {
		WRITE.current_ptr += shift_x * WRITE.pixel + shift_y * WRITE.line;
		WRITE.current_x += shift_x;
		WRITE.current_y += shift_y;

		if (!opt_lc && !opt_vlc) {
			write_move(f, fc, shift_x, shift_y);
			write_delta_image(f, fc, img_ptr, img_scanline, pal_ptr, pal_size);
		} else {
			write_base_image(f, fc, img_ptr, img_scanline, pal_ptr, pal_size);
		}
	}
}

void write_image(adv_fz* f, unsigned* fc, unsigned width, unsigned height, unsigned pixel, unsigned char* img_ptr, unsigned img_scanline, unsigned char* pal_ptr, unsigned pal_size, int shift_x, int shift_y) {
	if (pixel == 1) {
		data_ptr new_ptr;
		unsigned new_scanline;

		if (!opt_expand) {
			write_image_raw(f, fc, width, height, pixel, img_ptr, img_scanline, pal_ptr, pal_size, shift_x, shift_y);
		} else {
			write_expand(new_ptr, new_scanline, img_ptr, img_scanline, pal_ptr);
			write_image_raw(f, fc, width, height, 3, new_ptr, new_scanline, 0, 0, shift_x, shift_y);
		}
	} else if (pixel == 3) {
		unsigned char ovr_ptr[256*3];
		unsigned ovr_used[256];
		data_ptr new_ptr;
		unsigned new_scanline;

		if (!opt_reduce) {
			write_image_raw(f, fc, width, height, pixel, img_ptr, img_scanline, 0, 0, shift_x, shift_y);
		} else {
			if (WRITE.first || WRITE.pixel == 3) {
				memset(WRITE.pal_used, 0, sizeof(WRITE.pal_used));
				memset(WRITE.pal_ptr, 0, sizeof(WRITE.pal_ptr));
			}

			if (reduce_image(new_ptr, new_scanline, ovr_ptr, ovr_used, img_ptr, img_scanline) != 0) {
				/* reduction failed */
				if (WRITE.first) {
					write_image_raw(f, fc, width, height, pixel, img_ptr, img_scanline, 0, 0, shift_x, shift_y);
				} else {
					throw error_ignore() << "Color reduction failed";
				}
			} else {
				write_image_raw(f, fc, width, height, 1, new_ptr, new_scanline, ovr_ptr, 256*3, shift_x, shift_y);

				memcpy(WRITE.pal_used, ovr_used, sizeof(WRITE.pal_used));
			}
		}
	}
}

void write_footer(adv_fz* f, unsigned* fc) {
	free(WRITE.scroll_ptr);

	if (png_write_chunk(f, MNG_CN_MEND, 0, 0, fc) != 0) {
		throw error_png();
	}
}

/*************************************************************************************/
/* Conversion */

void convert_frame(adv_fz* f, unsigned* fc, unsigned tick) {
	unsigned char fram[10];
	unsigned fram_size;

	if (tick == WRITE.tick) {
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

	WRITE.tick = tick;

	if (png_write_chunk(f, MNG_CN_FRAM, fram, fram_size, fc) != 0) {
		throw error_png();
	}
}

void convert_header(adv_fz* f, unsigned* fc, unsigned frame_width, unsigned frame_height, unsigned frame_frequency, struct scroll* sc) {
	if (sc) {
		write_header(f, fc, frame_width, frame_height, frame_frequency, sc->x, sc->y, sc->width, sc->height);
	} else {
		write_header(f, fc, frame_width, frame_height, frame_frequency, 0, 0, 0, 0);
	}
}

void convert_image(adv_fz* f_out, unsigned* fc, adv_fz* f_in, unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline, unsigned char* pal_ptr, unsigned pal_size, struct scroll_coord* scc) {
	if (scc) {
		write_image(f_out, fc, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, scc->x, scc->y);
	} else {
		write_image(f_out, fc, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, 0, 0);
	}
}

void convert_f(adv_fz* f_in, adv_fz* f_out, unsigned* filec, unsigned* framec, struct scroll* sc) {
	unsigned counter;
	adv_mng* mng;
	bool first = true;

	mng = mng_init(f_in);

	*filec = 0;
	counter = 0;

	while (1) {
		unsigned pix_width;
		unsigned pix_height;
		unsigned char* pix_ptr;
		unsigned pix_pixel;
		unsigned pix_scanline;
		unsigned char* dat_ptr;
		unsigned dat_size;
		unsigned char* pal_ptr;
		unsigned pal_size;
		unsigned tick;
		int r;

		r = mng_read(mng, &pix_width, &pix_height, &pix_pixel, &dat_ptr, &dat_size, &pix_ptr, &pix_scanline, &pal_ptr, &pal_size, &tick, f_in);
		if (r < 0) {
			throw error_png();
		}
		if (r > 0)
			break;

		try {
			if (first) {
				unsigned frequency = mng_frequency_get(mng);
				if (opt_vlc && tick!=1) {
					// adjust the frequency
					frequency = (frequency + tick / 2) / tick;
					if (frequency == 0)
						frequency = 1;
				}
				convert_header(f_out, filec, mng_width_get(mng), mng_height_get(mng), frequency, sc);
				first = false;
			}

			if (!opt_vlc)
				convert_frame(f_out, filec, tick);

			if (sc) {
				if (counter >= sc->mac) {
					throw error() << "Internal error";
				}
				convert_image(f_out, filec, f_in, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, &sc->map[counter]);
			} else {
				convert_image(f_out, filec, f_in, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, 0);
			}
		} catch (...) {
			free(dat_ptr);
			free(pal_ptr);
			throw;
		}

		free(dat_ptr);
		free(pal_ptr);

		++counter;
		if (!opt_quiet) {
			cout << "Compressing frame " << counter << ", size " << *filec / counter << " [" << *filec << "]    \r";
			cout.flush();
		}
	}

	write_footer(f_out, filec);

	mng_done(mng);

	if (!opt_quiet) {
		// clear
		cout << "                                                    \r";
	}

	*framec = counter;
}

void convert_file(const string& path_src, const string& path_dst, struct scroll* sc) {
	adv_fz* f_in;
	adv_fz* f_out;
	
	unsigned filec;
	unsigned framec;

	f_in = fzopen(path_src.c_str(),"rb");
	if (!f_in) {
		throw error() << "Failed open for reading " << path_src;
	}

	f_out = fzopen(path_dst.c_str(),"wb");
	if (!f_out) {
		fzclose(f_in);
		throw error() << "Failed open for writing " << path_dst;
	}

	try {
		convert_f(f_in, f_out, &filec, &framec, sc);
	} catch (...) {
		fzclose(f_in);
		fzclose(f_out);
		remove(path_dst.c_str());
		throw;
	}

	fzclose(f_in);
	fzclose(f_out);
}

void convert_file(const string& path_src, const string& path_dst) {
	struct scroll sc;

	if (opt_scroll && opt_vlc) {
		throw error() << "The --scroll and --vlc options are incompatible";
	}

	if (opt_scroll && opt_lc) {
		throw error() << "The --scroll and --lc options are incompatible";
	}

	if (opt_scroll) {
		analyze_file(path_src, &sc);
		convert_file(path_src, path_dst, &sc);
	} else {
		convert_file(path_src, path_dst, 0);
	}
}

void convert_inplace(const string& path) {
	// temp name of the saved file
	string path_dst = file_basepath(path) + ".tmp";

	try {
		convert_file(path, path_dst);
	} catch (...) {
		remove(path_dst.c_str());
		throw;
	}

	unsigned dst_size = file_size(path_dst);
	if (!opt_force && file_size(path) < dst_size) {
		// delete the new file
		remove(path_dst.c_str());
		throw error_ignore() << "Bigger " << dst_size;
	} else {
		// prevent external signal
		sig_auto_lock sal;

		// delete the old file
		if (remove(path.c_str()) != 0) {
			remove(path_dst.c_str());
			throw error() << "Failed delete of " << path;
		}

		// rename the new version with the correct name
		if (::rename(path_dst.c_str(), path.c_str()) != 0) {
			throw error() << "Failed rename of " << path_dst << " to " << path;
		}
	}
}

// --------------------------------------------------------------------------
// List

void mng_print(const string& path) {
	unsigned type;
	unsigned size;
	adv_fz* f_in;

	f_in = fzopen(path.c_str(),"rb");
	if (!f_in) {
		throw error() << "Failed open for reading " << path;
	}

	try {
		if (mng_read_signature(f_in) != 0) {
			throw error_png();
		}

		do {
			unsigned char* data;

			if (png_read_chunk(f_in, &data, &size, &type) != 0) {
				throw error_png();
			}

			png_print_chunk(type, data, size);

			free(data);

		} while (type != MNG_CN_MEND);

	} catch (...) {
		fzclose(f_in);
		throw;
	}

	fzclose(f_in);
}

// --------------------------------------------------------------------------
// Command interface

void rezip_single(const string& file, unsigned long long& total_0, unsigned long long& total_1) {
	unsigned size_0;
	unsigned size_1;
	string desc;

	if (!file_exists(file)) {
		throw error() << "File " << file << " doesn't exist";
	}

	try {
		size_0 = file_size(file);

		try {
			convert_inplace(file);
		} catch (error& e) {
			if (!e.ignore_get())
				throw;
			desc = e.desc_get();
		}

		size_1 = file_size(file);

	} catch (error& e) {
		throw e << " on " << file;
	}

	if (!opt_quiet) {
		cout << setw(12) << size_0 << setw(12) << size_1 << " ";
		if (size_0) {
			unsigned perc = size_1 * 100LL / size_0;
			cout << setw(3) << perc;
		} else {
			cout << "  0";
		}
		cout << "% " << file;
		if (desc.length())
			cout << " (" << desc << ")";
		cout << endl;
	}

	total_0 += size_0;
	total_1 += size_1;
}

void rezip_all(int argc, char* argv[]) {
	unsigned long long total_0 = 0;
	unsigned long long total_1 = 0;

	for(int i=0;i<argc;++i)
		rezip_single(argv[i], total_0, total_1);

	if (!opt_quiet) {
		cout << setw(12) << total_0 << setw(12) << total_1 << " ";
		if (total_0) {
			unsigned perc = total_1 * 100LL / total_0;
			cout << setw(3) << perc;
		} else {
			cout << "  0";
		}
		cout << "%" << endl;
	}
}

void list_all(int argc, char* argv[]) {
	for(int i=0;i<argc;++i) {
		if (argc > 1)
			cout << "File: " << argv[i] << endl;
		mng_print(argv[i]);
	}
}

#ifdef HAVE_GETOPT_LONG
struct option long_options[] = {
	{"recompress", 0, 0, 'z'},
	{"list", 0, 0, 'l'},

	{"shrink-0", 0, 0, '0'},
	{"shrink-1", 0, 0, '1'},
	{"shrink-2", 0, 0, '2'},
	{"shrink-3", 0, 0, '3'},

	{"scroll", 1, 0, 's'},
	{"reduce", 0, 0, 'r'},
	{"expand", 0, 0, 'e'},
	{"lc", 0, 0, 'c'},
	{"vlc", 0, 0, 'C'},
	{"force", 0, 0, 'f'},

	{"quiet", 0, 0, 'q'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'V'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "zl0123s:recCfqhV"

void version() {
	cout << PACKAGE " v" VERSION " by Andrea Mazzoleni" << endl;
}

void usage() {
	version();

	cout << "Usage: advmng [options] [FILES...]" << endl;
	cout << endl;
	cout << "Modes:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-l, --list          ", "-l    ") "  List the content of the files" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-z, --recompress    ", "-z    ") "  Recompress the specified files" << endl;
	cout << "Options:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-0, --shrink-store  ", "-0    ") "  Don't compress" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-1, --shrink-fast   ", "-1    ") "  Compress fast" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-2, --shrink-normal ", "-2    ") "  Compress normal" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-3, --shrink-extra  ", "-3    ") "  Compress extra" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-4, --shrink-insane ", "-4    ") "  Compress extreme" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-s, --scroll NxM    ", "-s NxM") "  Enable the scroll optimization with a pattern" << endl;
	cout << "  " SWITCH_GETOPT_LONG("                    ", "      ") "  search. from -Nx-M to NxM. Example: -s 4x6" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-r, --reduce        ", "-r    ") "  Convert the output at palettized 8 bit" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-e, --expand        ", "-e    ") "  Convert the output at 24 bit" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-c, --lc            ", "-c    ") "  Use of the MNG LC (Low Complexity) format" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-C, --vlc           ", "-C    ") "  Use of the MNG VLC (Very Low Complexity) format" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-f, --force         ", "-f    ") "  Force the new file also if it's bigger" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-q, --quiet         ", "-q    ") "  Don't print on the console" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-h, --help          ", "-h    ") "  Help of the program" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-v, --version       ", "-v    ") "  Version of the program" << endl;
}

void process(int argc, char* argv[]) {
	enum cmd_t {
		cmd_unset, cmd_recompress, cmd_list
	} cmd = cmd_unset;

	opt_quiet = false;
	opt_level = shrink_normal;
	opt_reduce = false;
	opt_expand = false;
	opt_dx = 0;
	opt_dy = 0;
	opt_scroll = false;
	opt_lc = false;
	opt_vlc = false;
	opt_force = false;

	if (argc <= 1) {
		usage();
		return;
	}

	int c = 0;

	opterr = 0; // don't print errors

	while ((c =
#ifdef HAVE_GETOPT_LONG
		getopt_long(argc, argv, OPTIONS, long_options, 0))
#else
		getopt(argc, argv, OPTIONS))
#endif
	!= EOF) {
		switch (c) {
			case 'z' :
				if (cmd != cmd_unset)
					throw error() << "Too many commands";
				cmd = cmd_recompress;
				break;
			case 'l' :
				if (cmd != cmd_unset)
					throw error() << "Too many commands";
				cmd = cmd_list;
				break;
			case '0' :
				opt_level = shrink_none;
				opt_force = true;
				break;
			case '1' :
				opt_level = shrink_fast;
				break;
			case '2' :
				opt_level = shrink_normal;
				break;
			case '3' :
				opt_level = shrink_extra;
				break;
			case '4' :
				opt_level = shrink_extreme;
				break;
			case 's' : {
				int n;
				opt_dx = 0;
				opt_dy = 0;
				n = sscanf(optarg,"%dx%d",&opt_dx,&opt_dy);
				if (n != 2)
					throw error() << "Invalid option -s";
				if (opt_dx < 0 || opt_dy < 0)
					throw error() << "Invalid argument for option -s";
					error();
				if (opt_dx == 0 && opt_dy == 0)
					throw error() << "Invalid argument for option -s";
				opt_scroll = true;
				} break;
			case 'r' :
				opt_reduce = true;
				opt_expand = false;
				break;
			case 'e' :
				opt_reduce = false;
				opt_expand = true;
				break;
			case 'c' :
				opt_lc = true;
				opt_force = true;
				break;
			case 'C' :
				opt_vlc = true;
				opt_force = true;
				break;
			case 'f' :
				opt_force = true;
				break;
			case 'q' :
				opt_quiet = true;
				break;
			case 'h' :
				usage();
				return;
			case 'V' :
				version();
				return;
			default: {
				// not optimal code for g++ 2.95.3
				string opt;
				opt = (char)optopt;
				throw error() << "Unknow option `" << opt << "'";
			}
		} 
	}

	switch (cmd) {
	case cmd_recompress :
		rezip_all(argc - optind, argv + optind);
		break;
	case cmd_list :
		list_all(argc - optind, argv + optind);
		break;
	case cmd_unset :
		throw error() << "No command specified";
	}
}

int main(int argc, char* argv[]) {

	try {
		process(argc,argv);
	} catch (error& e) {
		cerr << e << endl;
		exit(EXIT_FAILURE);
	} catch (std::bad_alloc) {
		cerr << "Low memory" << endl;
		exit(EXIT_FAILURE);
	} catch (...) {
		cerr << "Unknow error" << endl;
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

