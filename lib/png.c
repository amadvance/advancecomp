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

#include "png.h"
#include "endianrw.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/**************************************************************************************/
/* PNG */

static unsigned char PNG_Signature[] = "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";

/**
 * Read a PNG data chunk.
 * \param f File to read.
 * \param data Where to put the allocated data of the chunk.
 * \param size Where to put the size of the chunk.
 * \param type Where to put the type of the chunk. 
 */
adv_error png_read_chunk(adv_fz* f, unsigned char** data, unsigned* size, unsigned* type)
{
	unsigned char cl[4];
	unsigned char ct[4];
	unsigned char cc[4];

	if (fzread(cl, 4, 1, f) != 1) {
		error_set("Error reading the chunk size");
		goto err;
	}

	*size = be_uint32_read(cl);

	if (fzread(ct, 4, 1, f) != 1) {
		error_set("Error reading the chunk type");
		goto err;
	}

	*type = be_uint32_read(ct);

	if (*size) {
		*data = malloc(*size);
		if (!*data) {
			error_set("Low memory");
			goto err;
		}

		if (fzread(*data, *size, 1, f) != 1) {
			error_set("Error reading the chunk data");
			goto err_data;
		}
	} else {
		*data = 0;
	}

	if (fzread(cc, 4, 1, f) != 1) {
		error_set("Error reading the chunk crc");
		goto err_data;
	}

	return 0;
err_data:
	free(*data);
err:
	return -1;
}

/**
 * Write a PNG data chunk.
 * \param f File to write.
 * \param type Type of the chunk.
 * \param data Data of the chunk.
 * \param size Size of the chunk.
 * \param count Pointer at the number of bytes written. It may be 0.
 */
adv_error png_write_chunk(adv_fz* f, unsigned type, const unsigned char* data, unsigned size, unsigned* count)
{
	unsigned char v[4];
	unsigned crc;

	be_uint32_write(v, size);
	if (fzwrite(v, 4, 1, f) != 1) {
		error_set("Error writing the chunk size");
		return -1;
	}

	be_uint32_write(v, type);
	if (fzwrite(v, 4, 1, f) != 1) {
		error_set("Error writing the chunk type");
		return -1;
	}

	crc = crc32(0, v, 4);
	if (size > 0) {
		if (fzwrite(data, size, 1, f) != 1) {
			error_set("Error writing the chunk data");
			return -1;
		}

		crc = crc32(crc, data, size);
	}

	be_uint32_write(v, crc);
	if (fzwrite(v, 4, 1, f) != 1) {
		error_set("Error writing the chunk crc");
		return -1;
	}

	if (count)
		*count += 4 + 4 + size + 4;

	return 0;
}

/**
 * Read the PNG file signature.
 * \param f File to read.
 */
adv_error png_read_signature(adv_fz* f)
{
	unsigned char signature[8];

	if (fzread(signature, 8, 1, f) != 1) {
		error_set("Error reading the signature");
		return -1;
	}

	if (memcmp(signature, PNG_Signature, 8)!=0) {
		error_set("Invalid PNG signature");
		return -1;
	}

	return 0;
}

/**
 * Write the PNG file signature.
 * \param f File to write.
 * \param count Pointer at the number of bytes written. It may be 0.
 */
adv_error png_write_signature(adv_fz* f, unsigned* count)
{
	if (fzwrite(PNG_Signature, 8, 1, f) != 1) {
		error_set("Error writing the signature");
		return -1;
	}

	if (count)
		*count += 8;

	return 0;
}

/**
 * Expand a 4 bits per pixel image to a 8 bits per pixel image.
 * \param width Width of the image.
 * \param height Height of the image.
 * \param ptr Data pointer. It must point at the first filter type byte.
 */
