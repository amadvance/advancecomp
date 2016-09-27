/*
 * This file is part of the Advance project.
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

#include "compress.h"
#include "data.h"

bool decompress_deflate_zlib(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned out_size)
{
	z_stream stream;
	int r;

	stream.next_in = const_cast<unsigned char*>(in_data);
	stream.avail_in = in_size;
	stream.next_out = out_data;
	stream.avail_out = out_size;
	stream.zalloc = (alloc_func)Z_NULL;
	stream.zfree = (free_func)Z_NULL;
	stream.opaque = Z_NULL;

	/* !! ZLIB UNDOCUMENTED FEATURE !! (used in gzio.c module )
	 * windowBits is passed < 0 to tell that there is no zlib header.
	 * Note that in this case inflate *requires* an extra "dummy" byte
	 * after the compressed stream in order to complete decompression and
	 * return Z_STREAM_END.
	 */
	r = inflateInit2(&stream, -15);
	if (r != Z_OK) {
		return false;
	}

	r = inflate(&stream, Z_SYNC_FLUSH);

	/* The zlib code effectively READ the dummy byte,
	 * this imply that the pointer MUST point to a valid data region.
	 * The dummy byte is not always needed, only if inflate return Z_OK
	 * instead of Z_STREAM_END.
	 */
	if (r == Z_OK) {
		/* dummy byte */
		unsigned char dummy = 0;
		stream.next_in = &dummy;
		stream.avail_in = 1;

		r = inflate(&stream, Z_SYNC_FLUSH);
	}

	if (r != Z_STREAM_END) {
		inflateEnd(&stream);
		return false;
	}

	r = inflateEnd(&stream);
	if (r != Z_OK) {
		return false;
	}

	if (stream.total_in != in_size || stream.total_out != out_size) {
		return false;
	}

	return true;
}

bool compress_deflate_zlib(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int compression_level, int strategy, int mem_level)
{
	z_stream stream;

	stream.next_in = const_cast<unsigned char*>(in_data);
	stream.avail_in = in_size;
	stream.next_out = out_data;
	stream.avail_out = out_size;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	int compression_window;
	unsigned required_window = in_size;

	// don't use the 8 bit window due a bug in the zlib 1.1.3 and previous
	if (required_window <= 512) compression_window = 9;
	else if (required_window <= 1024) compression_window = 10;
	else if (required_window <= 2048) compression_window = 11;
	else if (required_window <= 4096) compression_window = 12;
	else if (required_window <= 8192) compression_window = 13;
	else if (required_window <= 16384) compression_window = 14;
	else compression_window = 15;

	if (compression_window > MAX_WBITS)
		compression_window = MAX_WBITS;

	/* windowBits is passed < 0 to suppress the zlib header */
	if (deflateInit2(&stream, compression_level, Z_DEFLATED, -compression_window, mem_level, strategy) != Z_OK) {
		return false;
	}

	if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
		deflateEnd(&stream);
		return false;
	}

	out_size = stream.total_out;

	deflateEnd(&stream);

	return true;
}

bool decompress_rfc1950_zlib(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned out_size)
{
	unsigned long size = out_size;

	if (uncompress(out_data, &size, in_data, in_size) != Z_OK)
		return false;

	if (size != out_size)
		return false;

	return true;
}

bool compress_rfc1950_zlib(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int compression_level, int strategy, int mem_level)
{
	z_stream stream;

	stream.next_in = const_cast<unsigned char*>(in_data);
	stream.avail_in = in_size;
	stream.next_out = out_data;
	stream.avail_out = out_size;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	int compression_window;
	unsigned required_window = in_size;

	// don't use the 8 bit window due a bug in the zlib 1.1.3 and previous
	if (required_window <= 512) compression_window = 9;
	else if (required_window <= 1024) compression_window = 10;
	else if (required_window <= 2048) compression_window = 11;
	else if (required_window <= 4096) compression_window = 12;
	else if (required_window <= 8192) compression_window = 13;
	else if (required_window <= 16384) compression_window = 14;
	else compression_window = 15;

	if (compression_window > MAX_WBITS)
		compression_window = MAX_WBITS;

	if (deflateInit2(&stream, compression_level, Z_DEFLATED, compression_window, mem_level, strategy) != Z_OK) {
		return false;
	}

	if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
		deflateEnd(&stream);
		return false;
	}

	out_size = stream.total_out;

	deflateEnd(&stream);

	return true;
}

bool compress_deflate_libdeflate(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int compression_level)
{
	struct libdeflate_compressor* compressor;

	compressor = libdeflate_alloc_compressor(compression_level);

	out_size = libdeflate_deflate_compress(compressor, in_data, in_size, out_data, out_size);

	libdeflate_free_compressor(compressor);

	if (!out_size)
		return false;

	return true;
}

bool compress_rfc1950_libdeflate(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int compression_level)
{
	struct libdeflate_compressor* compressor;

	compressor = libdeflate_alloc_compressor(compression_level);

	out_size = libdeflate_zlib_compress(compressor, in_data, in_size, out_data, out_size);

	libdeflate_free_compressor(compressor);

	if (!out_size)
		return false;

	return true;
}

#if USE_BZIP2
bool compress_bzip2(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int blocksize, int workfactor)
{
	return BZ2_bzBuffToBuffCompress((char*)out_data, &out_size, (char*)(in_data), in_size, blocksize, 0, workfactor) == BZ_OK;
}

