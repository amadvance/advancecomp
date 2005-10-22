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
#include "mngex.h"
#include "except.h"
#include "file.h"
#include "compress.h"
#include "siglock.h"
#include "scroll.h"

#include "lib/endianrw.h"
#include "lib/mng.h"

#include <zlib.h>

#include <iostream>
#include <iomanip>

using namespace std;

int opt_dx;
int opt_dy;
int opt_limit;
bool opt_reduce;
bool opt_expand;
bool opt_noalpha;
shrink_t opt_level;
bool opt_quiet;
bool opt_verbose;
bool opt_scroll;
adv_mng_type opt_type;
bool opt_force;
bool opt_crc;

void clear_line()
{
	cout << "                                                              \r";
}

adv_scroll_info* analyze_f_mng(adv_fz* f)
{
	adv_mng* mng;
	unsigned counter;
	adv_scroll* scroll;
	int dx = 0;
	int dy = 0;

	mng = adv_mng_init(f);
	if (!mng) {
		throw error() << "Error in the mng stream";
	}

	scroll = scroll_init(opt_dx, opt_dy, opt_limit);

	counter = 0;

	try {
		while (1) {
			unsigned pix_width;
			unsigned pix_height;
			unsigned char* pix_ptr;
			unsigned pix_pixel;
			unsigned pix_scanline;
			unsigned char* dat_ptr_ext;
			unsigned dat_size;
			unsigned char* pal_ptr_ext;
			unsigned pal_size;
			unsigned tick;
			int r;

			r = adv_mng_read(mng, &pix_width, &pix_height, &pix_pixel, &dat_ptr_ext, &dat_size, &pix_ptr, &pix_scanline, &pal_ptr_ext, &pal_size, &tick, f);
			if (r < 0) {
				throw_png_error();
			}
			if (r > 0)
				break;

			data_ptr dat_ptr(dat_ptr_ext);
			data_ptr pal_ptr(pal_ptr_ext);

			scroll_analyze(scroll, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline);

			++counter;
			if (opt_verbose) {
				int x, y;
				scroll_last_get(scroll, &x, &y);
				if (dx < abs(x))
					dx = abs(x);
				if (dy < abs(y))
					dy = abs(y);

				cout << "Scroll frame " << counter << ", range " << dx << "x" << dy << "   \r";
				cout.flush();
			}
		}
	} catch (...) {
		adv_mng_done(mng);
		scroll_done(scroll);
		if (opt_verbose) {
			cout << endl;
		}
		throw;
	}

	adv_mng_done(mng);
	if (opt_verbose) {
		clear_line();
	}

	adv_scroll_info* info = scroll_info_init(scroll);

	scroll_done(scroll);

	return info;
}

adv_scroll_info* analyze_mng(const string& path)
{
	adv_fz* f;
	adv_scroll_info* info;

	f = fzopen(path.c_str(), "rb");
	if (!f) {
		throw error() << "Failed open for reading " << path;
	}

	try {
		info = analyze_f_mng(f);
	} catch (...) {
		fzclose(f);
		throw;
	}

	fzclose(f);

	return info;
}

adv_scroll_info* analyze_png(int argc, char* argv[])
{
	unsigned counter;
	adv_scroll* scroll;
	int dx = 0;
	int dy = 0;

	scroll = scroll_init(opt_dx, opt_dy, opt_limit);

	counter = 0;

	try {
		for(int i=0;i<argc;++i) {
			adv_fz* f_in;
			string path_src = argv[i];
			f_in = fzopen(path_src.c_str(), "rb");

			if (!f_in) {
				throw error() << "Failed open for reading " << path_src;
			}
			
			try {
				unsigned char* dat_ptr_ext;
				unsigned dat_size;
				unsigned pix_pixel;
				unsigned pix_width;
				unsigned pix_height;
				unsigned char* pal_ptr_ext;
				unsigned pal_size;
				unsigned char* pix_ptr;
				unsigned pix_scanline;

				if (adv_png_read(
					&pix_width, &pix_height, &pix_pixel,
					&dat_ptr_ext, &dat_size,
					&pix_ptr, &pix_scanline,
					&pal_ptr_ext, &pal_size,
					f_in
				) != 0) {
					throw_png_error();
				}

				data_ptr dat_ptr(dat_ptr_ext);
				data_ptr pal_ptr(pal_ptr_ext);

				scroll_analyze(scroll, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline);

				++counter;
				if (opt_verbose) {
					int x, y;
					scroll_last_get(scroll, &x, &y);
					if (dx < abs(x))
						dx = abs(x);
					if (dy < abs(y))
						dy = abs(y);

					cout << "Scroll frame " << counter << ", range " << dx << "x" << dy << "   \r";
					cout.flush();
				}

				fzclose(f_in);
			} catch (...) {
				fzclose(f_in);
				throw;
			}
		}
	} catch (...) {
		scroll_done(scroll);
		if (opt_verbose) {
			cout << endl;
		}
		throw;
	}

	if (opt_verbose) {
		clear_line();
	}

	adv_scroll_info* info = scroll_info_init(scroll);

	scroll_done(scroll);

	return info;
}

