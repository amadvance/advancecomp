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
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include "portable.h"

#include "fz.h"
#include "endianrw.h"

/* Zip format */
#define ZIP_LO_filename_length 0x1A
#define ZIP_LO_extra_field_length 0x1C
#define ZIP_LO_FIXED 0x1E /* size of fixed data structure */

/**************************************************************************/
/* unlocked interface for streams (faster than locked) */

static FILE* fopen_lock(const char* name, const char* mode)
{
	FILE* f;

	f = fopen(name, mode);

#if HAVE_FLOCKFILE
	if (f)
		flockfile(f);
#endif

	return f;
}

static int fclose_lock(FILE *f)
{
#if HAVE_FUNLOCKFILE
	funlockfile(f);
#endif

	return fclose(f);
}

static size_t fread_lock(void* data, size_t size, size_t count, FILE* f)
{
#if HAVE_FREAD_UNLOCKED
	return fread_unlocked(data, size, count, f);
#else
	return fread(data, size, count, f);
#endif
}

static size_t fwrite_lock(const void* data, size_t size, size_t count, FILE* f)
{
#if HAVE_FWRITE_UNLOCKED
	return fwrite_unlocked(data, size, count, f);
#else
	return fwrite(data, size, count, f);
#endif
}

static int feof_lock(FILE* f)
{
#if HAVE_FEOF_UNLOCKED
	return feof_unlocked(f);
#else
	return feof(f);
#endif
}

static int fgetc_lock(FILE* f)
{
#if HAVE_FGETC_UNLOCKED
	return fgetc_unlocked(f);
#else
	return fgetc(f);
#endif
}

/**************************************************************************/
/* fz interface */

#define INFLATE_INPUT_BUFFER_MAX 4096

adv_error fzgrow(adv_fz* f, unsigned size)
{
	if (f->type == fz_memory_write) {
		if (f->virtual_size < size) {
			unsigned char* data = realloc(f->data_write, size);
			if (!data)
				return -1;
			f->data_write = data;
			f->virtual_size = size;
		}
		return 0;
	} else {
			return -1;
	}
}

/**
 * Read from a file.
 * The semantic is like the C fread() function.
 */
unsigned fzread(void *buffer, unsigned size, unsigned number, adv_fz* f)
{
	if (f->type == fz_file) {
		return fread_lock(buffer, size, number, f->f);
	} else {
		unsigned total_size;

		/* adjust the number of record to read */
		total_size = size * number;
		if (f->virtual_pos + total_size >= f->virtual_size) {
			number = (f->virtual_size - f->virtual_pos) / size;
			if (!number)
				return 0;
			total_size = size * number;
		}

		if (f->type == fz_memory_read) {
			memcpy(buffer, f->data_read + f->virtual_pos, total_size);
			f->virtual_pos += total_size;
			return number;
		} else if (f->type == fz_memory_write) {
			memcpy(buffer, f->data_write + f->virtual_pos, total_size);
			f->virtual_pos += total_size;
			return number;
		} else if (f->type == fz_file_compressed) {
			int err = Z_STREAM_END;
			f->z.next_out = buffer;
			f->z.avail_out = total_size;

			while (f->z.avail_out) {
				if (!f->z.avail_in) {
					unsigned run = INFLATE_INPUT_BUFFER_MAX;
					if (run > f->remaining)
						run = f->remaining;
					f->z.next_in = f->zbuffer;
					run = fread_lock(f->zbuffer, 1, run, f->f);
					f->remaining -= run;
					/* add an extra byte at the end, required by the zlib library */
					if (run && !f->remaining)
						++run;
					f->z.avail_in = run;
				}
				if (!f->z.avail_in)
					break;
				err = inflate(&f->z, Z_NO_FLUSH);
				if (err != Z_OK)
					break;
			}

			total_size -= f->z.avail_out;
			f->virtual_pos += total_size;

			return total_size / size;
		}
	}

	return 0;
}

/**
 * Write to a file.
 * This function works only for files opened with fzopen().
 * The semantic is like the C fwrite() function.
 */