void png_expand_4(unsigned width, unsigned height, unsigned char* ptr)
{
	unsigned i, j;
	unsigned char* p8 = ptr + height * (width + 1) - 1;
	unsigned char* p4 = ptr + height * (width / 2 + 1) - 1;

	width /= 2;
	for(i=0;i<height;++i) {
		for(j=0;j<width;++j) {
			unsigned char v = *p4;
			*p8-- = v & 0xF;
			*p8-- = v >> 4;
			--p4;
		}
		--p8;
		--p4;
	}
}

/**
 * Expand a 2 bits per pixel image to a 8 bits per pixel image.
 * \param width Width of the image.
 * \param height Height of the image.
 * \param ptr Data pointer. It must point at the first filter type byte.
 */
void png_expand_2(unsigned width, unsigned height, unsigned char* ptr)
{
	unsigned i, j;
	unsigned char* p8 = ptr + height * (width + 1) - 1;
	unsigned char* p2 = ptr + height * (width / 4 + 1) - 1;

	width /= 4;
	for(i=0;i<height;++i) {
		for(j=0;j<width;++j) {
			unsigned char v = *p2;
			*p8-- = v & 0x3;
			*p8-- = (v >> 2) & 0x3;
			*p8-- = (v >> 4) & 0x3;
			*p8-- = v >> 6;
			--p2;
		}
		--p8;
		--p2;
	}
}

/**
 * Expand a 1 bit per pixel image to a 8 bit per pixel image.
 * \param width Width of the image.
 * \param height Height of the image.
 * \param ptr Data pointer. It must point at the first filter type byte.
 */
void png_expand_1(unsigned width, unsigned height, unsigned char* ptr)
{
	unsigned i, j;
	unsigned char* p8 = ptr + height * (width + 1) - 1;
	unsigned char* p1 = ptr + height * (width / 8 + 1) - 1;

	width /= 8;
	for(i=0;i<height;++i) {
		for(j=0;j<width;++j) {
			unsigned char v = *p1;
			*p8-- = v & 0x1;
			*p8-- = (v >> 1) & 0x1;
			*p8-- = (v >> 2) & 0x1;
			*p8-- = (v >> 3) & 0x1;
			*p8-- = (v >> 4) & 0x1;
			*p8-- = (v >> 5) & 0x1;
			*p8-- = (v >> 6) & 0x1;
			*p8-- = v >> 7;
			--p1;
		}
		--p8;
		--p1;
	}
}

/**
 * Unfilter a 8 bit image.
 * \param width With of the image.
 * \param height Height of the image.
 * \param p Data pointer. It must point at the first filter type byte.
 * \param line Scanline size of row.
 */
void png_unfilter_8(unsigned width, unsigned height, unsigned char* p, unsigned line)
{
	unsigned i, j;

	for(i=0;i<height;++i) {
		unsigned char f = *p++;

		if (f == 0) { /* none */
			p += width;
		} else if (f == 1) { /* sub */
			++p;
			for(j=1;j<width;++j) {
				p[0] += p[-1];
				++p;
			}
		} else if (f == 2) { /* up */
			if (i) {
				unsigned char* u = p - line;
				for(j=0;j<width;++j) {
					*p += *u;
					++p;
					++u;
				}
			} else {
				p += width;
			}
		} else if (f == 3) { /* average */
			if (i) {
				unsigned char* u = p - line;
				p[0] += u[0] / 2;
				++p;
				++u;
				for(j=1;j<width;++j) {
					unsigned a = (unsigned)u[0] + (unsigned)p[-1];
					p[0] += a >> 1;
					++p;
					++u;
				}
			} else {
				++p;
				for(j=1;j<width;++j) {
					p[0] += p[-1] / 2;
					++p;
				}
			}
		} else if (f == 4) { /* paeth */
			unsigned char* u = p - line;
			for(j=0;j<width;++j) {
				unsigned a, b, c;
				int v;
				int da, db, dc;
				a = j<1 ? 0 : p[-1];
				b = i<1 ? 0 : u[0];
				c = (j<1 || i<1) ? 0 : u[-1];
				v = a + b - c;
				da = v - a;
				if (da < 0)
					da = -da;
				db = v - b;
				if (db < 0)
					db = -db;
				dc = v - c;
				if (dc < 0)
					dc = -dc;
				if (da <= db && da <= dc)
					p[0] += a;
				else if (db <= dc)
					p[0] += b;
				else
					p[0] += c;
				++p;
				++u;
			}
		}

		p += line - width - 1;
	}
}