bool is_reducible_image(unsigned img_width, unsigned img_height, unsigned img_pixel, unsigned char* img_ptr, unsigned img_scanline)
{
	unsigned char col_ptr[256*3];
	unsigned col_count;
	unsigned i, j, k;

	// if an alpha channel is present th eimage cannot be palettized
	if (img_pixel != 3 && !opt_noalpha)
		return false;

	col_count = 0;
	for(i=0;i<img_height;++i) {
		unsigned char* p0 = img_ptr + i * img_scanline;
		for(j=0;j<img_width;++j) {
			for(k=0;k<col_count;++k) {
				if (col_ptr[k*3] == p0[0] && col_ptr[k*3+1] == p0[1] && col_ptr[k*3+2] == p0[2])
					break;
			}
			if (k == col_count) {
				if (col_count == 256)
					return false; /* too many colors */
				col_ptr[col_count*3] = p0[0];
				col_ptr[col_count*3+1] = p0[1];
				col_ptr[col_count*3+2] = p0[2];
				++col_count;
			}
			p0 += img_pixel;
		}
	}

	return true;
}

bool is_reducible_mng(const string& path)
{
	bool reducible;
	adv_fz* f;

	f = fzopen(path.c_str(), "rb");
	if (!f) {
		throw error() << "Failed open for reading " << path;
	}

	try {
		reducible = true;
		adv_mng* mng;

		mng = adv_mng_init(f);
		if (!mng) {
			throw error() << "Error in the mng stream";
		}

		if (opt_verbose) {
			cout << "Checking if the image is reducible\r";
			cout.flush();
		}

		try {
			while (reducible) {
				unsigned pix_width;
				unsigned pix_height;
				unsigned char* pix_ptr;
				unsigned pix_pixel;
				unsigned pix_scanline;
				unsigned char* dat_ptr_ext;
				unsigned dat_size;
				unsigned char* pal_ptr_ext;
				unsigned pal_size;
				unsigned tick;
				int r;

				r = adv_mng_read(mng, &pix_width, &pix_height, &pix_pixel, &dat_ptr_ext, &dat_size, &pix_ptr, &pix_scanline, &pal_ptr_ext, &pal_size, &tick, f);
				if (r < 0) {
					throw_png_error();
				}
				if (r > 0)
					break;

				data_ptr dat_ptr(dat_ptr_ext);
				data_ptr pal_ptr(pal_ptr_ext);

				if (!is_reducible_image(pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline))
					reducible = false;
			}
		} catch (...) {
			adv_mng_done(mng);
			if (opt_verbose) {
				cout << endl;
			}
			throw;
		}

		adv_mng_done(mng);
		if (opt_verbose) {
			clear_line();
		}

	} catch (...) {
		fzclose(f);
		throw;
	}

	fzclose(f);

	return reducible;
}

bool is_reducible_png(int argc, char* argv[])
{
	bool reducible;

	reducible = true;

	for(int i=1;i<argc && reducible;++i) {
		adv_fz* f_in;
		string path_src = argv[i];
		f_in = fzopen(path_src.c_str(), "rb");

		if (!f_in) {
			throw error() << "Failed open for reading " << path_src;
		}
			
		try {
			unsigned char* dat_ptr_ext;
			unsigned dat_size;
			unsigned pix_pixel;
			unsigned pix_width;
			unsigned pix_height;
			unsigned char* pal_ptr_ext;
			unsigned pal_size;
			unsigned char* pix_ptr;
			unsigned pix_scanline;

			if (adv_png_read(
				&pix_width, &pix_height, &pix_pixel,
				&dat_ptr_ext, &dat_size,
				&pix_ptr, &pix_scanline,
				&pal_ptr_ext, &pal_size,
				f_in
			) != 0) {
				throw_png_error();
			}

			data_ptr dat_ptr(dat_ptr_ext);
			data_ptr pal_ptr(pal_ptr_ext);

			if (!is_reducible_image(pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline))
				reducible = false;

			fzclose(f_in);
		} catch (...) {
			fzclose(f_in);
			throw;
		}
	}

	return reducible;
}

