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

#include "portable.h"

#include "mng.h"
#include "png.h"
#include "endianrw.h"
#include "error.h"

/**************************************************************************************/
/* MNG */

static unsigned char MNG_Signature[] = "\x8A\x4D\x4E\x47\x0D\x0A\x1A\x0A";

/**
 * Read the MNG file signature.
 * \param f File to read. 
 */
adv_error adv_mng_read_signature(adv_fz* f)
{
	unsigned char signature[8];

	if (fzread(signature, 8, 1, f) != 1) {
		error_set("Error reading the signature");
		return -1;
	}

	if (memcmp(signature, MNG_Signature, 8)!=0) {
		error_set("Invalid MNG signature");
		return -1;
	}

	return 0;
}

/**
 * Write the MNG file signature.
 * \param f File to write.
 * \param count Pointer at the number of bytes written. It may be 0.
 */
adv_error adv_mng_write_signature(adv_fz* f, unsigned* count)
{
	if (fzwrite(MNG_Signature, 8, 1, f) != 1) {
		error_set("Error writing the signature");
		return -1;
	}

	if (count)
		*count += 8;

	return 0;
}

static adv_error mng_read_ihdr(adv_mng* mng, adv_fz* f, const unsigned char* ihdr, unsigned ihdr_size)
{
	unsigned type;
	unsigned char* data;
	unsigned size;
	unsigned long dat_size;

	if (ihdr_size != 13) {
		error_set("Invalid IHDR size");
		goto err;
	}

	mng->dat_width = be_uint32_read(ihdr + 0);
	mng->dat_height = be_uint32_read(ihdr + 4);
	if (mng->dat_x + mng->frame_width > mng->dat_width) {
		error_set("Frame not complete");
		goto err;
	}
	if (mng->dat_y + mng->frame_height > mng->dat_height) {
		error_set("Frame not complete");
		goto err;
	}

	if (ihdr[8] != 8) { /* bit depth */
		error_unsupported_set("Unsupported bit depth");
		goto err;
	}
	if (ihdr[9] == 3) /* color type */
		mng->pixel = 1;
	else if (ihdr[9] == 2)
		mng->pixel = 3;
	else if (ihdr[9] == 6)
		mng->pixel = 4;
	else {
		error_unsupported_set("Unsupported color type");
		goto err;
	}
	if (ihdr[10] != 0) { /* compression */
		error_unsupported_set("Unsupported compression type");
		goto err;
	}
	if (ihdr[11] != 0) { /* filter */
		error_unsupported_set("unsupported Filter type");
		goto err;
	}
	if (ihdr[12] != 0) { /* interlace */
		error_unsupported_set("Unsupported interlace type");
		goto err;
	}

	if (!mng->dat_ptr) {
		mng->dat_line = mng->dat_width * mng->pixel + 1; /* +1 for the filter byte */
		mng->dat_size = mng->dat_height * mng->dat_line;
		mng->dat_ptr = malloc(mng->dat_size);
	} else {
		if (mng->dat_line != mng->dat_width * mng->pixel + 1
			|| mng->dat_size != mng->dat_height * mng->dat_line) {
			error_unsupported_set("Unsupported size change");
			goto err;
		}
	}

	if (mng->pixel == 1) {
		if (adv_png_read_chunk(f, &data, &size, &type) != 0)
			goto err;

		if (type != ADV_PNG_CN_PLTE) {
			error_set("Missing PLTE chunk");
			goto err_data;
		}

		if (size % 3 != 0 || size / 3 > 256) {
			error_set("Invalid palette size in PLTE chunk");
			goto err_data;
		}

		mng->pal_size = size;
		memcpy(mng->pal_ptr, data, size);

		free(data);
	} else {
		mng->pal_size = 0;
	}

	if (adv_png_read_chunk(f, &data, &size, &type) != 0)
		goto err;

	if (type != ADV_PNG_CN_IDAT) {
		error_set("Missing IDAT chunk");
		goto err_data;
	}

	dat_size = mng->dat_size;
	if (uncompress(mng->dat_ptr, &dat_size, data, size) != Z_OK) {
		error_set("Corrupt compressed data");
		goto err_data;
	}

	free(data);

	if (dat_size != mng->dat_size) {
		error_set("Corrupt compressed data");
		goto err;
	}

	if (mng->pixel == 1)
		adv_png_unfilter_8(mng->dat_width * mng->pixel, mng->dat_height, mng->dat_ptr, mng->dat_line);
	else if (mng->pixel == 3)
		adv_png_unfilter_24(mng->dat_width * mng->pixel, mng->dat_height, mng->dat_ptr, mng->dat_line);
	else if (mng->pixel == 4)
		adv_png_unfilter_32(mng->dat_width * mng->pixel, mng->dat_height, mng->dat_ptr, mng->dat_line);

	if (adv_png_read_chunk(f, &data, &size, &type) != 0)
		goto err;

	if (adv_png_read_iend(f, data, size, type) != 0)
		goto err_data;

	free(data);

	return 0;

err_data:
	free(data);
err:
	return -1;
}