/**
 * Unfilter a 24 bit image.
 * \param width With of the image.
 * \param height Height of the image.
 * \param p Data pointer. It must point at the first filter type byte.
 * \param line Scanline size of row.
 */
void png_unfilter_24(unsigned width, unsigned height, unsigned char* p, unsigned line)
{
	unsigned i, j;

	for(i=0;i<height;++i) {
		unsigned char f = *p++;

		if (f == 0) { /* none */
			p += width;
		} else if (f == 1) { /* sub */
			p += 3;
			for(j=3;j<width;++j) {
				p[0] += p[-3];
				++p;
			}
		} else if (f == 2) { /* up */
			if (i) {
				unsigned char* u = p - line;
				for(j=0;j<width;++j) {
					*p += *u;
					++p;
					++u;
				}
			} else {
				p += width;
			}
		} else if (f == 3) { /* average */
			if (i) {
				unsigned char* u = p - line;
				p[0] += u[0] / 2;
				p[1] += u[1] / 2;
				p[2] += u[2] / 2;
				p += 3;
				u += 3;
				for(j=3;j<width;++j) {
					unsigned a = (unsigned)u[0] + (unsigned)p[-3];
					p[0] += a >> 1;
					++p;
					++u;
				}
			} else {
				p += 3;
				for(j=3;j<width;++j) {
					p[0] += p[-3] / 2;
					++p;
				}
			}
		} else if (f == 4) { /* paeth */
			unsigned char* u = p - line;
			for(j=0;j<width;++j) {
				unsigned a, b, c;
				int v;
				int da, db, dc;
				a = j<3 ? 0 : p[-3];
				b = i<1 ? 0 : u[0];
				c = (j<3 || i<1) ? 0 : u[-3];
				v = a + b - c;
				da = v - a;
				if (da < 0)
					da = -da;
				db = v - b;
				if (db < 0)
					db = -db;
				dc = v - c;
				if (dc < 0)
					dc = -dc;
				if (da <= db && da <= dc)
					p[0] += a;
				else if (db <= dc)
					p[0] += b;
				else
					p[0] += c;
				++p;
				++u;
			}
		}

		p += line - width - 1;
	}
}

/**
 * Unfilter a 32 bit image.
 * \param width With of the image.
 * \param height Height of the image.
 * \param p Data pointer. It must point at the first filter type byte.
 * \param line Scanline size of row.
 */
void png_unfilter_32(unsigned width, unsigned height, unsigned char* p, unsigned line)
{
	unsigned i, j;

	for(i=0;i<height;++i) {
		unsigned char f = *p++;

		if (f == 0) { /* none */
			p += width;
		} else if (f == 1) { /* sub */
			p += 4;
			for(j=4;j<width;++j) {
				p[0] += p[-4];
				++p;
			}
		} else if (f == 2) { /* up */
			if (i) {
				unsigned char* u = p - line;
				for(j=0;j<width;++j) {
					*p += *u;
					++p;
					++u;
				}
			} else {
				p += width;
			}
		} else if (f == 3) { /* average */
			if (i) {
				unsigned char* u = p - line;
				p[0] += u[0] / 2;
				p[1] += u[1] / 2;
				p[2] += u[2] / 2;
				p[3] += u[3] / 2;
				p += 4;
				u += 4;
				for(j=4;j<width;++j) {
					unsigned a = (unsigned)u[0] + (unsigned)p[-4];
					p[0] += a >> 1;
					++p;
					++u;
				}
			} else {
				p += 4;
				for(j=4;j<width;++j) {
					p[0] += p[-4] / 2;
					++p;
				}
			}
		} else if (f == 4) { /* paeth */
			unsigned char* u = p - line;
			for(j=0;j<width;++j) {
				unsigned a, b, c;
				int v;
				int da, db, dc;
				a = j<4 ? 0 : p[-4];
				b = i<1 ? 0 : u[0];
				c = (j<4 || i<1) ? 0 : u[-4];
				v = a + b - c;
				da = v - a;
				if (da < 0)
					da = -da;
				db = v - b;
				if (db < 0)
					db = -db;
				dc = v - c;
				if (dc < 0)
					dc = -dc;
				if (da <= db && da <= dc)
					p[0] += a;
				else if (db <= dc)
					p[0] += b;
				else
					p[0] += c;
				++p;
				++u;
			}
		}

		p += line - width - 1;
	}
}