unsigned fzwrite(const void *buffer, unsigned size, unsigned number, adv_fz* f)
{
	if (f->type == fz_file) {
		return fwrite_lock(buffer, size, number, f->f);
	} else if (f->type == fz_memory_write) {
		unsigned total_size;

		/* adjust the number of record to write */
		total_size = size * number;

		if (fzgrow(f, f->virtual_pos + total_size) != 0)
			return -1;

		memcpy(f->data_write + f->virtual_pos, buffer, total_size);
		f->virtual_pos += total_size;
		return number;
	} else {
		return -1;
	}
}

static adv_fz* fzalloc(void)
{
	adv_fz* f;

	f = malloc(sizeof(adv_fz));
	if (!f)
		return 0;

	f->type = fz_invalid;
	f->virtual_pos = 0;
	f->virtual_size = 0;
	f->real_offset = 0;
	f->real_size = 0;
	f->data_read = 0;
	f->data_write = 0;
	f->f = 0;

	return f;
}

/**
 * Open a normal file.
 * The semantic is like the C fopen() function.
 */
adv_fz* fzopen(const char* file, const char* mode)
{
	adv_fz* f = fzalloc();
	if (!f)
		return 0;

	f->type = fz_file;
	f->f = fopen_lock(file, mode);
	if (!f->f) {
		free(f);
		return 0;
	}

	return f;
}

/**
 * Open a normal file doing all the write access in memory.
 * The semantic is like the C fopen() function.
 */
adv_fz* fzopennullwrite(const char* file, const char* mode)
{
	adv_fz* f = fzalloc();
	if (!f)
		goto err;

	f->type = fz_memory_write;
	f->virtual_pos = 0;
	f->f = fopen_lock(file, "rb");
	if (f->f) {
		struct stat st;

		if (fstat(fileno(f->f), &st) != 0) {
			goto err_close;
		}

		f->data_write = malloc(st.st_size);
		if (!f->data_write) {
			goto err_close;
		}

		f->virtual_size = st.st_size;

		if (fread_lock(f->data_write, st.st_size, 1, f->f) != 1) {
			goto err_data;
		}

		fclose_lock(f->f);
		f->f = 0;
	} else {
		if (strchr(mode, 'w')!=0 || strchr(mode, 'a')!=0) {
			/* creation allowed */
			f->data_write = 0;
			f->virtual_size = 0;
		} else {
			/* creation not possible */
			goto err_free;
		}
	}

	return f;

err_data:
	free(f->data_write);
err_close:
	fclose_lock(f->f);
err_free:
	free(f);
err:
	return 0;
}

/**
 * Open an uncompressed file part of a ZIP archive.
 * \param file ZIP archive.
 * \param offset Offset in the archive.
 * \param size Size of the data.
 */
adv_fz* fzopenzipuncompressed(const char* file, unsigned offset, unsigned size)
{
	unsigned char buf[ZIP_LO_FIXED];
	unsigned filename_length;
	unsigned extra_field_length;

	adv_fz* f = fzalloc();
	if (!f)
		return 0;

	f->type = fz_file;
	f->virtual_pos = 0;
	f->virtual_size = size;
	f->f = fopen_lock(file, "rb");
	if (!f->f) {
		free(f);
		return 0;
	}

	if (fseek(f->f, offset, SEEK_SET) != 0) {
		fclose_lock(f->f);
		free(f);
		return 0;
	}
	if (fread_lock(buf, ZIP_LO_FIXED, 1, f->f)!=1) {
		fclose_lock(f->f);
		free(f);
		return 0;
	}
	filename_length = le_uint16_read(buf+ZIP_LO_filename_length);
	extra_field_length = le_uint16_read(buf+ZIP_LO_extra_field_length);

	/* calculate offset to data and seek there */
	offset += ZIP_LO_FIXED + filename_length + extra_field_length;

	f->real_offset = offset;
	f->real_size = size;

	if (fseek(f->f, f->real_offset, SEEK_SET) != 0) {
		fclose_lock(f->f);
		free(f);
		return 0;
	}

	return f;
}