static adv_error mng_read_defi(adv_mng* mng, unsigned char* defi, unsigned defi_size)
{
	unsigned id;

	if (defi_size != 4 && defi_size != 12) {
		error_unsupported_set("Unsupported DEFI size");
		return -1;
	}

	id = be_uint16_read(defi + 0);
	if (id != 1) {
		error_unsupported_set("Unsupported id number in DEFI chunk");
		return -1;
	}
	if (defi[2] != 0) { /* visible */
		error_unsupported_set("Unsupported visible type in DEFI chunk");
		return -1;
	}
	if (defi[3] != 1) { /* concrete */
		error_unsupported_set("Unsupported concrete type in DEFI chunk");
		return -1;
	}

	if (defi_size >= 12) {
		mng->dat_x = - (int)be_uint32_read(defi + 4);
		mng->dat_y = - (int)be_uint32_read(defi + 8);
	} else {
		mng->dat_x = 0;
		mng->dat_y = 0;
	}

	return 0;
}

static adv_error mng_read_move(adv_mng* mng, adv_fz* f, unsigned char* move, unsigned move_size)
{
	unsigned id;

	if (move_size != 13) {
		error_unsupported_set("Unsupported MOVE size in MOVE chunk");
		return -1;
	}

	id = be_uint16_read(move + 0);
	if (id != 1) {
		error_unsupported_set("Unsupported id number in MOVE chunk");
		return -1;
	}

	id = be_uint16_read(move + 2);
	if (id != 1) {
		error_unsupported_set("Unsupported id number in MOVE chunk");
		return -1;
	}

	if (move[4] == 0) { /* replace */
		mng->dat_x = - (int)be_uint32_read(move + 5);
		mng->dat_y = - (int)be_uint32_read(move + 9);
	} else if (move[4] == 1) { /* adding */
		mng->dat_x += - (int)be_uint32_read(move + 5);
		mng->dat_y += - (int)be_uint32_read(move + 9);
	} else {
		error_unsupported_set("Unsupported move type in MOVE chunk");
		return -1;
	}

	return 0;
}

static void mng_delta_replacement(adv_mng* mng, unsigned pos_x, unsigned pos_y, unsigned width, unsigned height)
{
	unsigned i;
	unsigned bytes_per_run = width * mng->pixel;
	unsigned delta_bytes_per_scanline = bytes_per_run + 1;
	unsigned char* p0 = mng->dat_ptr + pos_y * mng->dat_line + pos_x * mng->pixel + 1;
	unsigned char* p1 = mng->dlt_ptr + 1;

	for(i=0;i<height;++i) {
		memcpy(p0, p1, bytes_per_run);
		p0 += mng->dat_line;
		p1 += delta_bytes_per_scanline;
	}
}

