/*
 * This file is part of the AdvanceSCAN project.
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

#include "portable.h"

#include "lib/endianrw.h"
#include "lib/mng.h"

#include "pngex.h"

#include <iostream>
#include <iomanip>

using namespace std;

void png_compress(shrink_t level, data_ptr& out_ptr, unsigned& out_size, const unsigned char* img_ptr, unsigned img_scanline, unsigned img_pixel, unsigned x, unsigned y, unsigned dx, unsigned dy)
{
	data_ptr fil_ptr;
	unsigned fil_size;
	unsigned fil_scanline;
	data_ptr z_ptr;
	unsigned z_size;
	unsigned i;
	unsigned char* p0;

	fil_scanline = dx * img_pixel + 1;
	fil_size = dy * fil_scanline;
	z_size = oversize_zlib(fil_size);

	fil_ptr = data_alloc(fil_size);
	z_ptr = data_alloc(z_size);

	p0 = fil_ptr;

	for(i=0;i<dy;++i) {
		const unsigned char* p1 = &img_ptr[x * img_pixel + (i+y) * img_scanline];
		*p0++ = 0;
		memcpy(p0, p1, dx * img_pixel);
		p0 += dx * img_pixel;
	}

	assert(p0 == fil_ptr + fil_size);

	if (!compress_zlib(level, z_ptr, z_size, fil_ptr, fil_size)) {
		throw error() << "Failed compression";
	}

	out_ptr = z_ptr;
	out_size = z_size;
}

void png_compress_delta(shrink_t level, data_ptr& out_ptr, unsigned& out_size, const unsigned char* img_ptr, unsigned img_scanline, unsigned img_pixel, const unsigned char* prev_ptr, unsigned prev_scanline, unsigned x, unsigned y, unsigned dx, unsigned dy)
{
	data_ptr fil_ptr;
	unsigned fil_size;
	unsigned fil_scanline;
	data_ptr z_ptr;
	unsigned z_size;
	unsigned i;
	unsigned char* p0;

	fil_scanline = dx * img_pixel + 1;
	fil_size = dy * fil_scanline;
	z_size = oversize_zlib(fil_size);

	fil_ptr = data_alloc(fil_size);
	z_ptr = data_alloc(z_size);

	p0 = fil_ptr;

	for(i=0;i<dy;++i) {
		unsigned j;
		const unsigned char* p1 = &img_ptr[x * img_pixel + (i+y) * img_scanline];
		const unsigned char* p2 = &prev_ptr[x * img_pixel + (i+y) * prev_scanline];

		*p0++ = 0;
		for(j=0;j<dx*img_pixel;++j)
			*p0++ = *p1++ - *p2++;
	}

	assert(p0 == fil_ptr + fil_size);

	if (!compress_zlib(level, z_ptr, z_size, fil_ptr, fil_size)) {
		throw error() << "Failed compression";
	}

	out_ptr = z_ptr;
	out_size = z_size;
}

void png_compress_palette_delta(data_ptr& out_ptr, unsigned& out_size, const unsigned char* pal_ptr, unsigned pal_size, const unsigned char* prev_ptr)
{
	unsigned i;
	unsigned char* dst_ptr;
	unsigned dst_size;

	dst_ptr = data_alloc(pal_size * 2);
	dst_size = 0;

	dst_ptr[dst_size++] = 0; /* replacement */

	i = 0;
	while (i<pal_size) {
		unsigned j;

		while (i < pal_size && prev_ptr[i] == pal_ptr[i] && prev_ptr[i+1] == pal_ptr[i+1] && prev_ptr[i+2] == pal_ptr[i+2])
			i += 3;

		if (i == pal_size)
			break;

		j = i;

		while (j < pal_size && (prev_ptr[j] != pal_ptr[j] || prev_ptr[j+1] != pal_ptr[j+1] || prev_ptr[j+2] != pal_ptr[j+2]))
			j += 3;

		dst_ptr[dst_size++] = i / 3; /* first index */
		dst_ptr[dst_size++] = (j / 3) - 1; /* last index */

		while (i < j) {
			dst_ptr[dst_size++] = pal_ptr[i++];
			dst_ptr[dst_size++] = pal_ptr[i++];
			dst_ptr[dst_size++] = pal_ptr[i++];
		}
	}

	if (dst_size == 1) {
		out_ptr = 0;
		out_size = 0;
		free(dst_ptr);
	} else {
		out_ptr = dst_ptr;
		out_size = dst_size;
	}
}