/**
 * Read until the PNG_CN_IEND is found.
 * \param f File to read.
 * \param data Pointer at the first chunk to analyze. This chunk is not deallocated.
 * \param data_size Size of the data chunk.
 * \param type Type of the data chunk.
 */
adv_error png_read_iend(adv_fz* f, const unsigned char* data, unsigned data_size, unsigned type)
{
	if (type == PNG_CN_IEND)
		return 0;

	/* ancillary bit. bit 5 of first byte. 0 (uppercase) = critical, 1 (lowercase) = ancillary. */
	if ((type & 0x20000000) == 0) {
		char buf[4];
		be_uint32_write(buf, type);
		error_unsupported_set("Unsupported critical chunk '%c%c%c%c'", buf[0], buf[1], buf[2], buf[3]);
		return -1;
	}

	while (1) {
		unsigned char* ptr;
		unsigned ptr_size;

		/* read next */
		if (png_read_chunk(f, &ptr, &ptr_size, &type) != 0) {
			return -1;
		}

		free(ptr);

		if (type == PNG_CN_IEND)
			return 0;

		/* ancillary bit. bit 5 of first byte. 0 (uppercase) = critical, 1 (lowercase) = ancillary. */
		if ((type & 0x20000000) == 0) {
			char buf[4];
			be_uint32_write(buf, type);
			error_unsupported_set("Unsupported critical chunk '%c%c%c%c'", buf[0], buf[1], buf[2], buf[3]);
			return -1;
		}
	}

	return 0;
}

/**
 * Read from the PNG_CN_IHDR chunk to the PNG_CN_IEND chunk.
 * \param pix_width Where to put the image width.
 * \param pix_height Where to put the image height.
 * \param pix_pixel Where to put the image bytes per pixel.
 * \param dat_ptr Where to put the allocated data pointer.
 * \param dat_size Where to put the allocated data size.
 * \param pix_ptr Where to put pointer at the start of the image data.
 * \param pix_scanline Where to put the length of a scanline in bytes.
 * \param pal_ptr Where to put the allocated palette data pointer. Set to 0 if the image is RGB.
 * \param pal_size Where to put the palette size in number of colors. Set to 0 if the image is RGB.
 * \param rns_ptr Where to put the allocated transparency data pointer. Set to 0 if the image hasn't transparency.
 * \param rns_size Where to put the transparency size in number of bytes. Set to 0 if the image hasn't transparency.
 * \param f File to read.
 * \param data Pointer at the IHDR chunk. This chunk is not deallocated.
 * \param data_size Size of the IHDR chuck.
 */