static void mng_delta_addition(adv_mng* mng, unsigned pos_x, unsigned pos_y, unsigned width, unsigned height)
{
	unsigned i, j;
	unsigned bytes_per_run = width * mng->pixel;
	unsigned delta_bytes_per_scanline = bytes_per_run + 1;
	unsigned char* p0 = mng->dat_ptr + pos_y * mng->dat_line + pos_x * mng->pixel + 1;
	unsigned char* p1 = mng->dlt_ptr + 1;

	for(i=0;i<height;++i) {
		for(j=0;j<bytes_per_run;++j) {
			*p0++ += *p1++;
		}
		p0 += mng->dat_line - bytes_per_run;
		p1 += delta_bytes_per_scanline - bytes_per_run;
	}
}

static adv_error mng_read_delta(adv_mng* mng, adv_fz* f, unsigned char* dhdr, unsigned dhdr_size)
{
	unsigned type;
	unsigned char* data;
	unsigned size;
	unsigned width;
	unsigned height;
	unsigned pos_x;
	unsigned pos_y;
	unsigned id;
	unsigned ope;

	if (dhdr_size != 4 && dhdr_size != 12 && dhdr_size != 20) {
		error_unsupported_set("Unsupported DHDR size");
		goto err;
	}

	id = be_uint16_read(dhdr + 0);
	if (id != 1) /* object id 1 */ {
		error_unsupported_set("Unsupported id number in DHDR chunk");
		goto err;
	}

	if (dhdr[2] != 1) /* PNG stream without IHDR header */ {
		error_unsupported_set("Unsupported delta type in DHDR chunk");
		goto err;
	}

	ope = dhdr[3];
	if (ope != 0 && ope != 1 && ope != 4 && ope != 7) {
		error_unsupported_set("Unsupported delta operation in DHDR chunk");
		goto err;
	}

	if (!mng->dat_ptr) {
		error_set("Invalid delta context in DHDR chunk");
		goto err;
	}

	if (!mng->dlt_ptr) {
		mng->dlt_line = mng->frame_width * mng->pixel + 1; /* +1 for the filter byte */
		mng->dlt_size = mng->frame_height * mng->dlt_line;
		mng->dlt_ptr = malloc(mng->dlt_size);
	}

	if (dhdr_size >= 12) {
		width = be_uint32_read(dhdr + 4);
		height = be_uint32_read(dhdr + 8);
	} else {
		width = mng->frame_width;
		height = mng->frame_height;
	}

	if (dhdr_size >= 20) {
		pos_x = be_uint32_read(dhdr + 12);
		pos_y = be_uint32_read(dhdr + 16);
	} else {
		pos_x = 0;
		pos_y = 0;
	}

	if (adv_png_read_chunk(f, &data, &size, &type) != 0)
		goto err;

	if (type == ADV_PNG_CN_PLTE) {
		if (mng->pixel != 1) {
			error_set("Unexpected PLTE chunk");
			goto err_data;
		}

		if (size % 3 != 0 || size / 3 > 256) {
			error_set("Invalid palette size");
			goto err_data;
		}

		mng->pal_size = size;
		memcpy(mng->pal_ptr, data, size);

		free(data);

		if (adv_png_read_chunk(f, &data, &size, &type) != 0)
			goto err;
	}

	if (type == ADV_MNG_CN_PPLT) {
		unsigned i;
		if (mng->pixel != 1) {
			error_set("Unexpected PPLT chunk");
			goto err_data;
		}

		if (data[0] != 0) { /* RGB replacement */
			error_set("Unsupported palette operation in PPLT chunk");
			goto err_data;
		}

		i = 1;
		while (i < size) {
			unsigned v0, v1, delta_size;
			if (i + 2 > size) {
				error_set("Invalid palette size in PPLT chunk");
				goto err_data;
			}
			v0 = data[i++];
			v1 = data[i++];
			delta_size = (v1 - v0 + 1) * 3;
			if (i + delta_size > size) {
				error_set("Invalid palette format in PPLT chunk");
				goto err_data;
			}
			memcpy(mng->pal_ptr + v0 * 3, data + i, delta_size);
			i += delta_size;
		}

		free(data);

		if (adv_png_read_chunk(f, &data, &size, &type) != 0)
			goto err;
	}

	if (type == ADV_PNG_CN_IDAT) {
		unsigned long dlt_size;

		if (pos_x + width > mng->dat_width || pos_y + height > mng->dat_height) {
			error_set("Frame not complete in IDAT chunk");
			goto err_data;
		}

		dlt_size = mng->dat_size;
		if (uncompress(mng->dlt_ptr, &dlt_size, data, size) != Z_OK) {
			error_set("Corrupt compressed data in IDAT chunk");
			goto err_data;
		}

		if (ope == 0 || ope == 4) {
			mng_delta_replacement(mng, pos_x, pos_y, width, height);
		} else if (ope == 1) {
			mng_delta_addition(mng, pos_x, pos_y, width, height);
		} else {
			error_unsupported_set("Unsupported delta operation");
			goto err_data;
		}

		free(data);

		if (adv_png_read_chunk(f, &data, &size, &type) != 0)
			goto err;
	} else {
		if (ope != 7) {
			error_unsupported_set("Unsupported delta operation");
			goto err_data;
		}
	}

	if (adv_png_read_iend(f, data, size, type) != 0)
		goto err_data;

	free(data);

	return 0;

err_data:
	free(data);
err:
	return -1;
}

