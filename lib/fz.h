/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999, 2000, 2001, 2002, 2003 Andrea Mazzoleni
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

/** \file
 * Generic compressed file support.
 */

#ifndef __FZ_H
#define __FZ_H

#include "extra.h"

#include <zlib.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Types of files supported.
 */
enum adv_fz_enum {
	fz_invalid, /**< Invalid file. */
	fz_file, /**< Real file. Read and write.*/
	fz_file_part, /**< Part of a real file. Read and write but not expandible. */
	fz_file_compressed, /**< ZLIB compressed part of a file. Only read. */
	fz_memory_read, /**< Memory image of a file. Only read. */
	fz_memory_write /**< Memory image of a file. Read and write. */
};

/**
 * Compressed file context.
 */
typedef struct adv_fz_struct {
	unsigned type; /**< Type of file. One of the ::adv_fz_enum. */
	unsigned virtual_pos; /**< Current position on the virtual file. */
	unsigned virtual_size; /**< Size of the virtual file. */

	unsigned real_offset; /**< Starting position on file part used. */
	unsigned real_size; /**< Size of the file part used. */

	const unsigned char* data_read; /**< Memory used by the file. */
	unsigned char* data_write; /**< Memory used by the file which need to be freed. */

	FILE* f; /**< Handler used by the file. */

	z_stream z; /**< Compressed stream. */
	unsigned char* zbuffer; /**< Buffer used for reading a compressed stream. */
	unsigned remaining; /**< Remainig bytes of a compressed stream. */
} adv_fz;

/** \addtogroup CompressedFile */
/*@{*/

adv_fz* fzopen(const char* file, const char* mode);
adv_fz* fzopennullwrite(const char* file, const char* mode);
adv_fz* fzopenzipuncompressed(const char* file, unsigned offset, unsigned size);
adv_fz* fzopenzipcompressed(const char* file, unsigned offset, unsigned size_compressed, unsigned size_uncompressed);
adv_fz* fzopenmemory(const unsigned char* data, unsigned size);

unsigned fzread(void *buffer, unsigned size, unsigned number, adv_fz* f);
unsigned fzwrite(const void *buffer, unsigned size, unsigned number, adv_fz* f);
adv_error fzclose(adv_fz* f);
long fztell(adv_fz* f);
long fzsize(adv_fz* f);
adv_error fzseek(adv_fz* f, long offset, int mode);
int fzgetc(adv_fz* f);
adv_error fzungetc(int c, adv_fz* f);
char* fzgets(char *s, int n, adv_fz* f);
adv_error fzeof(adv_fz* f);
adv_error le_uint8_fzread(adv_fz* f, unsigned* v);
adv_error le_uint16_fzread(adv_fz* f, unsigned* v);
adv_error le_uint32_fzread(adv_fz* f, unsigned* v);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif

