/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2003, 2004, 2005 Andrea Mazzoleni
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
#include "file.h"
#include "compress.h"
#include "siglock.h"

#include "lib/mng.h"
#include "lib/endianrw.h"

#include <iostream>
#include <iomanip>

using namespace std;

shrink_t opt_level;
bool opt_quiet;
bool opt_force;

enum ftype_t {
	ftype_png,
	ftype_mng,
	ftype_gz
};

#define BLOCK_SIZE 4096

struct block_t {
	unsigned char* data;
	block_t* next;
};

void copy_data(adv_fz* f_in, adv_fz* f_out, unsigned char* data, unsigned size)
{
	if (fzread(data, size, 1, f_in) != 1) {
		throw error() << "Error reading";
	}

	if (fzwrite(data, size, 1, f_out) != 1) {
		throw error() << "Error writing";
	}
}

void copy_data(adv_fz* f_in, adv_fz* f_out, unsigned size)
{
	while (size > 0) {
		unsigned char c;

		if (fzread(&c, 1, 1, f_in) != 1) {
			throw error() << "Error reading";
		}

		if (fzwrite(&c, 1, 1, f_out) != 1) {
			throw error() << "Error writing";
		}

		--size;
	}
}

void copy_zero(adv_fz* f_in, adv_fz* f_out)
{
	while (1) {
		unsigned char c;

		if (fzread(&c, 1, 1, f_in) != 1) {
			throw error() << "Error reading";
		}

		if (fzwrite(&c, 1, 1, f_out) != 1) {
			throw error() << "Error writing";
		}

		if (!c)
			break;
	}
}

void read_deflate(adv_fz* f_in, unsigned size, unsigned char*& res_data, unsigned& res_size)
{
	z_stream z;
	block_t* base;
	block_t* i;
	unsigned pos;
	int r;

	base = new block_t;
	base->data = data_alloc(BLOCK_SIZE);
	base->next = 0;
	i = base;

	memset(&z, 0, sizeof(z));
	z.zalloc = 0;
	z.zfree = 0;
	z.next_out = i->data;
	z.avail_out = BLOCK_SIZE;
	z.next_in = 0;
	z.avail_in = 0;

	/* !! ZLIB UNDOCUMENTED FEATURE !! (used in gzio.c module )
	 * windowBits is passed < 0 to tell that there is no zlib header.
	 * Note that in this case inflate *requires* an extra "dummy" byte
	 * after the compressed stream in order to complete decompression and
	 * return Z_STREAM_END.
	 */

	r = inflateInit2(&z, -15);

	unsigned char block[BLOCK_SIZE];
	unsigned char dummy;
	int dummy_used = 0;
	while (r == Z_OK && (size > 0 || z.avail_in != 0 || !dummy_used || z.avail_out == 0)) {
		if (z.avail_in == 0 && size > 0) {
			unsigned run = size;
			if (run > BLOCK_SIZE)
				run = BLOCK_SIZE;
			size -= run;
			if (fzread(block, run, 1, f_in) != 1)
				throw error() << "Error reading";
			z.next_in = block;
			z.avail_in = run;
		}

		/* The zlib code effectively READ the dummy byte,
		 * this imply that the pointer MUST point to a valid data region.
		 * The dummy byte is not always needed, only if inflate return Z_OK
		 * instead of Z_STREAM_END.
		 */
		if (z.avail_in == 0 && size == 0 && !dummy_used) {
			dummy = 0;
			z.next_in = &dummy;
			z.avail_in = 1;
			dummy_used = 1;
		}

		if (z.avail_out == 0) {
			block_t* next;
			next = new block_t;
			next->data = data_alloc(BLOCK_SIZE);
			next->next = 0;
			i->next = next;
			i = next;
			z.next_out = i->data;
			z.avail_out = BLOCK_SIZE;
		}

		r = inflate(&z, Z_NO_FLUSH);
	}

	if (r != Z_STREAM_END) {
		inflateEnd(&z);
		throw error() << "Invalid compressed data";
	}

	r = inflateEnd(&z);
	if (r != Z_OK) {
		throw error() << "Invalid compressed data";
	}

	res_size = z.total_out;
	res_data = data_alloc(res_size);

	pos = 0;
	i = base;
	while (i) {
		block_t* next;

		if (pos < res_size) {
			unsigned run = BLOCK_SIZE;
			if (run > res_size - pos)
				run = res_size - pos;
			memcpy(res_data + pos, i->data, run);
			pos += run;
		}

		data_free(i->data);
		next = i->next;
		delete i;
		i = next;
	}

	if (pos != res_size) {
		throw error() << "Internal error";
	}
}