static void mng_import(
	adv_mng* mng,
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	adv_bool own)
{
	unsigned char* current_ptr = mng->dat_ptr + mng->dat_x * mng->pixel + mng->dat_y * mng->dat_line + 1;

	*pix_width = mng->frame_width;
	*pix_height = mng->frame_height;
	*pix_pixel = mng->pixel;

	if (mng->pixel == 1) {
		*pal_ptr = malloc(mng->pal_size);
		memcpy(*pal_ptr, mng->pal_ptr, mng->pal_size);
		*pal_size = mng->pal_size;
	} else {
		*pal_ptr = 0;
		*pal_size = 0;
	}

	if (own) {
		*dat_ptr = mng->dat_ptr;
		*dat_size = mng->dat_size;
	} else {
		*dat_ptr = 0;
		*dat_size = 0;
	}

	*pix_ptr = current_ptr;
	*pix_scanline = mng->dat_line;
}

static adv_error mng_read(
	adv_mng* mng,
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned* tick,
	adv_fz* f,
	adv_bool own)
{
	unsigned type;
	unsigned char* data;
	unsigned size;

	if (mng->end_flag)
		return -1;

	*tick = mng->frame_tick;

	while (1) {
		if (adv_png_read_chunk(f, &data, &size, &type) != 0)
			goto err;

		switch (type) {
			case ADV_MNG_CN_DEFI :
				if (mng_read_defi(mng, data, size) != 0)
					goto err_data;
				free(data);
				break;
			case ADV_MNG_CN_MOVE :
				if (mng_read_move(mng, f, data, size) != 0)
					goto err_data;
				free(data);
				break;
			case ADV_PNG_CN_IHDR :
				if (mng_read_ihdr(mng, f, data, size) != 0)
					goto err_data;
				free(data);
				mng_import(mng, pix_width, pix_height, pix_pixel, dat_ptr, dat_size, pix_ptr, pix_scanline, pal_ptr, pal_size, own);
				return 0;
			case ADV_MNG_CN_DHDR :
				if (mng_read_delta(mng, f, data, size) != 0)
					goto err_data;
				free(data);
				mng_import(mng, pix_width, pix_height, pix_pixel, dat_ptr, dat_size, pix_ptr, pix_scanline, pal_ptr, pal_size, own);
				return 0;
			case ADV_MNG_CN_MEND :
				mng->end_flag = 1;
				free(data);
				return 1;
			case ADV_MNG_CN_FRAM :
				if (size > 1) {
					unsigned i = 1;
					while (i < size && data[i])
						++i;
					if (size >= i+9) {
						unsigned v = be_uint32_read(data + i+5);
						if (v < 1)
							v = 1;
						if (data[i+1] == 1 || data[i+1] == 2)
							*tick = v;
						if (data[i+1] == 2)
							mng->frame_tick = v;
					}
				}
				free(data);
				break;
			case ADV_MNG_CN_BACK :
				/* ignored OUTOFSPEC */
				free(data);
				break;
			case ADV_MNG_CN_LOOP :
			case ADV_MNG_CN_ENDL :
			case ADV_MNG_CN_SAVE :
			case ADV_MNG_CN_SEEK :
			case ADV_MNG_CN_TERM :
				/* ignored */
				free(data);
				break;
			default :
				/* ancillary bit. bit 5 of first byte. 0 (uppercase) = critical, 1 (lowercase) = ancillary. */
				if ((type & 0x20000000) == 0) {
					char buf[4];
					be_uint32_write(buf, type);
					error_unsupported_set("Unsupported critical chunk '%c%c%c%c'", buf[0], buf[1], buf[2], buf[3]);
					goto err_data;
				}
				/* ignored */
				free(data);
				break;
		}
	}

	free(data);

	return 1;

err_data:
	free(data);
err:
	return -1;
}