adv_error png_read_ihdr(
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned char** rns_ptr, unsigned* rns_size,
	adv_fz* f, const unsigned char* data, unsigned data_size
)
{
	unsigned char* ptr;
	unsigned ptr_size;
	unsigned type;
	unsigned long res_size;
	unsigned pixel;
	unsigned width;
	unsigned width_align;
	unsigned height;
	unsigned depth;
	int r;
	z_stream z;

	*dat_ptr = 0;
	*pix_ptr = 0;
	*pal_ptr = 0;
	*rns_ptr = 0;

	if (data_size != 13) {
		error_set("Invalid IHDR size %d instead of 13", data_size);
		goto err;
	}

	*pix_width = width = be_uint32_read(data + 0);
	*pix_height = height = be_uint32_read(data + 4);

	depth = data[8];
	if (data[9] == 3 && depth == 8) {
		pixel = 1;
		width_align = width;
	} else if (data[9] == 3 && depth == 4) {
		pixel = 1;
		width_align = (width + 1) & ~1;
	} else if (data[9] == 3 && depth == 2) {
		pixel = 1;
		width_align = (width + 3) & ~3;
	} else if (data[9] == 3 && depth == 1) {
		pixel = 1;
		width_align = (width + 7) & ~7;
	} else if (data[9] == 2 && depth == 8) {
		pixel = 3;
		width_align = width;
	} else {
		error_unsupported_set("Unsupported bit depth/color type, %d/%d", (unsigned)data[8], (unsigned)data[9]);
		goto err;
	}
	*pix_pixel = pixel;

	if (data[10] != 0) { /* compression */
		error_unsupported_set("Unsupported compression, %d instead of 0", (unsigned)data[10]);
		goto err;
	}
	if (data[11] != 0) { /* filter */
		error_unsupported_set("Unsupported filter, %d instead of 0", (unsigned)data[11]);
		goto err;
	}
	if (data[12] != 0) { /* interlace */
		error_unsupported_set("Unsupported interlace %d", (unsigned)data[12]);
		goto err;
	}

	if (png_read_chunk(f, &ptr, &ptr_size, &type) != 0)
		goto err;

	while (type != PNG_CN_PLTE && type != PNG_CN_IDAT) {
		free(ptr);

		if (png_read_chunk(f, &ptr, &ptr_size, &type) != 0)
			goto err;
	}

	if (type == PNG_CN_PLTE) {
		if (pixel != 1) {
			error_set("Unexpected PLTE chunk");
			goto err_ptr;
		}

		if (ptr_size > 256*3) {
			error_set("Invalid palette size in PLTE chunk");
			goto err_ptr;
		}

		*pal_ptr = ptr;
		*pal_size = ptr_size;

		if (png_read_chunk(f, &ptr, &ptr_size, &type) != 0)
			goto err;
	} else {
		if (pixel != 3) {
			error_set("Missing PLTE chunk");
			goto err_ptr;
		}

		*pal_ptr = 0;
		*pal_size = 0;
	}

	*rns_size = 0;
	while (type != PNG_CN_IDAT) {
		if (type == PNG_CN_tRNS) {
			*rns_ptr = ptr;
			*rns_size = ptr_size;
		} else {
			free(ptr);
		}

		if (png_read_chunk(f, &ptr, &ptr_size, &type) != 0)
			goto err;
	}

	*dat_size = height * (width_align * pixel + 1);
	*dat_ptr = malloc(*dat_size);
	*pix_scanline = width_align * pixel + 1;
	*pix_ptr = *dat_ptr + 1;

	z.zalloc = 0;
	z.zfree = 0;
	z.next_out = *dat_ptr;
	z.avail_out = *dat_size;
	z.next_in = 0;
	z.avail_in = 0;

	r = inflateInit(&z);

	while (r == Z_OK && type == PNG_CN_IDAT) {
		z.next_in = ptr;
		z.avail_in = ptr_size;

		r = inflate(&z, Z_NO_FLUSH);

		free(ptr);

		if (png_read_chunk(f, &ptr, &ptr_size, &type) != 0) {
			inflateEnd(&z);
			goto err;
		}
	}

	res_size = z.total_out;

	inflateEnd(&z);

	if (r != Z_STREAM_END) {
		error_set("Invalid compressed data");
		goto err_ptr;
	}

	if (depth == 8) {
		if (res_size != *dat_size) {
			error_set("Invalid decompressed size");
			goto err_ptr;
		}

		if (pixel == 1)
			png_unfilter_8(width * pixel, height, *dat_ptr, width_align * pixel + 1);
		else
			png_unfilter_24(width * pixel, height, *dat_ptr, width_align * pixel + 1);

	} else if (depth == 4) {
		if (res_size != height * (width_align / 2 + 1)) {
			error_set("Invalid decompressed size");
			goto err_ptr;
		}

		png_unfilter_8(width_align / 2, height, *dat_ptr, width_align / 2 + 1);

		png_expand_4(width_align, height, *dat_ptr);
	} else if (depth == 2) {
		if (res_size != height * (width_align / 4 + 1)) {
			error_set("Invalid decompressed size");
			goto err_ptr;
		}

		png_unfilter_8(width_align / 4, height, *dat_ptr, width_align / 4 + 1);

		png_expand_2(width_align, height, *dat_ptr);
	} else if (depth == 1) {
		if (res_size != height * (width_align / 8 + 1)) {
			error_set("Invalid decompressed size");
			goto err_ptr;
		}

		png_unfilter_8(width_align / 8, height, *dat_ptr, width_align / 8 + 1);

		png_expand_1(width_align, height, *dat_ptr);
	}

	if (png_read_iend(f, ptr, ptr_size, type)!=0) {
		goto err_ptr;
	}

	free(ptr);
	return 0;

err_ptr:
	free(ptr);
err:
	free(*dat_ptr);
	free(*pal_ptr);
	free(*rns_ptr);
	return -1;
}

