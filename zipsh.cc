/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2004, 2005 Andrea Mazzoleni
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

#include "zip.h"
#include "data.h"

#include <zlib.h>

#include <iostream>

using namespace std;

void zip_entry::uncompressed_read(unsigned char* uncompressed_data) const
{
	assert(data);

	if (info.compression_method == ZIP_METHOD_DEFLATE) {
		if (!decompress_deflate_zlib(data, compressed_size_get(), uncompressed_data, uncompressed_size_get())) {
			throw error_invalid() << "Invalid compressed data on file " << name_get();
		}
#ifdef USE_BZIP2
	} else if (info.compression_method == ZIP_METHOD_BZIP2) {
		if (!decompress_bzip2(data, compressed_size_get(), uncompressed_data, uncompressed_size_get())) {
			throw error_invalid() << "Invalid compressed data on file " << name_get();
		}
#endif
#ifdef USE_LZMA
	} else if (info.compression_method == ZIP_METHOD_LZMA) {
		if (!decompress_lzma_7z(data, compressed_size_get(), uncompressed_data, uncompressed_size_get())) {
			throw error_invalid() << "Invalid compressed data on file " << name_get();
		}
#endif
	} else if (info.compression_method == ZIP_METHOD_STORE) {
		memcpy(uncompressed_data, data, uncompressed_size_get());
	} else {
		throw error_unsupported() << "Unsupported compression method on file " << name_get();
	}
}

void zip_entry::test() const
{
	assert(data);

	unsigned char* uncompressed_data = data_alloc(uncompressed_size_get());

	try {
		uncompressed_read(uncompressed_data);

		if (info.crc32 != crc32(0, uncompressed_data, uncompressed_size_get())) {
			throw error_invalid() << "Invalid crc on file " << name_get();
		}
	} catch (...) {
		data_free(uncompressed_data);
		throw;
	}

	data_free(uncompressed_data);
}

static bool got(unsigned char* c0_data, unsigned c0_size, unsigned c0_met, unsigned char* c1_data, unsigned c1_size, unsigned c1_met, bool substitute_if_equal, bool standard, bool store)
{
	bool c0_acceptable = c0_data!=0;
	bool c1_acceptable = c1_data!=0;

	if (standard) {
		c0_acceptable = c0_acceptable && (c0_met <= ZIP_METHOD_DEFLATE);
		c1_acceptable = c1_acceptable && (c1_met <= ZIP_METHOD_DEFLATE);
	}

	if (store) {
		c0_acceptable = c0_acceptable && (c0_met == ZIP_METHOD_STORE);
		c1_acceptable = c1_acceptable && (c1_met == ZIP_METHOD_STORE);
	}

	if (!c1_acceptable)
		return false;

	if (!c0_acceptable)
		return true;

	if (c0_size > c1_size)
		return true;

	if (substitute_if_equal && c0_size == c1_size)
		return true;

	return false;
}