void read_idat(adv_fz* f, unsigned char*& data, unsigned& size, unsigned& type, unsigned char*& res_data, unsigned& res_size)
{
	z_stream z;
	block_t* base;
	block_t* i;
	unsigned pos;
	int r;
	unsigned char* next_data;
	unsigned next_size;
	unsigned next_type;

	base = new block_t;
	base->data = data_alloc(BLOCK_SIZE);
	base->next = 0;
	i = base;

	memset(&z, 0, sizeof(z));
	z.zalloc = 0;
	z.zfree = 0;
	z.next_out = i->data;
	z.avail_out = BLOCK_SIZE;
	z.next_in = data;
	z.avail_in = size;

	if (adv_png_read_chunk(f, &next_data, &next_size, &next_type) != 0) {
		throw_png_error();
	}

	r = inflateInit(&z);

	while (r == Z_OK && (next_type == ADV_PNG_CN_IDAT || z.avail_in != 0 || z.avail_out == 0)) {
		if (z.avail_in == 0 && next_type == ADV_PNG_CN_IDAT) {
			free(data);

			data = next_data;
			size = next_size;
			z.next_in = data;
			z.avail_in = size;

			if (adv_png_read_chunk(f, &next_data, &next_size, &next_type) != 0) {
				inflateEnd(&z);
				throw_png_error();
			}
		}

		if (z.avail_out == 0) {
			block_t* next;
			next = new block_t;
			next->data = data_alloc(BLOCK_SIZE);
			next->next = 0;
			i->next = next;
			i = next;
			z.next_out = i->data;
			z.avail_out = BLOCK_SIZE;
		}

		r = inflate(&z, Z_NO_FLUSH);
	}

	if (r != Z_STREAM_END) {
		inflateEnd(&z);
		throw error() << "Invalid compressed data";
	}

	r = inflateEnd(&z);
	if (r != Z_OK) {
		throw error() << "Invalid compressed data";
	}

	free(data);

	data = next_data;
	size = next_size;
	type = next_type;

	res_size = z.total_out;
	res_data = data_alloc(res_size);

	pos = 0;
	i = base;
	while (i) {
		block_t* next;

		if (pos < res_size) {
			unsigned run = BLOCK_SIZE;
			if (run > res_size - pos)
				run = res_size - pos;
			memcpy(res_data + pos, i->data, run);
			pos += run;
		}

		data_free(i->data);
		next = i->next;
		delete i;
		i = next;
	}

	if (pos != res_size) {
		throw error() << "Internal error";
	}
}

void convert_dat(adv_fz* f_in, adv_fz* f_out, unsigned end)
{
	unsigned char* data;
	unsigned type;
	unsigned size;

	while (1) {
		if (adv_png_read_chunk(f_in, &data, &size, &type) != 0) {
			throw_png_error();
		}

		if (type == ADV_PNG_CN_IDAT) {
			unsigned char* res_data;
			unsigned res_size;

			read_idat(f_in, data, size, type, res_data, res_size);

			unsigned cmp_size = oversize_zlib(res_size);
			unsigned char* cmp_data = data_alloc(cmp_size);

			if (!compress_zlib(opt_level, cmp_data, cmp_size, res_data, res_size)) {
				throw error() << "Error compressing";
			}

			data_free(res_data);

			if (adv_png_write_chunk(f_out, ADV_PNG_CN_IDAT, cmp_data, cmp_size, 0) != 0) {
				throw_png_error();
			}

			data_free(cmp_data);

		}

		if (adv_png_write_chunk(f_out, type, data, size, 0) != 0) {
			throw_png_error();
		}

		free(data);

		if (type == end)
			break;
	}
}

