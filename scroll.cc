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

#include "portable.h"

#include "scroll.h"
#include "data.h"

#if defined(__GNUC__) && defined(__i386__)
#define USE_MMX
#endif
// #define USE_OPTC

static unsigned compare_line(unsigned width, unsigned height, unsigned char* p0, unsigned char* p1, unsigned line)
{
	unsigned i, j;
	unsigned count = 0;

	for(i=0;i<height;++i) {
#if defined(USE_MMX)
		/* MMX ASM optimized version */
		j = width;
		while (j >= 8) {
			unsigned char data[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 };

			unsigned run = j / 8;

			if (run > 255)
				run = 255; /* prevent overflow in the sum byte registers */

			j = j - run * 8;

			__asm__ __volatile__(
				"movq 0(%3), %%mm2\n"
				"movq 8(%3), %%mm3\n"

				"0:\n"

				"movq 0(%0), %%mm0\n"
				"movq 0(%1), %%mm1\n"

				"pcmpeqb %%mm0, %%mm1\n"
				"pand %%mm3, %%mm1\n"
				"paddb %%mm1, %%mm2\n"

				"addl $8, %0\n"
				"addl $8, %1\n"

				"decl %2\n"
				"jnz 0b\n"

				"movq %%mm2, 0(%3)\n"

				: "+r" (p0), "+r" (p1), "+r" (run)
				: "r" (data)
				: "cc", "memory"
			);

			count += data[0];
			count += data[1];
			count += data[2];
			count += data[3];
			count += data[4];
			count += data[5];
			count += data[6];
			count += data[7];
		}
		while (j > 0) {
			if (p0[0] == p1[0])
				++count;
			++p0;
			++p1;
			--j;
		}
#else
#if defined(USE_OPTC)
		/* C optimized version */
		j = width;
		while (j >= 4) {
			unsigned v0 = *(unsigned*)p0;
			unsigned v1 = *(unsigned*)p1;
			v0 ^= v1;
			if (v0) {
				if ((v0 & 0x000000FF) == 0)
					++count;
				if ((v0 & 0x0000FF00) == 0)
					++count;
				if ((v0 & 0x00FF0000) == 0)
					++count;
				if ((v0 & 0xFF000000) == 0)
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
#else
		/* standard C implementation */
		for(j=0;j<width;++j) {
			if (p0[0] == p1[0])
				++count;
			++p0;
			++p1;
		}
#endif
#endif
		p0 += line - width;
		p1 += line - width;
	}

#if defined(USE_MMX)
	__asm__ __volatile__ (
		"emms"
	);
#endif

	return count;
}

static unsigned compare_shift(int x, int y, unsigned width, unsigned height, unsigned char* p0, unsigned char* p1, unsigned pixel, unsigned line)
{
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

	if (pixel == 1) {
		count = compare_line(dx, dy, p0, p1, line);
	} else {
		/* the exact number of equal pixels isn't really required, the */
		/* number of equal channels also works well */
		count = compare_line(dx*3, dy, p0, p1, line) / 3;
	}

	return count;
}

static void compare(adv_scroll* scroll, int* x, int* y, unsigned width, unsigned height, unsigned char* p0, unsigned char* p1, unsigned pixel, unsigned line)
{
	int i, j;
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

	for(i=-scroll->range_dx;i<=scroll->range_dx;++i) {
		for(j=-scroll->range_dy;j<=scroll->range_dy;++j) {
			if ((j || i) && (abs(i)+abs(j)<=scroll->range_limit)) {
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

	/* if the number of matching pixel is too small don't scroll */
	if (best_count < total / 4) {
		*x = 0;
		*y = 0;
	} else {
		*x = best_x;
		*y = best_y;
	}
}

static void insert(adv_scroll_info* info, int x, int y)
{
	if (info->mac == info->max) {
		info->max *= 2;
		if (!info->max)
			info->max = 64;
		info->map = (adv_scroll_coord*)realloc(info->map, info->max * sizeof(adv_scroll_coord));
	}

	info->map[info->mac].x = x;
	info->map[info->mac].y = y;
	++info->mac;
}

void scroll_analyze(adv_scroll* scroll, unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline)
{
	unsigned char* ptr;
	unsigned scanline;
	unsigned i;

	scanline = pix_width * pix_pixel;
	ptr = data_alloc(pix_height * scanline);

	for(i=0;i<pix_height;++i) {
		memcpy(ptr + i*scanline, pix_ptr + i*pix_scanline, scanline);
	}

	data_free(scroll->pre_ptr);
	scroll->pre_ptr = scroll->cur_ptr;
	scroll->cur_ptr = ptr;

	if (scroll->pre_ptr && scroll->cur_ptr) {
		int x;
		int y;
		compare(scroll, &x, &y, pix_width, pix_height, scroll->pre_ptr, scroll->cur_ptr, pix_pixel, scanline);
		insert(scroll->info, x, y);
	} else {
		insert(scroll->info, 0, 0);
	}
}

static void postprocessing(adv_scroll_info* info)
{
	int px = 0;
	int py = 0;
	int min_x = 0;
	int min_y = 0;
	int max_x = 0;
	int max_y = 0;
	unsigned i;

	for(i=0;i<info->mac;++i) {
		px += info->map[i].x;
		py += info->map[i].y;
		if (px < min_x)
			min_x = px;
		if (py < min_y)
			min_y = py;
		if (px > max_x)
			max_x = px;
		if (py > max_y)
			max_y = py;
	}

	info->x = -min_x;
	info->y = -min_y;
	info->width = max_x - min_x;
	info->height = max_y - min_y;
}

adv_scroll* scroll_init(int dx, int dy, int limit)
{
	adv_scroll* scroll = (adv_scroll*)malloc(sizeof(adv_scroll));

	scroll->info = (adv_scroll_info*)malloc(sizeof(adv_scroll_info));

	scroll->info->map = 0;
	scroll->info->mac = 0;
	scroll->info->max = 0;

	scroll->range_dx = dx;
	scroll->range_dy = dy;
	scroll->range_limit = limit;

	scroll->cur_ptr = 0;
	scroll->pre_ptr = 0;

	return scroll;
}

void scroll_last_get(adv_scroll* scroll, int* x, int* y)
{
	if (!scroll->info || !scroll->info->mac) {
		*x = 0;
		*y = 0;
	} else {
		*x = scroll->info->map[scroll->info->mac-1].x;
		*y = scroll->info->map[scroll->info->mac-1].y;
	}
}

adv_scroll_info* scroll_info_init(adv_scroll* scroll)
{
	adv_scroll_info* info = scroll->info;

	postprocessing(info);

	scroll->info = 0;

	return info;
}

void scroll_info_done(adv_scroll_info* info)
{
	free(info->map);
	free(info);
}

void scroll_done(adv_scroll* scroll)
{
	data_free(scroll->pre_ptr);
	data_free(scroll->cur_ptr);
	if (scroll->info)
		scroll_info_done(scroll->info);
	free(scroll);
}