bool zip_entry::shrink(bool standard, shrink_t level)
{
	assert(data);

	bool modify = false;

	// remove unneeded data
	if (info.local_extra_field_length != 0)
		modify = true;
	data_free(local_extra_field);
	local_extra_field = 0;
	info.local_extra_field_length = 0;

	if (info.central_extra_field_length != 0)
		modify = true;
	data_free(central_extra_field);
	central_extra_field = 0;
	info.central_extra_field_length = 0;

	if (info.file_comment_length != 0)
		modify = true;
	data_free(file_comment);
	file_comment = 0;
	info.file_comment_length = 0;

	unsigned char* uncompressed_data = data_alloc(uncompressed_size_get());

	try {
		uncompressed_read(uncompressed_data);

		if (info.crc32 != crc32(0, uncompressed_data, uncompressed_size_get())) {
			throw error_invalid() << "Invalid crc on file " << name_get();
		}

	} catch (...) {
		data_free(uncompressed_data);
		throw;
	}

	unsigned char* c0_data;
	unsigned c0_size;
	unsigned c0_ver;
	unsigned c0_met;
	unsigned c0_fla;

	// previous compressed data
	c0_data = data;
	c0_size = info.compressed_size;
	c0_ver = info.version_needed_to_extract;
	c0_met = info.compression_method;
	c0_fla = info.general_purpose_bit_flag;

	if (level != shrink_none) {
		// test compressed data
		unsigned char* c1_data;
		unsigned c1_size;
		unsigned c1_ver;
		unsigned c1_met;
		unsigned c1_fla;

#if defined(USE_7Z) && defined(USE_LZMA)
		if (level != shrink_fast && !standard) {
			unsigned lzma_algo;
			unsigned lzma_dictsize;
			unsigned lzma_fastbytes;

			switch (level) {
			case shrink_normal :
				lzma_algo = 1;
				lzma_dictsize = 1 << 20;
				lzma_fastbytes = 32;
				break;
			case shrink_extra :
				lzma_algo = 2;
				lzma_dictsize = 1 << 22;
				lzma_fastbytes = 64;
				break;
			case shrink_extreme :
				lzma_algo = 2;
				lzma_dictsize = 1 << 24;
				lzma_fastbytes = 64;
				break;
			default:
				assert(0);
			}

			// compress with lzma
			c1_data = data_alloc(uncompressed_size_get());
			c1_size = uncompressed_size_get();
			c1_ver = 20;
			c1_met = ZIP_METHOD_LZMA;
			c1_fla = 0;

			if (!compress_lzma_7z(uncompressed_data, uncompressed_size_get(), c1_data, c1_size, lzma_algo, lzma_dictsize, lzma_fastbytes)) {
				data_free(c1_data);
				c1_data = 0;
			}

			if (got(c0_data, c0_size, c0_met, c1_data, c1_size, c1_met, true, standard, level == shrink_none)) {
				data_free(c0_data);
				c0_data = c1_data;
				c0_size = c1_size;
				c0_ver = c1_ver;
				c0_met = c1_met;
				c0_fla = c1_fla;
				modify = true;
			} else {
				data_free(c1_data);
			}
		}
#endif

#if defined(USE_BZIP2)
		if (level != shrink_fast && !standard) {
			unsigned bzip2_level;
			unsigned bzip2_workfactor;

			switch (level) {
			case shrink_normal :
				bzip2_level = 6;
				bzip2_workfactor = 30;
				break;
			case shrink_extra :
				bzip2_level = 9;
				bzip2_workfactor = 60;
				break;
			case shrink_extreme :
				bzip2_level = 9;
				bzip2_workfactor = 120;
				break;
			default:
				assert(0);
			}

			// compress with bzip2
			c1_data = data_alloc(uncompressed_size_get());
			c1_size = uncompressed_size_get();
			c1_ver = 20;
			c1_met = ZIP_METHOD_BZIP2;
			c1_fla = 0;

			if (!compress_bzip2(uncompressed_data, uncompressed_size_get(), c1_data, c1_size, bzip2_level, bzip2_workfactor)) {
				data_free(c1_data);
				c1_data = 0;
			}

			if (got(c0_data, c0_size, c0_met, c1_data, c1_size, c1_met, true, standard, level == shrink_none)) {
				data_free(c0_data);
				c0_data = c1_data;
				c0_size = c1_size;
				c0_ver = c1_ver;
				c0_met = c1_met;
				c0_fla = c1_fla;
				modify = true;
			} else {
				data_free(c1_data);
			}
		}
#endif

#if defined(USE_7Z)
#if defined(USE_LZMA)
		// try only for small files or if standard compression is required
		// otherwise assume that lzma is better
		if (level != shrink_fast && (standard || uncompressed_size_get() <= (1 << 16))) {
#else
		if (level != shrink_fast) {
#endif
			unsigned sz_passes;
			unsigned sz_fastbytes;

			switch (level) {
			case shrink_normal :
				sz_passes = 1;
				sz_fastbytes = 64;
				break;
			case shrink_extra :
				sz_passes = 3;
				sz_fastbytes = 128;
				break;
			case shrink_extreme :
				sz_passes = 5;
				sz_fastbytes = 255;
				break;
			default:
				assert(0);
			}

			// compress with 7z
			c1_data = data_alloc(uncompressed_size_get());
			c1_size = uncompressed_size_get();
			c1_ver = 20;
			c1_met = ZIP_METHOD_DEFLATE;
			c1_fla = ZIP_GEN_FLAGS_DEFLATE_MAXIMUM;
			if (!compress_deflate_7z(uncompressed_data, uncompressed_size_get(), c1_data, c1_size, sz_passes, sz_fastbytes)) {
				data_free(c1_data);
				c1_data = 0;
			}

			if (got(c0_data, c0_size, c0_met, c1_data, c1_size, c1_met, true, standard, level == shrink_none)) {
				data_free(c0_data);
				c0_data = c1_data;
				c0_size = c1_size;
				c0_ver = c1_ver;
				c0_met = c1_met;
				c0_fla = c1_fla;
				modify = true;
			} else {
				data_free(c1_data);
			}
		}

		if (level == shrink_fast) {
			// compress with zlib Z_DEFAULT_COMPRESSION/Z_DEFAULT_STRATEGY/6
			c1_data = data_alloc(uncompressed_size_get());
			c1_size = uncompressed_size_get();
			c1_ver = 20;
			c1_met = ZIP_METHOD_DEFLATE;
			c1_fla = ZIP_GEN_FLAGS_DEFLATE_NORMAL;
			if (!compress_deflate_zlib(uncompressed_data, uncompressed_size_get(), c1_data, c1_size, Z_DEFAULT_COMPRESSION, Z_DEFAULT_STRATEGY, 6)) {
				data_free(c1_data);
				c1_data = 0;
			}

			if (got(c0_data, c0_size, c0_met, c1_data, c1_size, c1_met, true, standard, level == shrink_none)) {
				data_free(c0_data);
				c0_data = c1_data;
				c0_size = c1_size;
				c0_ver = c1_ver;
				c0_met = c1_met;
				c0_fla = c1_fla;
				modify = true;
			} else {
				data_free(c1_data);
			}
		}
#else
		unsigned libz_level = Z_BEST_COMPRESSION;

		if (1) {
			// compress with zlib Z_BEST_COMPRESSION/Z_DEFAULT_STRATEGY/MAX_MEM_LEVEL
			c1_data = data_alloc(uncompressed_size_get());
			c1_size = uncompressed_size_get();
			c1_ver = 20;
			c1_met = ZIP_METHOD_DEFLATE;
			c1_fla = ZIP_GEN_FLAGS_DEFLATE_MAXIMUM;
			if (!compress_deflate_zlib(uncompressed_data, uncompressed_size_get(), c1_data, c1_size, libz_level, Z_DEFAULT_STRATEGY, MAX_MEM_LEVEL)) {
				data_free(c1_data);
				c1_data = 0;
			}

			if (got(c0_data, c0_size, c0_met, c1_data, c1_size, c1_met, true, standard, level == shrink_none)) {
				data_free(c0_data);
				c0_data = c1_data;
				c0_size = c1_size;
				c0_ver = c1_ver;
				c0_met = c1_met;
				c0_fla = c1_fla;
				modify = true;
			} else {
				data_free(c1_data);
			}
		}

		if (1) {
			// compress with zlib Z_BEST_COMPRESSION/Z_DEFAULT_STRATEGY/6
			c1_data = data_alloc(uncompressed_size_get());
			c1_size = uncompressed_size_get();
			c1_ver = 20;
			c1_met = ZIP_METHOD_DEFLATE;
			c1_fla = ZIP_GEN_FLAGS_DEFLATE_MAXIMUM;
			if (!compress_deflate_zlib(uncompressed_data, uncompressed_size_get(), c1_data, c1_size, libz_level, Z_DEFAULT_STRATEGY, 6)) {
				data_free(c1_data);
				c1_data = 0;
			}

			if (got(c0_data, c0_size, c0_met, c1_data, c1_size, c1_met, true, standard, level == shrink_none)) {
				data_free(c0_data);
				c0_data = c1_data;
				c0_size = c1_size;
				c0_ver = c1_ver;
				c0_met = c1_met;
				c0_fla = c1_fla;
				modify = true;
			} else {
				data_free(c1_data);
			}
		}

		if (1) {
			// compress with zlib Z_BEST_COMPRESSION/Z_FILTERED/MAX_MEM_LEVEL
			c1_data = data_alloc(uncompressed_size_get());
			c1_size = uncompressed_size_get();
			c1_ver = 20;
			c1_met = ZIP_METHOD_DEFLATE;
			c1_fla = ZIP_GEN_FLAGS_DEFLATE_MAXIMUM;
			if (!compress_deflate_zlib(uncompressed_data, uncompressed_size_get(), c1_data, c1_size, libz_level, Z_FILTERED, MAX_MEM_LEVEL)) {
				data_free(c1_data);
				c1_data = 0;
			}

			if (got(c0_data, c0_size, c0_met, c1_data, c1_size, c1_met, true, standard, level == shrink_none)) {
				data_free(c0_data);
				c0_data = c1_data;
				c0_size = c1_size;
				c0_ver = c1_ver;
				c0_met = c1_met;
				c0_fla = c1_fla;
				modify = true;
			} else {
				data_free(c1_data);
			}
		}

		if (1) {
			// compress with zlib Z_BEST_COMPRESSION/Z_HUFFMAN_ONLY/MAX_MEM_LEVEL
			c1_data = data_alloc(uncompressed_size_get());
			c1_size = uncompressed_size_get();
			c1_ver = 20;
			c1_met = ZIP_METHOD_DEFLATE;
			c1_fla = ZIP_GEN_FLAGS_DEFLATE_MAXIMUM;
			if (!compress_deflate_zlib(uncompressed_data, uncompressed_size_get(), c1_data, c1_size, libz_level, Z_HUFFMAN_ONLY, MAX_MEM_LEVEL)) {
				data_free(c1_data);
				c1_data = 0;
			}

			if (got(c0_data, c0_size, c0_met, c1_data, c1_size, c1_met, true, standard, level == shrink_none)) {
				data_free(c0_data);
				c0_data = c1_data;
				c0_size = c1_size;
				c0_ver = c1_ver;
				c0_met = c1_met;
				c0_fla = c1_fla;
				modify = true;
			} else {
				data_free(c1_data);
			}
		}
#endif
	}

	// store
	if (got(c0_data, c0_size, c0_met, uncompressed_data, uncompressed_size_get(), ZIP_METHOD_STORE, true, standard, level == shrink_none)) {
		data_free(c0_data);
		c0_data = uncompressed_data;
		c0_size = uncompressed_size_get();
		c0_ver = 10;
		c0_met = ZIP_METHOD_STORE;
		c0_fla = 0;
		modify = true;
	} else {
		data_free(uncompressed_data);
	}

	// set the best option found
	data = c0_data;
	info.compressed_size = c0_size;
	info.version_needed_to_extract = c0_ver;
	info.compression_method = c0_met;
	info.general_purpose_bit_flag = c0_fla;

	return modify;
}

void zip::test() const
{
	assert(flag.read);

	for(const_iterator i = begin();i!=end();++i)
		i->test();
}

void zip::shrink(bool standard, shrink_t level)
{
	assert(flag.read);

	// remove unneeded data
	if (info.zipfile_comment_length != 0)
		flag.modify = true;
	data_free(zipfile_comment);
	zipfile_comment = 0;
	info.zipfile_comment_length = 0;

	for(iterator i=begin();i!=end();++i)
		if (i->shrink(standard, level))
			flag.modify = true;
}