void convert_header(adv_mng_write* mng, adv_fz* f, unsigned* fc, unsigned frame_width, unsigned frame_height, unsigned frame_frequency, adv_scroll_info* info, bool alpha)
{
	if (info) {
		mng_write_header(mng, f, fc, frame_width, frame_height, frame_frequency, info->x, info->y, info->width, info->height, alpha);
	} else {
		mng_write_header(mng, f, fc, frame_width, frame_height, frame_frequency, 0, 0, 0, 0, alpha);
	}
}

void convert_image(adv_mng_write* mng, adv_fz* f_out, unsigned* fc, unsigned pix_width, unsigned pix_height, unsigned pix_pixel, unsigned char* pix_ptr, unsigned pix_scanline, unsigned char* pal_ptr, unsigned pal_size, adv_scroll_coord* scc)
{
	if (opt_noalpha && pix_pixel == 4) {
		/* convert to 3 bytes per pixel */
		unsigned dst_pixel = 3;
		unsigned dst_scanline = 3 * pix_width;
		data_ptr dst_ptr;

		dst_ptr = data_alloc(dst_scanline * pix_height);

		unsigned i, j;
		for(i=0;i<pix_height;++i) {
			const unsigned char* p0 = pix_ptr + i * pix_scanline;
			unsigned char* p1 = dst_ptr + i * dst_scanline;
			for(j=0;j<pix_width;++j) {
				p1[0] = p0[0];
				p1[1] = p0[1];
				p1[2] = p0[2];
				p0 += 4;
				p1 += 3;
			}
		}

		convert_image(mng, f_out, fc, pix_width, pix_height, dst_pixel, dst_ptr, dst_scanline, 0, 0, scc);
	} else {
		if (scc) {
			mng_write_image(mng, f_out, fc, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, scc->x, scc->y);
		} else {
			mng_write_image(mng, f_out, fc, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, 0, 0);
		}
	}
}

void convert_f_mng(adv_fz* f_in, adv_fz* f_out, unsigned* filec, unsigned* framec, adv_scroll_info* info, bool reduce, bool expand)
{
	unsigned counter;
	adv_mng* mng;
	adv_mng_write* mng_write;
	bool first = true;

	mng = adv_mng_init(f_in);
	if (!mng) {
		throw error() << "Error in the mng stream";
	}

	mng_write = mng_write_init(opt_type, opt_level, reduce, expand);
	if (!mng_write) {
		throw error() << "Error in the mng stream";
	}

	*filec = 0;
	counter = 0;

	try {
		while (1) {
			unsigned pix_width;
			unsigned pix_height;
			unsigned char* pix_ptr;
			unsigned pix_pixel;
			unsigned pix_scanline;
			unsigned char* dat_ptr_ext;
			unsigned dat_size;
			unsigned char* pal_ptr_ext;
			unsigned pal_size;
			unsigned tick;
			int r;

			r = adv_mng_read(mng, &pix_width, &pix_height, &pix_pixel, &dat_ptr_ext, &dat_size, &pix_ptr, &pix_scanline, &pal_ptr_ext, &pal_size, &tick, f_in);
			if (r < 0) {
				throw_png_error();
			}
			if (r > 0)
				break;

			data_ptr dat_ptr(dat_ptr_ext);
			data_ptr pal_ptr(pal_ptr_ext);

			if (first) {
				unsigned frequency = adv_mng_frequency_get(mng);
				if (opt_type == mng_vlc && tick!=1) {
					// adjust the frequency
					frequency = (frequency + tick / 2) / tick;
					if (frequency == 0)
						frequency = 1;
				}
				convert_header(mng_write, f_out, filec, adv_mng_width_get(mng), adv_mng_height_get(mng), frequency, info, pix_pixel == 4 && !opt_noalpha);
				first = false;
			}

			if (opt_type != mng_vlc)
				mng_write_frame(mng_write, f_out, filec, tick);

			if (info) {
				if (counter >= info->mac) {
					throw error() << "Internal error";
				}
				convert_image(mng_write, f_out, filec, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, &info->map[counter]);
			} else {
				convert_image(mng_write, f_out, filec, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, 0);
			}

			++counter;
			if (opt_verbose) {
				cout << "Compressing ";
				if (reduce) cout << "and reducing ";
				if (expand) cout << "and expanding ";
				cout << "frame " << counter << ", size " << *filec << "    \r";
				cout.flush();
			}
		}
	} catch (...) {
		adv_mng_done(mng);
		mng_write_done(mng_write);
		if (opt_verbose) {
			cout << endl;
		}
		throw;
	}

	mng_write_footer(mng_write, f_out, filec);

	adv_mng_done(mng);
	mng_write_done(mng_write);

	if (opt_verbose) {
		clear_line();
	}

	*framec = counter;
}