void png_print_chunk(unsigned type, unsigned char* data, unsigned size)
{
	char tag[5];
	unsigned i;

	be_uint32_write(tag, type);
	tag[4] = 0;

	cout << tag << setw(8) << size;

	switch (type) {
		case ADV_MNG_CN_MHDR :
			cout << " width:" << be_uint32_read(data+0) << " height:" << be_uint32_read(data+4) << " frequency:" << be_uint32_read(data+8);
			cout << " simplicity:" << be_uint32_read(data+24);
			cout << "(bit";
			for(i=0;i<32;++i) {
				if (be_uint32_read(data+24) & (1 << i)) {
					cout << "," << i;
				}
			}
			cout << ")";
		break;
		case ADV_MNG_CN_DHDR :
			cout << " id:" << be_uint16_read(data+0);
			switch (data[2]) {
				case 0 : cout << " img:unspecified"; break;
				case 1 : cout << " img:png"; break;
				case 2 : cout << " img:jng"; break;
				default: cout << " img:?"; break;
			}
			switch (data[3]) {
				case 0 : cout << " delta:entire_replacement"; break;
				case 1 : cout << " delta:block_addition"; break;
				case 2 : cout << " delta:block_alpha_addition"; break;
				case 3 : cout << " delta:block_color_addition"; break;
				case 4 : cout << " delta:block_replacement"; break;
				case 5 : cout << " delta:block_alpha_replacement"; break;
				case 6 : cout << " delta:block_color_replacement"; break;
				case 7 : cout << " delta:no_change"; break;
				default: cout << " delta:?"; break;
			}
			if (size >= 12) {
				cout << " width:" << be_uint32_read(data + 4) << " height:" << be_uint32_read(data + 8);
			}
			if (size >= 20) {
				cout << " x:" << (int)be_uint32_read(data + 12) << " y:" << (int)be_uint32_read(data + 16);
			}
		break;
		case ADV_MNG_CN_FRAM :
			if (size >= 1) {
				cout << " mode:" << (unsigned)data[0];
			}
			if (size > 1) {
				i = 1;
				while (i < size && data[i]!=0)
					++i;
				cout << " len:" << i-1;

				if (size >= i+2) {
					cout << " delay_mode:" << (unsigned)data[i+1];
				}

				if (size >= i+3) {
					cout << " timeout:" << (unsigned)data[i+2];
				}

				if (size >= i+4) {
					cout << " clip:" << (unsigned)data[i+3];
				}

				if (size >= i+5) {
					cout << " syncid:" << (unsigned)data[i+4];
				}

				if (size >= i+9) {
					cout << " tick:" << be_uint32_read(data+i+5);
				}

				if (size >= i+13) {
					cout << " timeout:" << be_uint32_read(data+i+9);
				}

				if (size >= i+14) {
					cout << " dt:" << (unsigned)data[i+10];
				}

				if (size >= i+15) {
					cout << " ...";
				}
			}
			break;
		case ADV_MNG_CN_DEFI :
			cout << " id:" << be_uint16_read(data+0);
			if (size >= 3) {
				switch (data[2]) {
					case 0 : cout << " visible:yes"; break;
					case 1 : cout << " visible:no"; break;
					default : cout << " visible:?"; break;
				}
			}
			if (size >= 4) {
				switch (data[3]) {
					case 0 : cout << " concrete:abstract"; break;
					case 1 : cout << " concrete:concrete"; break;
					default : cout << " concrete:?"; break;
				}
			}
			if (size >= 12) {
				cout << " x:" << (int)be_uint32_read(data + 4) << " y:" << (int)be_uint32_read(data + 8);
			}
			if (size >= 28) {
				cout << " left:" << be_uint32_read(data + 12) << " right:" << be_uint32_read(data + 16) << " top:" << be_uint32_read(data + 20) << " bottom:" << be_uint32_read(data + 24);
			}
		break;
		case ADV_MNG_CN_MOVE :
			cout << " id_from:" << be_uint16_read(data+0) << " id_to:" << be_uint16_read(data+2);
			switch (data[4]) {
				case 0 : cout << " type:replace"; break;
				case 1 : cout << " type:add"; break;
				default : cout << " type:?"; break;
			}
			cout << " x:" << (int)be_uint32_read(data + 5) << " y:" << (int)be_uint32_read(data + 9);
			break;
		case ADV_MNG_CN_PPLT :
			switch (data[0]) {
				case 0 : cout << " type:replacement_rgb"; break;
				case 1 : cout << " type:delta_rgb"; break;
				case 2 : cout << " type:replacement_alpha"; break;
				case 3 : cout << " type:delta_alpha"; break;
				case 4 : cout << " type:replacement_rgba"; break;
				case 5 : cout << " type:delta_rgba"; break;
				default : cout << " type:?"; break;
			}
			i = 1;
			while (i<size) {
				unsigned ssize;
				cout << " " << (unsigned)data[i] << ":" << (unsigned)data[i+1];
				if (data[0] == 0 || data[1] == 1)
					ssize = 3;
				else if (data[0] == 2 || data[1] == 3)
					ssize = 1;
				else
					ssize = 4;
				i += 2 + (data[i+1] - data[i] + 1) * ssize;
			}
			break;
		case ADV_PNG_CN_IHDR :
			cout << " width:" << be_uint32_read(data) << " height:" << be_uint32_read(data + 4);
			cout << " depth:" << (unsigned)data[8];
			cout << " color_type:" << (unsigned)data[9];
			cout << " compression:" << (unsigned)data[10];
			cout << " filter:" << (unsigned)data[11];
			cout << " interlace:" << (unsigned)data[12];
		break;
	}

	cout << endl;
}

