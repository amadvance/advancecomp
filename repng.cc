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

#include "pngex.h"
#include "utility.h"
#include "compress.h"
#include "siglock.h"

#include "lib/endianrw.h"

#include <iostream>
#include <iomanip>

using namespace std;

// --------------------------------------------------------------------------
// Static

shrink_t opt_level;
bool opt_quiet;
bool opt_force;
bool opt_crc;

// --------------------------------------------------------------------------
// Conversion

bool reduce_image(unsigned char** out_ptr, unsigned* out_scanline, unsigned char* ovr_ptr, unsigned* ovr_count, unsigned width, unsigned height, unsigned char* img_ptr, unsigned img_scanline)
{
	unsigned char col_ptr[256*3];
	unsigned col_count;
	unsigned i,j,k;
	unsigned char* new_ptr;
	unsigned new_scanline;

	new_scanline = width;
	new_ptr = data_alloc(height * new_scanline);
	col_count = 0;
	for(i=0;i<height;++i) {
		unsigned char* p0 = img_ptr + i * img_scanline;
		unsigned char* p1 = new_ptr + i * new_scanline;
		for(j=0;j<width;++j) {
			for(k=0;k<col_count;++k) {
				if (col_ptr[k*3] == p0[0] && col_ptr[k*3+1] == p0[1] && col_ptr[k*3+2] == p0[2])
					break;
			}
			if (k == col_count) {
				if (col_count == 256) {
					data_free(new_ptr);
					return false; /* too many colors */
				}
				col_ptr[col_count*3] = p0[0];
				col_ptr[col_count*3+1] = p0[1];
				col_ptr[col_count*3+2] = p0[2];
				++col_count;
			}
			*p1 = k;
			++p1;
			p0 += 3;
		}
	}

	memcpy(ovr_ptr, col_ptr, col_count * 3);
	*ovr_count = col_count;
	*out_ptr = new_ptr;
	*out_scanline = new_scanline;

	return true;
}

void write_image(adv_fz* f, unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline, unsigned char* pal_ptr, unsigned pal_size, unsigned char* rns_ptr, unsigned rns_size)
{
	if (pix_pixel == 1) {
		png_write(f, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, rns_ptr, rns_size, opt_level);
	} else {
		unsigned char ovr_ptr[256*3];
		unsigned ovr_count;
		unsigned char* new_ptr;
		unsigned new_scanline;

		new_ptr = 0;

		try {
			if (rns_size == 0
				&& reduce_image(&new_ptr, &new_scanline, ovr_ptr, &ovr_count, pix_width, pix_height, pix_ptr, pix_scanline)) {
				png_write(f, pix_width, pix_height, 1, new_ptr, new_scanline, ovr_ptr, ovr_count * 3, 0, 0, opt_level);
			} else {
				png_write(f, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, 0, 0, rns_ptr, rns_size, opt_level);
			}
		} catch (...) {
			data_free(new_ptr);
			throw;
		}

		data_free(new_ptr);
	}
}

void convert_f(adv_fz* f_in, adv_fz* f_out)
{
	unsigned char* dat_ptr;
	unsigned dat_size;
	unsigned pix_pixel;
	unsigned pix_width;
	unsigned pix_height;
	unsigned char* pal_ptr;
	unsigned pal_size;
	unsigned char* rns_ptr;
	unsigned rns_size;
	unsigned char* pix_ptr;
	unsigned pix_scanline;

	if (png_read_rns(
		&pix_width, &pix_height, &pix_pixel,
		&dat_ptr, &dat_size,
		&pix_ptr, &pix_scanline,
		&pal_ptr, &pal_size,
		&rns_ptr, &rns_size,
		f_in
	) != 0) {
		throw error_png();
	}

	try {
		write_image(
			f_out,
			pix_width, pix_height, pix_pixel,
			pix_ptr, pix_scanline,
			pal_ptr, pal_size,
			rns_ptr, rns_size
		);
	} catch (...) {
		free(dat_ptr);
		free(pal_ptr);
		free(rns_ptr);
		throw;
	}

	free(dat_ptr);
	free(pal_ptr);
	free(rns_ptr);
}

void convert_inplace(const string& path)
{
	adv_fz* f_in;
	adv_fz* f_out;

	// temp name of the saved file
	string path_dst = file_basepath(path) + ".tmp";

	f_in = fzopen(path.c_str(),"rb");
	if (!f_in) {
		throw error() << "Failed open for reading " << path;
	}

	f_out = fzopen(path_dst.c_str(),"wb");
	if (!f_out) {
		fzclose(f_in);
		throw error() << "Failed open for writing " << path_dst;
	}

	try {
		convert_f(f_in,f_out);
	} catch (...) {
		fzclose(f_in);
		fzclose(f_out);
		remove(path_dst.c_str());
		throw;
	}

	fzclose(f_in);
	fzclose(f_out);

	unsigned dst_size = file_size(path_dst);
	if (!opt_force && file_size(path) < dst_size) {
		// delete the new file
		remove(path_dst.c_str());
		throw error_ignore() << "Bigger " << dst_size;
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

// --------------------------------------------------------------------------
// List

void png_print(const string& path)
{
	unsigned type;
	unsigned size;
	adv_fz* f_in;

	f_in = fzopen(path.c_str(),"rb");
	if (!f_in) {
		throw error() << "Failed open for reading " << path;
	}

	try {
		if (png_read_signature(f_in) != 0) {
			throw error_png();
		}

		do {
			unsigned char* data;

			if (png_read_chunk(f_in, &data, &size, &type) != 0) {
				throw error_png();
			}

			if (opt_crc) {
				cout << hex << setw(8) << setfill('0') << crc32(0, data, size);
				cout << " ";
				cout << dec << setw(0) << setfill(' ') << size;
				cout << "\n";
			} else {
				png_print_chunk(type, data, size);
			}

			free(data);

		} while (type != PNG_CN_IEND);
	} catch (...) {
		fzclose(f_in);
		throw;
	}

	fzclose(f_in);
}

// --------------------------------------------------------------------------
// Command interface

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
		} catch (error& e) {
			if (!e.ignore_get())
				throw;
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

void list_all(int argc, char* argv[])
{
	for(int i=0;i<argc;++i) {
		if (argc > 1 && !opt_crc)
			cout << "File: " << argv[i] << endl;
		png_print(argv[i]);
	}
}

#if HAVE_GETOPT_LONG
struct option long_options[] = {
	{"recompress", 0, 0, 'z'},
	{"list", 0, 0, 'l'},
	{"list-crc", 0, 0, 'L'},

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

#define OPTIONS "zlL01234fqhV"

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
	cout << "  " SWITCH_GETOPT_LONG("-l, --list          ", "-l") "  List the content of the files" << endl;
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
		cmd_unset, cmd_recompress, cmd_list
	} cmd = cmd_unset;

	opt_quiet = false;
	opt_level = shrink_normal;
	opt_force = false;
	opt_crc = false;

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
			case 'l' :
				if (cmd != cmd_unset)
					throw error() << "Too many commands";
				cmd = cmd_list;
				break;
			case 'L' :
				if (cmd != cmd_unset)
					throw error() << "Too many commands";
				cmd = cmd_list;
				opt_crc = true;
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
	case cmd_list :
		list_all(argc - optind, argv + optind);
		break;
	case cmd_unset :
		throw error() << "No command specified";
	}
}

int main(int argc, char* argv[])
{
	try {
		process(argc,argv);
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