void convert_mng(const string& path_src, const string& path_dst)
{
	adv_scroll_info* info;
	bool reduce;
	bool expand;

	if (opt_scroll && opt_type == mng_vlc) {
		throw error() << "The --scroll and --vlc options are incompatible";
	}

	if (opt_scroll && opt_type == mng_lc) {
		throw error() << "The --scroll and --lc options are incompatible";
	}

	if (opt_scroll) {
		info = analyze_mng(path_src);
	} else {
		info = 0;
	}

	if (opt_reduce) {
		reduce = is_reducible_mng(path_src);
	} else {
		reduce = false;
	}

	if (opt_expand) {
		expand = true;
	} else {
		expand = false;
	}

	adv_fz* f_in;
	adv_fz* f_out;
	
	unsigned filec;
	unsigned framec;

	f_in = fzopen(path_src.c_str(), "rb");
	if (!f_in) {
		throw error() << "Failed open for reading " << path_src;
	}

	f_out = fzopen(path_dst.c_str(), "wb");
	if (!f_out) {
		fzclose(f_in);
		throw error() << "Failed open for writing " << path_dst;
	}

	try {
		convert_f_mng(f_in, f_out, &filec, &framec, info, reduce, expand);
	} catch (...) {
		fzclose(f_in);
		fzclose(f_out);
		remove(path_dst.c_str());
		throw;
	}

	fzclose(f_in);
	fzclose(f_out);

	if (info)
		scroll_info_done(info);
}