void convert_png(adv_fz* f_in, adv_fz* f_out)
{
	if (adv_png_read_signature(f_in) != 0) {
		throw_png_error();
	}

	if (adv_png_write_signature(f_out, 0) != 0) {
		throw_png_error();
	}

	convert_dat(f_in, f_out, ADV_PNG_CN_IEND);
}

void convert_mng(adv_fz* f_in, adv_fz* f_out)
{
	if (adv_mng_read_signature(f_in) != 0) {
		throw_png_error();
	}

	if (adv_mng_write_signature(f_out, 0) != 0) {
		throw_png_error();
	}

	convert_dat(f_in, f_out, ADV_MNG_CN_MEND);
}

void convert_gz(adv_fz* f_in, adv_fz* f_out)
{
	unsigned char header[10];

	copy_data(f_in, f_out, header, 10);

	if (header[0] != 0x1f || header[1] != 0x8b) {
		throw error() << "Invalid GZ signature";
	}

	if (header[2] != 0x8 /* deflate */) {
		throw error_unsupported() << "Compression method not supported";
	}

	if ((header[3] & 0xE0) != 0) {
		throw error_unsupported() << "Unsupported flag";
	}

	if (header[3] & (1 << 2) /* FLG.FEXTRA */) {
		unsigned char extra_len[2];

		copy_data(f_in, f_out, extra_len, 2);

		copy_data(f_in, f_out, le_uint16_read(extra_len));
	}

	if (header[3] & (1 << 3) /* FLG.FNAME */) {
		copy_zero(f_in, f_out);
	}

	if (header[3] & (1 << 4) /* FLG.FCOMMENT */) {
		copy_zero(f_in, f_out);
	}

	if (header[3] & (1 << 1) /* FLG.FHCRC */) {
		copy_data(f_in, f_out, 2);
	}

	long size = fzsize(f_in);
	if (size < 0) {
		throw error() << "Error reading";
	}
	long pos = fztell(f_in);
	if (pos < 0) {
		throw error() << "Error reading";
	}
	size -= pos;
	if (size < 8) {
		throw error() << "Invalid file format";
	}
	size -= 8;

	unsigned char* res_data;
	unsigned res_size;
	read_deflate(f_in, size, res_data, res_size);

	unsigned cmp_size = oversize_deflate(res_size);
	unsigned char* cmp_data = data_alloc(cmp_size);

	unsigned crc = crc32(0, res_data, res_size);

	if (!compress_deflate(opt_level, cmp_data, cmp_size, res_data, res_size))
		throw error() << "Error compressing";

	data_free(res_data);

	if (fzwrite(cmp_data, cmp_size, 1, f_out) != 1)
		throw error() << "Error writing";

	data_free(cmp_data);

	unsigned char footer[8];

	copy_data(f_in, f_out, footer, 8);

	if (crc != le_uint32_read(footer))
		throw error() << "Invalid crc";

	if ((res_size & 0xFFFFFFFF) != le_uint32_read(footer + 4))
		throw error() << "Invalid size";
}