void png_write(adv_fz* f, unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline, unsigned char* pal_ptr, unsigned pal_size, unsigned char* rns_ptr, unsigned rns_size, shrink_t level)
{
	unsigned char ihdr[13];
	data_ptr z_ptr;
	unsigned z_size;

	if (adv_png_write_signature(f, 0) != 0) {
		throw_png_error();
	}

	be_uint32_write(ihdr + 0, pix_width);
	be_uint32_write(ihdr + 4, pix_height);
	ihdr[8] = 8; /* bit depth */
	if (pix_pixel == 1)
		ihdr[9] = 3; /* color type */
	else if (pix_pixel == 3)
		ihdr[9] = 2; /* color type */
	else if (pix_pixel == 4)
		ihdr[9] = 6; /* color type */
	else
		throw error() << "Invalid format";

	ihdr[10] = 0; /* compression */
	ihdr[11] = 0; /* filter */
	ihdr[12] = 0; /* interlace */

	if (adv_png_write_chunk(f, ADV_PNG_CN_IHDR, ihdr, sizeof(ihdr), 0) != 0) {
		throw_png_error();
	}

	if (pal_size) {
		if (adv_png_write_chunk(f, ADV_PNG_CN_PLTE, pal_ptr, pal_size, 0) != 0) {
			throw_png_error();
		}
	}

	if (rns_size) {
		if (adv_png_write_chunk(f, ADV_PNG_CN_tRNS, rns_ptr, rns_size, 0) != 0) {
			throw_png_error();
		}
	}

	png_compress(level, z_ptr, z_size, pix_ptr, pix_scanline, pix_pixel, 0, 0, pix_width, pix_height);

	if (adv_png_write_chunk(f, ADV_PNG_CN_IDAT, z_ptr, z_size, 0) != 0) {
		throw_png_error();
	}

	if (adv_png_write_chunk(f, ADV_PNG_CN_IEND, 0, 0, 0) != 0) {
		throw_png_error();
	}
}

void png_convert_4(
	unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline,
	unsigned char* pal_ptr, unsigned pal_size,
	unsigned char** dst_ptr, unsigned* dst_pixel, unsigned* dst_scanline)
{
	*dst_pixel = 4;
	*dst_scanline = 4 * pix_width;
	*dst_ptr = (unsigned char*)malloc(*dst_scanline * pix_height);

	if (pix_pixel == 3) {
		unsigned i, j;
		for(i=0;i<pix_height;++i) {
			const unsigned char* p0 = pix_ptr + i * pix_scanline;
			unsigned char* p1 = *dst_ptr + i * *dst_scanline;
			for(j=0;j<pix_width;++j) {
				p1[0] = p0[0];
				p1[1] = p0[1];
				p1[2] = p0[2];
				p1[3] = 0xFF;
				p0 += 3;
				p1 += 4;
			}
		}
	} else if (pix_pixel == 1) {
		unsigned i, j;
		for(i=0;i<pix_height;++i) {
			const unsigned char* p0 = pix_ptr + i * pix_scanline;
			unsigned char* p1 = *dst_ptr + i * *dst_scanline;
			for(j=0;j<pix_width;++j) {
				unsigned char* c = &pal_ptr[p0[0]*3];
				p1[0] = c[0];
				p1[1] = c[1];
				p1[2] = c[2];
				p1[3] = 0xFF;
				p0 += 1;
				p1 += 4;
			}
		}
	} else if (pix_pixel == 4) {
		unsigned i;
		for(i=0;i<pix_height;++i) {
			const unsigned char* p0 = pix_ptr + i * pix_scanline;
			unsigned char* p1 = *dst_ptr + i * *dst_scanline;
			memcpy(p1, p0, *dst_scanline);
		}
	} else {
		throw error_unsupported() << "Unsupported format";
	}
}