void convert_mng_inplace(const string& path)
{
	// temp name of the saved file
	string path_dst = file_basepath(path) + ".tmp";

	try {
		convert_mng(path, path_dst);
	} catch (...) {
		remove(path_dst.c_str());
		throw;
	}

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

void mng_print(const string& path)
{
	unsigned type;
	unsigned size;
	adv_fz* f_in;

	f_in = fzopen(path.c_str(), "rb");
	if (!f_in) {
		throw error() << "Failed open for reading " << path;
	}

	try {
		if (adv_mng_read_signature(f_in) != 0) {
			throw_png_error();
		}

		do {
			unsigned char* data_ext;

			if (adv_png_read_chunk(f_in, &data_ext, &size, &type) != 0) {
				throw_png_error();
			}

			data_ptr data(data_ext);

			if (opt_crc) {
				cout << hex << setw(8) << setfill('0') << crc32(0, data, size);
				cout << " ";
				cout << dec << setw(0) << setfill(' ') << size;
				cout << "\n";
			} else {
				png_print_chunk(type, data, size);
			}

		} while (type != ADV_MNG_CN_MEND);

	} catch (...) {
		fzclose(f_in);
		throw;
	}

	fzclose(f_in);
}

void extract(const string& path_src)
{
	adv_fz* f_in;
	adv_mng* mng;
	adv_fz* f_out;
	unsigned counter;
	string base;
	unsigned first_tick;

	base = file_basename(path_src);
	
	f_in = fzopen(path_src.c_str(), "rb");
	if (!f_in) {
		throw error() << "Failed open for reading " << path_src;
	}

	mng = adv_mng_init(f_in);
	if (!mng) {
		throw error() << "Error in the mng stream";
	}

	counter = 0;
	first_tick = 1;

	while (1) {
		unsigned pix_width;
		unsigned pix_height;
		unsigned char* pix_ptr;
		unsigned pix_pixel;
		unsigned pix_scanline;
		unsigned char* dat_ptr_ext;
		unsigned dat_size;
		unsigned char* pal_ptr_ext;
		unsigned pal_size;
		unsigned tick;
		unsigned char* dst_ptr;
		unsigned dst_pixel;
		unsigned dst_scanline;
		int r;

		r = adv_mng_read(mng, &pix_width, &pix_height, &pix_pixel, &dat_ptr_ext, &dat_size, &pix_ptr, &pix_scanline, &pal_ptr_ext, &pal_size, &tick, f_in);
		if (r < 0) {
			throw_png_error();
		}
		if (r > 0)
			break;

		data_ptr dat_ptr(dat_ptr_ext);
		data_ptr pal_ptr(pal_ptr_ext);

		if (counter == 0) {
			first_tick = tick;
			if (!first_tick)
				first_tick = 1;
		}

		ostringstream path_dst;
		path_dst << base << "-";

		// not optimal code for g++ 2.95.3
		path_dst.setf(ios::right, ios::adjustfield);

		path_dst << setw(8) << setfill('0') << counter;
		path_dst << ".png";

		if (!opt_quiet) {
			cout << path_dst.str() << endl;
		}

		f_out = fzopen(path_dst.str().c_str(), "wb");
		if (!f_out) {
			fzclose(f_in);
			throw error() << "Failed open for writing " << path_dst.str();
		}

		++counter;

		if (!opt_noalpha) {
			// convert to 4 byte RGBA format.
			// mencoder 0.90 has problems with 3 byte RGB format.
			png_convert_4(pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, &dst_ptr, &dst_pixel, &dst_scanline);

			png_write(f_out, pix_width, pix_height, dst_pixel, dst_ptr, dst_scanline, 0, 0, 0, 0, opt_level);

			free(dst_ptr);
		} else {
			png_write(f_out, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, 0, 0, opt_level);
		}

		fzclose(f_out);
	}

	cout << adv_mng_frequency_get(mng) / (double)first_tick << endl;

	if (opt_verbose) {
		cout << endl;
		cout << "Example mencoder call:" << endl;
		cout << "mencoder " << base << "-\\*.png -mf on:w=" << adv_mng_width_get(mng) << ":h=" << adv_mng_height_get(mng) << ":fps=" << adv_mng_frequency_get(mng) / first_tick << ":type=png -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=1000:vhq -o " << base << ".avi" << endl;
	}

	adv_mng_done(mng);

	fzclose(f_in);
}

void add_all(int argc, char* argv[], unsigned frequency)
{
	unsigned counter;
	unsigned filec;
	adv_fz* f_out;
	string path_dst;
	adv_scroll_info* info;
	adv_mng_write* mng_write;
	bool reduce;
	bool expand;

	if (argc < 2) {
		throw error() << "Missing arguments";
	}

	if (opt_scroll && opt_type == mng_vlc) {
		throw error() << "The --scroll and --vlc options are incompatible";
	}

	if (opt_scroll && opt_type == mng_lc) {
		throw error() << "The --scroll and --lc options are incompatible";
	}

	if (opt_scroll) {
		info = analyze_png(argc - 1, argv + 1);
	} else {
		info = 0;
	}

	if (opt_reduce) {
		reduce = is_reducible_png(argc - 1, argv + 1);
	} else {
		reduce = false;
	}

	if (opt_expand) {
		expand = true;
	} else {
		expand = false;
	}

	path_dst = argv[0];
	f_out = fzopen(path_dst.c_str(), "wb");
	if (!f_out) {
		throw error() << "Failed open for writing " << path_dst;
	}

	mng_write = mng_write_init(opt_type, opt_level, reduce, expand);
	if (!mng_write) {
		throw error() << "Error in the mng stream";
	}

	filec = 0;
	counter = 0;

	try {
		for(int i=1;i<argc;++i) {
			adv_fz* f_in;
			string path_src = argv[i];
			f_in = fzopen(path_src.c_str(), "rb");
			if (!f_in) {
				throw error() << "Failed open for reading " << path_src;
			}

			try {
				unsigned char* dat_ptr_ext;
				unsigned dat_size;
				unsigned pix_pixel;
				unsigned pix_width;
				unsigned pix_height;
				unsigned char* pal_ptr_ext;
				unsigned pal_size;
				unsigned char* pix_ptr;
				unsigned pix_scanline;

				if (adv_png_read(
					&pix_width, &pix_height, &pix_pixel,
					&dat_ptr_ext, &dat_size,
					&pix_ptr, &pix_scanline,
					&pal_ptr_ext, &pal_size,
					f_in
				) != 0) {
					throw_png_error();
				}

				data_ptr dat_ptr(dat_ptr_ext);
				data_ptr pal_ptr(pal_ptr_ext);

				if (!mng_write_has_header(mng_write)) {
					convert_header(mng_write, f_out, &filec, pix_width, pix_height, frequency, info, pix_pixel == 4 && !opt_noalpha);
				}

				if (opt_type != mng_vlc)
					mng_write_frame(mng_write, f_out, &filec, 1);

				if (info) {
					if (counter >= info->mac) {
						throw error() << "Internal error";
					}
					convert_image(mng_write, f_out, &filec, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, &info->map[counter]);
				} else {
					convert_image(mng_write, f_out, &filec, pix_width, pix_height, pix_pixel, pix_ptr, pix_scanline, pal_ptr, pal_size, 0);
				}

				fzclose(f_in);

				++counter;
				if (opt_verbose) {
					cout << "Compressing ";
					if (reduce) cout << "and reducing ";
					if (expand) cout << "and expanding ";
					cout << "frame " << counter << ", size " << filec << "    \r";
					cout.flush();
				}

			} catch (...) {
				fzclose(f_in);
				throw;
			}
		}
	} catch (...) {
		if (opt_verbose) {
			cout << endl;
		}
		fzclose(f_out);
		remove(path_dst.c_str());
		if (info)
			scroll_info_done(info);
		throw;
	}

	mng_write_footer(mng_write, f_out, &filec);

	mng_write_done(mng_write);

	fzclose(f_out);

	if (info)
		scroll_info_done(info);

	if (opt_verbose) {
		clear_line();
	}
}

void remng_single(const string& file, unsigned long long& total_0, unsigned long long& total_1)
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
			convert_mng_inplace(file);
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

void remng_all(int argc, char* argv[])
{
	unsigned long long total_0 = 0;
	unsigned long long total_1 = 0;

	for(int i=0;i<argc;++i)
		remng_single(argv[i], total_0, total_1);

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
		mng_print(argv[i]);
	}
}

