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

#if USE_BZIP2
bool compress_bzip2(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned& out_size, int blocksize, int workfactor)
{
	return BZ2_bzBuffToBuffCompress(out_data, &out_size, const_cast<unsigned char*>(in_data), in_size, blocksize, 0, workfactor) == BZ_OK;
}

bool decompress_bzip2(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned out_size)
{
	unsigned size = out_size;

	if (BZ2_bzBuffToBuffDecompress(out_data, &size, const_cast<unsigned char*>(in_data), in_size, 0, 0)!=BZ_OK)
		return false;

	if (size != out_size)
		return false;

	return true;
}
#endif

bool compress_zlib(shrink_t level, unsigned char* out_data, unsigned& out_size, const unsigned char* in_data, unsigned in_size)
{
#if defined(USE_7Z)
	if (level == shrink_normal || level == shrink_extra || level == shrink_extreme) {
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

		if (!compress_rfc1950_7z(in_data, in_size, out_data, out_size, sz_passes, sz_fastbytes)) {
			return false;
		}

		return true;
	}
#endif

	int libz_level = Z_BEST_COMPRESSION;

	switch (level) {
	case shrink_none :
		libz_level = Z_NO_COMPRESSION;
		break;
	case shrink_fast :
		libz_level = Z_DEFAULT_COMPRESSION;
		break;
	case shrink_normal :
		libz_level = Z_DEFAULT_COMPRESSION;
		break;
	case shrink_extra :
		libz_level = Z_BEST_COMPRESSION;
		break;
	case shrink_extreme :
		libz_level = Z_BEST_COMPRESSION;
		break;
	}

	if (!compress_rfc1950_zlib(in_data, in_size, out_data, out_size, libz_level, Z_DEFAULT_STRATEGY, MAX_MEM_LEVEL)) {
		return false;
	}

	return true;
}

bool compress_deflate(shrink_t level, unsigned char* out_data, unsigned& out_size, const unsigned char* in_data, unsigned in_size)
{
#if defined(USE_7Z)
	if (level == shrink_normal || level == shrink_extra || level == shrink_extreme) {
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

		if (!compress_deflate_7z(in_data, in_size, out_data, out_size, sz_passes, sz_fastbytes)) {
			return false;
		}

		return true;
	}
#endif

	int libz_level = Z_BEST_COMPRESSION;

	switch (level) {
	case shrink_none :
		libz_level = Z_NO_COMPRESSION;
		break;
	case shrink_fast :
		libz_level = Z_DEFAULT_COMPRESSION;
		break;
	case shrink_normal :
		libz_level = Z_DEFAULT_COMPRESSION;
		break;
	case shrink_extra :
		libz_level = Z_BEST_COMPRESSION;
		break;
	case shrink_extreme :
		libz_level = Z_BEST_COMPRESSION;
		break;
	}

	if (!compress_deflate_zlib(in_data, in_size, out_data, out_size, libz_level, Z_DEFAULT_STRATEGY, MAX_MEM_LEVEL)) {
		return false;
	}

	return true;
}

unsigned oversize_deflate(unsigned size)
{
	return size * 11 / 10 + 12;
}

unsigned oversize_zlib(unsigned size)
{
	return oversize_deflate(size) + 10;
}