static void compressed_init(adv_fz* f)
{
	int err;

	memset(&f->z, 0, sizeof(f->z));
	f->z.zalloc = 0;
	f->z.zfree = 0;
	f->z.opaque = 0;
	f->z.next_in  = 0;
	f->z.avail_in = 0;
	f->z.next_out = 0;
	f->z.avail_out = 0;

	f->zbuffer = (unsigned char*)malloc(INFLATE_INPUT_BUFFER_MAX+1); /* +1 for the extra byte at the end */
	f->remaining = f->real_size;

	/*
	 * windowBits is passed < 0 to tell that there is no zlib header.
	 * Note that in this case inflate *requires* an extra "dummy" byte
	 * after the compressed stream in order to complete decompression and
	 * return Z_STREAM_END.
	 */
	err = inflateInit2(&f->z, -MAX_WBITS);

	assert(err == Z_OK);
}

static void compressed_done(adv_fz* f)
{
	inflateEnd(&f->z);
	free(f->zbuffer);
}

/**
 * Open an compressed file part of a ZIP archive.
 * \param file ZIP archive.
 * \param offset Offset in the archive.
 * \param size_compressed Size of the compressed data.
 * \param size_uncompressed Size of the uncompressed data.
 */
adv_fz* fzopenzipcompressed(const char* file, unsigned offset, unsigned size_compressed, unsigned size_uncompressed)
{
	unsigned char buf[ZIP_LO_FIXED];
	unsigned filename_length;
	unsigned extra_field_length;

	adv_fz* f = fzalloc();
	if (!f)
		return 0;

	f->type = fz_file_compressed;
	f->virtual_pos = 0;
	f->virtual_size = size_uncompressed;
	f->f = fopen_lock(file, "rb");
	if (!f->f) {
		free(f);
		return 0;
	}

	if (fseek(f->f, offset, SEEK_SET) != 0) {
		fclose_lock(f->f);
		free(f);
		return 0;
	}
	if (fread_lock(buf, ZIP_LO_FIXED, 1, f->f)!=1) {
		fclose_lock(f->f);
		free(f);
		return 0;
	}
	filename_length = le_uint16_read(buf+ZIP_LO_filename_length);
	extra_field_length = le_uint16_read(buf+ZIP_LO_extra_field_length);

	/* calculate offset to data and seek there */
	offset += ZIP_LO_FIXED + filename_length + extra_field_length;

	f->real_offset = offset;
	f->real_size = size_compressed;

	if (fseek(f->f, f->real_offset, SEEK_SET) != 0) {
		fclose_lock(f->f);
		free(f);
		return 0;
	}

	compressed_init(f);

	return f;
}

/**
 * Open a memory range as a file.
 * \param data Data.
 * \param size Size of the data.
 */
adv_fz* fzopenmemory(const unsigned char* data, unsigned size)
{
	adv_fz* f = fzalloc();
	if (!f)
		return 0;

	f->type = fz_memory_read;
	f->virtual_pos = 0;
	f->virtual_size = size;
	f->data_read = data;

	return f;
}

/**
 * Close a file.
 * The semantic is like the C fclose() function.
 */
adv_error fzclose(adv_fz* f)
{
	if (f->type == fz_file) {
		fclose_lock(f->f);
		free(f);
	} else if (f->type == fz_file_part) {
		fclose_lock(f->f);
		free(f);
	} else if (f->type == fz_file_compressed) {
		compressed_done(f);
		fclose_lock(f->f);
		free(f);
	} else if (f->type == fz_memory_read) {
		free(f);
	} else if (f->type == fz_memory_write) {
		free(f->data_write);
		free(f);
	} else {
		return -1;
	}

	return 0;
}

/**
 * Read a char from the file.
 * The semantic is like the C fgetc() function.
 * It assume to read a binary file without any "\r", "\n" conversion.
 */
int fzgetc(adv_fz* f)
{
	if (f->type == fz_file) {
		return fgetc_lock(f->f);
	} else {
		unsigned char r;
		if (fzread(&r, 1, 1, f)==1)
			return r;
		else
			return EOF;
	}
}


/**
 * Unread a char from the file.
 * This function works only for files opened with fzopen(). 
 * The semantic is like the C fungetc() function.
 */
adv_error fzungetc(int c, adv_fz* f)
{
	if (f->type == fz_file) {
		return ungetc(c, f->f);
	} else {
		return -1;
	}
}

/**
 * Read a string from the file.
 * The semantic is like the C fgets() function.
 */