/**
 * Read a MNG image.
 * This function returns always a null dat_ptr pointer because the
 * referenced image is stored in the mng context.
 * \param mng MNG context previously returned by mng_init().
 * \param pix_width Where to put the image width.
 * \param pix_height Where to put the image height.
 * \param pix_pixel Where to put the image bytes per pixel.
 * \param dat_ptr Where to put the allocated data pointer.
 * \param dat_size Where to put the allocated data size.
 * \param pix_ptr Where to put pointer at the start of the image data.
 * \param pix_scanline Where to put the length of a scanline in bytes.
 * \param pal_ptr Where to put the allocated palette data pointer. Set to 0 if the image is RGB.
 * \param pal_size Where to put the palette size in number of colors. Set to 0 if the image is RGB.
 * \param tick Where to put the number of tick of the frame. The effective time can be computed dividing by mng_frequency_get().
 * \param f File to read. 
 * \return
 *   - == 0 ok
 *   - == 1 end of the mng stream
 *   - < 0 error
 */
adv_error adv_mng_read(
	adv_mng* mng,
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned* tick,
	adv_fz* f)
{
	return mng_read(mng, pix_width, pix_height, pix_pixel,
		dat_ptr, dat_size, pix_ptr, pix_scanline,
		pal_ptr, pal_size, tick, f, 0);
}

/**
 * Read a MNG image and implicitely call adv_mng_done().
 * This function returns always a not null dat_ptr pointer.
 * The mng context is always destroyed. Also if an error
 * is returned.
 */
adv_error adv_mng_read_done(
	adv_mng* mng,
	unsigned* pix_width, unsigned* pix_height, unsigned* pix_pixel,
	unsigned char** dat_ptr, unsigned* dat_size,
	unsigned char** pix_ptr, unsigned* pix_scanline,
	unsigned char** pal_ptr, unsigned* pal_size,
	unsigned* tick,
	adv_fz* f)
{
	adv_error r;

	r = mng_read(mng, pix_width, pix_height, pix_pixel,
		dat_ptr, dat_size, pix_ptr, pix_scanline,
		pal_ptr, pal_size, tick, f, 1);

	if (r != 0)
		free(mng->dat_ptr);
	free(mng->dlt_ptr);
	free(mng);

	return r;
}

/**
 * Initialize a MNG reading stream.
 * \param f File to read.
 * \return Return the MNG context. It must be destroied calling mng_done(). On error return 0.
 */