bool decompress_bzip2(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned out_size)
{
	unsigned size = out_size;

	if (BZ2_bzBuffToBuffDecompress((char*)out_data, &size, (char*)in_data, in_size, 0, 0)!=BZ_OK)
		return false;

	if (size != out_size)
		return false;

	return true;
}
#endif

bool compress_zlib(shrink_t level, unsigned char* out_data, unsigned& out_size, const unsigned char* in_data, unsigned in_size)
{
	if (level.level == shrink_insane) {
		ZopfliOptions opt_zopfli;
		unsigned char* data;
		size_t size;

		ZopfliInitOptions(&opt_zopfli);
		opt_zopfli.numiterations = level.iter > 5 ? level.iter : 5;

		size = 0;
		data = 0;

		ZopfliCompress(&opt_zopfli, ZOPFLI_FORMAT_ZLIB, in_data, in_size, &data, &size);

		if (size < out_size) {
			memcpy(out_data, data, size);
			out_size = static_cast<unsigned>(size);
		}

		free(data);
	}

	if (level.level == shrink_extra) {
		unsigned sz_passes;
		unsigned sz_fastbytes;
		unsigned char* data;
		unsigned size;

		switch (level.level) {
		case shrink_extra :
			sz_passes = level.iter > 15 ? level.iter : 15;
			if (sz_passes > 255)
				sz_passes = 255;
			sz_fastbytes = 255;
			break;
		default:
			assert(0);
		}

		size = out_size;
		data = data_alloc(size);

		if (compress_rfc1950_7z(in_data, in_size, data, size, sz_passes, sz_fastbytes)) {
			memcpy(out_data, data, size);
			out_size = size;
		}

		data_free(data);

		return true;
	}

	if (level.level == shrink_normal || level.level == shrink_extra || level.level == shrink_insane) {
		int compression_level;
		unsigned char* data;
		unsigned size;

		switch (level.level) {
		case shrink_normal :
			compression_level = 12;
			break;
		case shrink_extra :
			// assume that 7z is better, but does a fast try to cover some corner cases
			compression_level = 12;
			break;
		case shrink_insane :
			// assume that zopfli is better, but does a fast try to cover some corner cases
			compression_level = 12;
			break;
		default:
			assert(0);
		}

		size = out_size;
		data = data_alloc(size);

		if (compress_rfc1950_libdeflate(in_data, in_size, data, size, compression_level)) {
			memcpy(out_data, data, size);
			out_size = size;
		}

		data_free(data);

		return true;
	}

	if (level.level == shrink_none || level.level == shrink_fast) {
		int libz_level;
		unsigned char* data;
		unsigned size;

		switch (level.level) {
		case shrink_none :
			libz_level = Z_NO_COMPRESSION;
			break;
		default:
			libz_level = Z_BEST_COMPRESSION;
			break;
		}

		size = out_size;
		data = data_alloc(size);

		if (compress_rfc1950_zlib(in_data, in_size, data, size, libz_level, Z_DEFAULT_STRATEGY, MAX_MEM_LEVEL)) {
			memcpy(out_data, data, size);
			out_size = size;
		}

		data_free(data);
	}

	return true;
}

bool compress_deflate(shrink_t level, unsigned char* out_data, unsigned& out_size, const unsigned char* in_data, unsigned in_size)
{
	if (level.level == shrink_insane) {
		ZopfliOptions opt_zopfli;
		unsigned char* data;
		size_t size;
		
		ZopfliInitOptions(&opt_zopfli);
		opt_zopfli.numiterations = level.iter > 5 ? level.iter : 5;

		size = 0;
		data = 0;

		ZopfliCompress(&opt_zopfli, ZOPFLI_FORMAT_DEFLATE, in_data, in_size, &data, &size);

		if (size < out_size) {
			memcpy(out_data, data, size);
			out_size = static_cast<unsigned>(size);
		}

		free(data);
	}

	// note that in some case, 7z is better than zopfli
	if (level.level == shrink_normal || level.level == shrink_extra || level.level == shrink_insane) {
		int compression_level;
		unsigned char* data;
		unsigned size;

		switch (level.level) {
		case shrink_normal :
			compression_level = 6;
			break;
		case shrink_extra :
			compression_level = 12;
			break;
		case shrink_insane :
			// assume that zopfli is better, but does a fast try to cover some corner cases
			compression_level = 12;
			break;
		default:
			assert(0);
		}

		size = out_size;
		data = data_alloc(size);

		if (compress_deflate_libdeflate(in_data, in_size, data, size, compression_level)) {
			memcpy(out_data, data, size);
			out_size = size;
		}

		data_free(data);
	}

	if (level.level == shrink_none || level.level == shrink_fast) {
		int libz_level;
		unsigned char* data;
		unsigned size;

		switch (level.level) {
		case shrink_none :
			libz_level = Z_NO_COMPRESSION;
			break;
		default:
			libz_level = Z_BEST_COMPRESSION;
			break;
		}

		size = out_size;
		data = data_alloc(size);

		if (compress_deflate_zlib(in_data, in_size, data, size, libz_level, Z_DEFAULT_STRATEGY, MAX_MEM_LEVEL)) {
			memcpy(out_data, data, size);
			out_size = size;
		}

		data_free(data);
	}

	return true;
}

unsigned oversize_deflate(unsigned size)
{
	return size + size / 10 + 12;
}

unsigned oversize_zlib(unsigned size)
{
	return oversize_deflate(size) + 10;
}