/**
 * Load a PNG image.
 * The image is stored in memory as present in the PNG format. It imply that the row scanline
 * is generally greather than the row size.
 * \param pix_width Where to put the image width.
 * \param pix_height Where to put the image height.
 * \param pix_pixel Where to put the image bytes per pixel.
 * \param dat_ptr Where to put the allocated data pointer.
 * \param dat_size Where to put the allocated data size.
 * \param pix_ptr Where to put pointer at the start of the image data.
 * \param pix_scanline Where to put the length of a scanline in bytes.
 * \param pal_ptr Where to put the allocated palette data pointer. Set to 0 if the image is RGB.
 * \param pal_size Where to put the palette size in number of colors. Set to 0 if the image is RGB.
 * \param rns_ptr Where to put the allocated transparency data pointer. Set to 0 if the image hasn't transparency.
 * \param rns_size Where to put the transparency size in number of bytes. Set to 0 if the image hasn't transparency.
 * \param f File to read.
 */
adv_error png_read_rns(
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned char** rns_ptr, unsigned* rns_size,
	adv_fz* f
)
{
	unsigned char* data;
	unsigned type;
	unsigned size;

	if (png_read_signature(f) != 0) {
		goto err;
	}

	do {
		if (png_read_chunk(f, &data, &size, &type) != 0) {
			goto err;
		}

		switch (type) {
			case PNG_CN_IHDR :
				if (png_read_ihdr(pix_width, pix_height, pix_pixel, dat_ptr, dat_size, pix_ptr, pix_scanline, pal_ptr, pal_size, rns_ptr, rns_size, f, data, size) != 0)
					goto err_data;
				return 0;
			default :
				/* ancillary bit. bit 5 of first byte. 0 (uppercase) = critical, 1 (lowercase) = ancillary. */
				if ((type & 0x20000000) == 0) {
					char buf[4];
					be_uint32_write(buf, type);
					error_unsupported_set("Unsupported critical chunk '%c%c%c%c'", buf[0], buf[1], buf[2], buf[3]);
					goto err_data;
				}
				/* ignored */
				break;
		}

		free(data);

	} while (type != PNG_CN_IEND);

	error_set("Invalid PNG file");
	return -1;

err_data:
	free(data);
err:
	return -1;
}

/**
 * Load a PNG image.
 * Like png_read_rns() but without transparency.
 */
adv_error png_read(
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	adv_fz* f
)
{
	adv_error r;
	unsigned char* rns_ptr;
	unsigned rns_size;

	r = png_read_rns(pix_width, pix_height, pix_pixel, dat_ptr, dat_size, pix_ptr, pix_scanline, pal_ptr, pal_size, &rns_ptr, &rns_size, f);

	if (r == 0)
		free(rns_ptr);

	return r;
}