adv_mng* adv_mng_init(adv_fz* f)
{
	adv_mng* mng;

	unsigned type;
	unsigned char* data;
	unsigned size;
	unsigned simplicity;

	mng = malloc(sizeof(adv_mng));
	if (!mng)
		goto err;

	mng->end_flag = 0;
	mng->pixel = 0;
	mng->dat_ptr = 0;
	mng->dat_size = 0;
	mng->dat_line = 0;
	mng->dat_x = 0;
	mng->dat_y = 0;
	mng->dat_width = 0;
	mng->dat_height = 0;
	mng->dlt_ptr = 0;
	mng->dlt_size = 0;
	mng->dlt_line = 0;
	mng->pal_size = 0;

	if (adv_mng_read_signature(f) != 0)
		goto err_mng;

	if (adv_png_read_chunk(f, &data, &size, &type) != 0)
		goto err_mng;

	if (type != ADV_MNG_CN_MHDR) {
		error_set("Missing MHDR chunk\n");
		goto err_data;
	}

	if (size != 28) {
		error_set("Invalid MHDR size\n");
		goto err_data;
	}

	mng->frame_width = be_uint32_read(data + 0);
	mng->frame_height = be_uint32_read(data + 4);
	mng->frame_frequency = be_uint32_read(data + 8);
	if (mng->frame_frequency < 1)
		mng->frame_frequency = 1;
	mng->frame_tick = 1;
	simplicity = be_uint32_read(data + 24);

	free(data);

	return mng;

err_data:
	free(data);
err_mng:
	free(mng);
err:
	return 0;
}

/**
 * Destory a MNG context.
 * \param mng MNG context previously returned by mng_init().
 */
void adv_mng_done(adv_mng* mng)
{
	free(mng->dat_ptr);
	free(mng->dlt_ptr);
	free(mng);
}

/**
 * Get the base frequency of the MNG.
 * This value can be used to convert the number of tick per frame in a time.
 * \param mng MNG context.
 * \return Frequency in Hz.
 */
unsigned adv_mng_frequency_get(adv_mng* mng)
{
	return mng->frame_frequency;
}

/**
 * Get the width of the frame.
 * \param mng MNG context.
 * \return Width in pixel.
 */
unsigned adv_mng_width_get(adv_mng* mng)
{
	return mng->frame_width;
}

/**
 * Get the height of the frame.
 * \param mng MNG context.
 * \return height in pixel.
 */
unsigned adv_mng_height_get(adv_mng* mng)
{
	return mng->frame_height;
}

adv_error adv_mng_write_mhdr(
	unsigned pix_width, unsigned pix_height,
	unsigned frequency, adv_bool is_lc,
	adv_fz* f, unsigned* count)
{
	uint8 mhdr[28];
	unsigned simplicity;

	simplicity = (1 << 0) /* Enable flags */
		| (1 << 6); /* Enable flags */
	if (is_lc)
		simplicity |= (1 << 1); /* Basic features */

	memset(mhdr, 0, 28);
	be_uint32_write(mhdr, pix_width);
	be_uint32_write(mhdr + 4, pix_height);
	be_uint32_write(mhdr + 8, frequency);
	be_uint32_write(mhdr + 24, simplicity);

	if (adv_png_write_chunk(f, ADV_MNG_CN_MHDR, mhdr, 28, count)!=0)
		return -1;

	return 0;
}

adv_error adv_mng_write_mend(adv_fz* f, unsigned* count)
{
	if (adv_png_write_chunk(f, ADV_MNG_CN_MEND, 0, 0, count)!=0)
		return -1;

	return 0;
}

adv_error adv_mng_write_fram(unsigned tick, adv_fz* f, unsigned* count)
{
	uint8 fram[10];
	unsigned fi;

	fi = tick;
	if (fi < 1)
		fi = 1;

	fram[0] = 1; /* Framing_mode: 1 */
	fram[1] = 0; /* Null byte */
	fram[2] = 2; /* Reset delay */
	fram[3] = 0; /* No timeout change */
	fram[4] = 0; /* No clip change */
	fram[5] = 0; /* No sync id change */
	be_uint32_write(fram+6, fi); /* Delay in tick */

	if (adv_png_write_chunk(f, ADV_MNG_CN_FRAM, fram, 10, count)!=0)
		return -1;

	return 0;
}