void extract_all(int argc, char* argv[])
{
	for(int i=0;i<argc;++i) {
		extract(argv[i]);
	}
}

#if HAVE_GETOPT_LONG
struct option long_options[] = {
	{"recompress", 0, 0, 'z'},
	{"list", 0, 0, 'l'},
	{"list-crc", 0, 0, 'L'},
	{"extract", 0, 0, 'x'},
	{"add", 1, 0, 'a'},

	{"shrink-store", 0, 0, '0'},
	{"shrink-fast", 0, 0, '1'},
	{"shrink-normal", 0, 0, '2'},
	{"shrink-extra", 0, 0, '3'},
	{"shrink-insane", 0, 0, '4'},

	{"scroll-square", 1, 0, 'S'},
	{"scroll", 1, 0, 's'},
	{"reduce", 0, 0, 'r'},
	{"expand", 0, 0, 'e'},
	{"lc", 0, 0, 'c'},
	{"vlc", 0, 0, 'C'},
	{"force", 0, 0, 'f'},

	{"quiet", 0, 0, 'q'},
	{"verbose", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'V'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "zlLxa:01234s:S:rencCfqvhV"

void version()
{
	cout << PACKAGE " v" VERSION " by Andrea Mazzoleni" << endl;
}

void usage()
{
	version();

	cout << "Usage: advmng [options] [FILES...]" << endl;
	cout << endl;
	cout << "Modes:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-l, --list            ", "-l    ") "  List the content of the files" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-z, --recompress      ", "-z    ") "  Recompress the specified files" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-x, --extract         ", "-x    ") "  Extract all the .PNG frames" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-a, --add F MNG PNG...", "-a    ") "  Create a .MNG file at F frame/s from .PNG files" << endl;
	cout << "Options:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-0, --shrink-store    ", "-0    ") "  Don't compress" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-1, --shrink-fast     ", "-1    ") "  Compress fast" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-2, --shrink-normal   ", "-2    ") "  Compress normal" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-3, --shrink-extra    ", "-3    ") "  Compress extra" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-4, --shrink-insane   ", "-4    ") "  Compress extreme" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-s, --scroll NxM      ", "-s NxM") "  Enable the scroll optimization with a NxM pattern" << endl;
	cout << "  " SWITCH_GETOPT_LONG("                      ", "      ") "  search. from -Nx-M to NxM. Example: -s 4x6" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-S, --scroll-square N ", "-S N  ") "  Enable the square scroll optimization with a NxN pattern" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-r, --reduce          ", "-r    ") "  Convert the output to palette 8 bit (if possible)" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-e, --expand          ", "-e    ") "  Convert the output to rgb 24 bit" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-n, --noalpha         ", "-n    ") "  Remove the alpha channel" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-c, --lc              ", "-c    ") "  Use the MNG LC (Low Complexity) format" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-C, --vlc             ", "-C    ") "  Use the MNG VLC (Very Low Complexity) format" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-f, --force           ", "-f    ") "  Force the new file also if it's bigger" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-q, --quiet           ", "-q    ") "  Don't print on the console" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-v, --verbose         ", "-v    ") "  Print on the console more information" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-h, --help            ", "-h    ") "  Help of the program" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-V, --version         ", "-B    ") "  Version of the program" << endl;
}