void convert_inplace(const string& path)
{
	adv_fz* f_in;
	adv_fz* f_out;

	// temp name of the saved file
	string path_dst = file_basepath(path) + ".tmp";

	f_in = fzopen(path.c_str(), "rb");
	if (!f_in) {
		throw error() << "Failed open for reading " << path;
	}

	try {
		// read the header
		unsigned char header[8];
		if (fzread(header, 8, 1, f_in) != 1)
			throw error() << "Error reading " << path;

		// detect the file type
		ftype_t ftype;
		if (header[0] == 0x1f && header[1] == 0x8b) {
			ftype = ftype_gz;
		} else if (header[0] == 0x89 && header[1] == 0x50 && header[2] == 0x4E && header[3] == 0x47) {
			ftype = ftype_png;
		} else if (header[0] == 0x8A && header[1] == 0x4D && header[2] == 0x4E && header[3] == 0x47) {
			ftype = ftype_mng;
		} else {
			throw error() << "File type not supported";
		}

		// restore the file position
		if (fzseek(f_in, 0, SEEK_SET) != 0) {
			throw error() << "Error seeking " << path;
		}

		f_out = fzopen(path_dst.c_str(), "wb");
		if (!f_out) {
			throw error() << "Failed open for writing " << path_dst;
		}

		try {
			switch (ftype) {
			case ftype_png :
				convert_png(f_in, f_out);
				break;
			case ftype_mng :
				convert_mng(f_in, f_out);
				break;
			case ftype_gz :
				convert_gz(f_in, f_out);
				break;
			}
		} catch (...) {
			fzclose(f_out);
			remove(path_dst.c_str());
			throw;
		}

	} catch (...) {
		fzclose(f_in);
		throw;
	}

	fzclose(f_in);
	fzclose(f_out);

	unsigned dst_size = file_size(path_dst);
	if (!opt_force && file_size(path) < dst_size) {
		// delete the new file
		remove(path_dst.c_str());
		throw error_unsupported() << "Bigger " << dst_size;
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

void rezip_single(const string& file, unsigned long long& total_0, unsigned long long& total_1)
{
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
		} catch (error_unsupported& e) {
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

void rezip_all(int argc, char* argv[])
{
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

#if HAVE_GETOPT_LONG
struct option long_options[] = {
	{"recompress", 0, 0, 'z'},
	{"list", 0, 0, 'l'},

	{"shrink-store", 0, 0, '0'},
	{"shrink-fast", 0, 0, '1'},
	{"shrink-normal", 0, 0, '2'},
	{"shrink-extra", 0, 0, '3'},
	{"shrink-insane", 0, 0, '4'},

	{"quiet", 0, 0, 'q'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'V'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "zl01234fqhV"

void version()
{
	cout << PACKAGE " v" VERSION " by Andrea Mazzoleni" << endl;
}

void usage()
{
	version();

	cout << "Usage: advpng [options] [FILES...]" << endl;
	cout << endl;
	cout << "Modes:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-z, --recompress    ", "-z") "  Recompress the specified files" << endl;
	cout << "Options:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-0, --shrink-store  ", "-0") "  Don't compress" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-1, --shrink-fast   ", "-1") "  Compress fast" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-2, --shrink-normal ", "-2") "  Compress normal" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-3, --shrink-extra  ", "-3") "  Compress extra" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-4, --shrink-insane ", "-4") "  Compress extreme" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-f, --force         ", "-f") "  Force the new file also if it's bigger" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-q, --quiet         ", "-q") "  Don't print on the console" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-h, --help          ", "-h") "  Help of the program" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-V, --version       ", "-V") "  Version of the program" << endl;
}

void process(int argc, char* argv[])
{
	enum cmd_t {
		cmd_unset, cmd_recompress
	} cmd = cmd_unset;

	opt_quiet = false;
	opt_level = shrink_normal;
	opt_force = false;

	if (argc <= 1) {
		usage();
		return;
	}

	int c = 0;

	opterr = 0; // don't print errors

	while ((c =
#if HAVE_GETOPT_LONG
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
			throw error() << "Unknown option `" << opt << "'";
			}
		}
	}

	switch (cmd) {
	case cmd_recompress :
		rezip_all(argc - optind, argv + optind);
		break;
	case cmd_unset :
		throw error() << "No command specified";
	}
}

int main(int argc, char* argv[])
{
	try {
		process(argc, argv);
	} catch (error& e) {
		cerr << e << endl;
		exit(EXIT_FAILURE);
	} catch (std::bad_alloc) {
		cerr << "Low memory" << endl;
		exit(EXIT_FAILURE);
	} catch (...) {
		cerr << "Unknown error" << endl;
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