char* fzgets(char *s, int n, adv_fz* f)
{
	char* r;

	if (n < 2) {
		r = 0;
	} else {
		int c = fzgetc(f);
		if (c == EOF) {
			r = 0;
		} else {
			r = s;
			while (n > 1) {
				*s++ = c;
				--n;
				if (c == '\n')
					break;
				if (n > 1)
					c = fzgetc(f);
			}
			if (n) {
				*s = 0;
			} else {
				r = 0;
			}
		}
	}

	return r;
}

/**
 * Check if the file pointer is at the end.
 * The semantic is like the C feof() function.
 */
adv_error fzeof(adv_fz* f)
{
	if (f->type == fz_file) {
		return feof_lock(f->f);
	} else {
		return f->virtual_pos >= f->virtual_size;
	}
}

/**
 * Get the current position in the file.
 * The semantic is like the C ftell() function.
 */
long fztell(adv_fz* f)
{
	if (f->type == fz_file) {
		return ftell(f->f);
	} else {
		return f->virtual_pos;
	}
}

/**
 * Get the size of the file.
 */
long fzsize(adv_fz* f)
{
	if (f->type == fz_file) {
		struct stat st;
		if (fstat(fileno(f->f), &st) != 0) {
			return -1;
		}
		return st.st_size;
	} else {
		return f->virtual_size;
	}
}

/**
 * Seek to an arbitrary position.
 * The semantic is like the C fseek() function.
 */
adv_error fzseek(adv_fz* f, long offset, int mode)
{
	if (f->type == fz_file) {
		switch (mode) {
			case SEEK_SET :
				return fseek(f->f, f->real_offset + offset, SEEK_SET);
			case SEEK_CUR :
				return fseek(f->f, offset, SEEK_CUR);
			case SEEK_END :
				if (f->real_size)
					return fseek(f->f, f->real_size - offset, SEEK_SET);
				else
					return fseek(f->f, offset, SEEK_END);
			default:
				return -1;
		}
	} else {
		int pos;
		switch (mode) {
			case SEEK_SET :
				pos = offset;
				break;
			case SEEK_CUR :
				pos = f->virtual_pos + offset;
				break;
			case SEEK_END :
				pos = f->virtual_size - offset;
				break;
			default:
				return -1;
		}
		if (pos < 0 || pos > f->virtual_size)
			return -1;

		if (f->type == fz_memory_read) {
			f->virtual_pos = pos;
			return 0;
		} else if (f->type == fz_memory_write) {
			if (fzgrow(f, pos) != 0)
				return -1;
			f->virtual_pos = pos;
			return 0;
		} else if (f->type == fz_file_part) {
			if (fseek(f->f, f->real_offset + f->virtual_pos, SEEK_SET) != 0)
				return -1;
			f->virtual_pos = pos;
			return 0;
		} else if (f->type == fz_file_compressed) {
			unsigned offset;

			if (pos < f->virtual_pos) {
				/* if backward reopen the file */
				int err;
				compressed_done(f);
				err = fseek(f->f, f->real_offset, SEEK_SET);
				f->virtual_pos = 0;
				compressed_init(f);
				if (err != 0)
					return -1;
			}

			/* data to read */
			offset = pos - f->virtual_pos;

			/* read all the data */
			while (offset > 0) {
				unsigned char buffer[256];
				unsigned run = offset;
				if (run > 256)
					run = 256;
				if (fzread(buffer, run, 1, f) != 1)
					return -1;
				offset -= run;
			}

			return 0;
		} else {
			return -1;
		}
	}
}

adv_error le_uint8_fzread(adv_fz* f, unsigned* v)
{
	unsigned char p[1];

	if (fzread(p, 1, 1, f) != 1)
		return -1;
	*v = le_uint8_read(p);

	return 0;
}

adv_error le_uint16_fzread(adv_fz* f, unsigned* v)
{
	unsigned char p[2];

	if (fzread(p, 2, 1, f) != 1)
		return -1;
	*v = le_uint16_read(p);

	return 0;
}

adv_error le_uint32_fzread(adv_fz* f, unsigned* v)
{
	unsigned char p[4];

	if (fzread(p, 4, 1, f) != 1)
		return -1;
	*v = le_uint32_read(p);

	return 0;
}