void process(int argc, char* argv[])
{
	unsigned add_frequency;
	enum cmd_t {
		cmd_unset, cmd_recompress, cmd_list, cmd_extract, cmd_add
	} cmd = cmd_unset;

	opt_quiet = false;
	opt_verbose = false;
	opt_level = shrink_normal;
	opt_reduce = false;
	opt_expand = false;
	opt_noalpha = false;
	opt_dx = 0;
	opt_dy = 0;
	opt_limit = 0;
	opt_scroll = false;
	opt_type = mng_std;
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
		case 'x' :
			if (cmd != cmd_unset)
				throw error() << "Too many commands";
			cmd = cmd_extract;
			break;
		case 'a' : {
			int n;
			if (cmd != cmd_unset)
				throw error() << "Too many commands";
			cmd = cmd_add;
			n = sscanf(optarg, "%d", &add_frequency);
			if (n != 1)
				throw error() << "Invalid option -a";
			if (add_frequency < 1 || add_frequency > 250)
				throw error() << "Invalid frequency";
			} break;
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
		case 's' : {
			int n;
			opt_dx = 0;
			opt_dy = 0;
			n = sscanf(optarg, "%dx%d", &opt_dx, &opt_dy);
			if (n != 2)
				throw error() << "Invalid option -s";
			if (opt_dx < 0 || opt_dy < 0
				|| (opt_dx == 0 && opt_dy == 0)
				|| opt_dx > 128 || opt_dy > 128)
				throw error() << "Invalid argument for option -s";
			opt_scroll = true;
			opt_limit = opt_dx + opt_dy;
			} break;
		case 'S' : {
			int n;
			opt_limit = 0;
			n = sscanf(optarg, "%d", &opt_limit);
			if (n != 1)
				throw error() << "Invalid option -S";
			if (opt_limit < 1 || opt_limit > 128)
				throw error() << "Invalid argument for option -S";
			opt_scroll = true;
			opt_dx = opt_limit;
			opt_dy = opt_limit;
			} break;
		case 'r' :
			opt_reduce = true;
			opt_expand = false;
			break;
		case 'e' :
			opt_reduce = false;
			opt_expand = true;
			break;
		case 'n' :
			opt_noalpha = true;
			break;
		case 'c' :
			opt_type = mng_lc;
			opt_force = true;
			break;
		case 'C' :
			opt_type = mng_vlc;
			opt_force = true;
			break;
		case 'f' :
			opt_force = true;
			break;
		case 'q' :
			opt_verbose = false;
			opt_quiet = true;
			break;
		case 'v' :
			opt_verbose = true;
			opt_quiet = false;
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
		remng_all(argc - optind, argv + optind);
		break;
	case cmd_list :
		list_all(argc - optind, argv + optind);
		break;
	case cmd_extract :
		extract_all(argc - optind, argv + optind);
		break;
	case cmd_add :
		add_all(argc - optind, argv + optind, add_frequency);
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


