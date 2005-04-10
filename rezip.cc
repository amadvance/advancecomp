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

#include "zip.h"
#include "file.h"

#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

void rezip_single(const string& file, unsigned long long& total_0, unsigned long long& total_1, bool quiet, bool standard, shrink_t level)
{
	zip z(file);

	unsigned size_0;
	unsigned size_1;

	if (!file_exists(file)) {
		throw error() << "File " << file << " doesn't exist";
	}

	try {
		size_0 = file_size(file);

		z.open();
		z.load();

		z.shrink(standard, level);

		z.save();

		z.close();

		size_1 = file_size(file);
	} catch (error& e) {
		throw e << " on " << file;
	}

	if (!quiet) {
		cout << setw(12) << size_0 << setw(12) << size_1 << " ";
		if (size_0) {
			unsigned perc = size_1 * 100LL / size_0;
			cout << setw(3) << perc;
		} else {
			cout << "  0";
		}
		cout << "% " << file << endl;
	}

	total_0 += size_0;
	total_1 += size_1;
}

void rezip_all(int argc, char* argv[], bool quiet, bool standard, shrink_t level)
{
	unsigned long long total_0 = 0;
	unsigned long long total_1 = 0;

	for(int i=0;i<argc;++i)
		rezip_single(argv[i], total_0, total_1, quiet, standard, level);

	if (!quiet) {
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

void list_single(const string& file, bool crc)
{
	if (!file_exists(file)) {
		throw error() << "File " << file << " doesn't exist";
	}

	zip z(file);

/*
Archive:  /mnt/bag/home/am/data/src/advscan/prova.zip
  Length   Method    Size  Ratio   Date   Time   CRC-32    Name
 --------  ------  ------- -----   ----   ----   ------    ----
     4366  Unk:012    1030  76%  04-20-02 13:58  58ab4966  test/advmame.lst
      268  Defl:X       89  67%  04-18-02 18:13  e6527450  test/advscan.rc
       29  Stored       29   0%  04-22-02 19:28  d3f60a47  test/test
   101057  Unk:012   90765  10%  04-20-02 14:55  c42107d7  test/unknown/40love_reject.zip
    96730  Unk:012   95538   1%  04-20-02 14:55  cafec0cf  test/unknown/40love2.zip
    96730  Unk:012   95538   1%  04-20-02 14:55  cafec0cf  test/rom/40love.zip
 --------          -------  ---                            -------
   299180           282989   5%                            6 files
*/

	try {
		z.open();

		unsigned long long compressed_size = 0;
		unsigned long long uncompressed_size = 0;

		if (crc) {
			for(zip::iterator i=z.begin();i!=z.end();++i) {
				cout << hex << setw(8) << setfill('0') << i->crc_get();
				cout << " ";
				cout << dec << setw(0) << setfill(' ') << i->compressed_size_get();
				cout << "\n";
			}

		} else {

		cout << "Archive:  " << file << endl;
		cout << "  Length   Method    Size  Ratio   Date   Time   CRC-32    Name" << endl;
		cout << " --------  ------  ------- -----   ----   ----   ------    ----" << endl;

		for(zip::iterator i=z.begin();i!=z.end();++i) {

			// not optimal code for g++ 2.95.3
			cout.setf(ios::right, ios::adjustfield);

			cout << dec << setw(9) << setfill(' ') << i->uncompressed_size_get();

			cout << "  ";

			string m;

			switch (i->method_get()) {
			case zip_entry::store : m = "Stored"; break;
			case zip_entry::shrunk : m = "Shrunk"; break;
			case zip_entry::reduce1 : m = "Reduce1"; break;
			case zip_entry::reduce2 : m = "Reduce2"; break;
			case zip_entry::reduce3 : m = "Reduce3"; break;
			case zip_entry::reduce4 : m = "Reduce4"; break;
			case zip_entry::implode_4kdict_2tree : m = "Impl:42"; break;
			case zip_entry::implode_8kdict_2tree : m = "Impl:82"; break;
			case zip_entry::implode_4kdict_3tree : m = "Impl:43"; break;
			case zip_entry::implode_8kdict_3tree : m = "Impl:83"; break;
			case zip_entry::deflate0 : m = "Defl:S"; break;
			case zip_entry::deflate1 : m = "Defl:S"; break;
			case zip_entry::deflate2 : m = "Defl:S"; break;
			case zip_entry::deflate3 : m = "Defl:F"; break;
			case zip_entry::deflate4 : m = "Defl:F"; break;
			case zip_entry::deflate5 : m = "Defl:F"; break;
			case zip_entry::deflate6 : m = "Defl:N"; break;
			case zip_entry::deflate7 : m = "Defl:N"; break;
			case zip_entry::deflate8 : m = "Defl:N"; break;
			case zip_entry::deflate9 : m = "Defl:X"; break;
			case zip_entry::bzip2 : m = "Bzip2"; break;
			case zip_entry::lzma : m = "LZMA"; break;
			default: m = "Unknown"; break;
			}

			cout.setf(ios::left, ios::adjustfield);

			// not optimal code for g++ 2.95.3
			cout << setw(7) << m.c_str();

			cout.setf(ios::right, ios::adjustfield);

			cout << setw(8) << i->compressed_size_get();

			cout << " ";

			if (i->uncompressed_size_get()) {
				unsigned perc = (i->uncompressed_size_get() - i->compressed_size_get()) * 100LL / i->uncompressed_size_get();
				cout << setw(3) << perc;
			} else {
				cout << "  0";
			}

			cout << "%  ";

			time_t t = i->time_get();
			struct tm* tm = localtime(&t);

			cout << setw(2) << setfill('0') << tm->tm_mon + 1;
			cout << "-";
			cout << setw(2) << setfill('0') << tm->tm_mday;
			cout << "-";
			cout << setw(2) << setfill('0') << (tm->tm_year % 100);

			cout << " ";

			cout << setw(2) << setfill('0') << tm->tm_hour;
			cout << ":";
			cout << setw(2) << setfill('0') << tm->tm_min;

			cout << "  ";

			cout << hex << setw(8) << setfill('0') << i->crc_get();

			cout << "  ";

			cout << i->name_get();

			cout << endl;

			uncompressed_size += i->uncompressed_size_get();
			compressed_size += i->compressed_size_get();
		}

		cout << " --------          -------  ---                            -------" << endl;

		cout << dec << setw(9) << setfill(' ') << uncompressed_size;

		cout << "        ";

		cout << dec << setw(9) << setfill(' ') << compressed_size;

		cout << " ";

		if (uncompressed_size) {
			unsigned perc = (uncompressed_size - compressed_size) * 100LL / uncompressed_size;
			cout << setw(3) << perc;
		} else {
			cout << "  0";
		}

		cout << "%                            ";

		cout << z.size() << " files" << endl;

		}

		z.close();

	} catch (error& e) {
		throw e << " on " << file;
	}
}

void list_all(int argc, char* argv[], bool crc)
{
	string file(argv[0]);
	for(int i=0;i<argc;++i) {
		list_single(argv[i], crc);
	}
}

void test_single(const string& file, bool quiet)
{
	zip z(file);

	if (!file_exists(file)) {
		throw error() << "File " << file << " doesn't exist";
	}

	if (!quiet)
		cout << file << endl;

	try {
		z.open();
		z.load();
		z.test();
		z.close();
	} catch (error& e) {
		throw e << " on " << file;
	}
}

void test_all(int argc, char* argv[], bool quiet)
{
	for(int i=0;i<argc;++i)
		test_single(argv[i], quiet);
}

void extract_all(int argc, char* argv[], bool quiet)
{
	if (argc > 1)
		throw error() << "Too many archives specified";
	if (argc < 1)
		throw error() << "No archive specified";

	zip z(argv[0]);

	z.open();
	z.load();

	for(zip::iterator i=z.begin();i!=z.end();++i) {
		unsigned char* data = (unsigned char*)operator new(i->uncompressed_size_get());

		try {
			i->uncompressed_read(data);

			string file = i->name_get();

			// if end with / it's a directory
			if (file.length() && file[file.length()-1]!='/') {

				if (!quiet)
					cout << file << endl;

				file_mktree(file);

				FILE* f = fopen(file.c_str(), "wb");
				if (!f)
					throw error() << "Failed open for writing file " << file;

				try {
					if (fwrite(data, i->uncompressed_size_get(), 1, f)!=1)
						throw error() << "Failed write file " << file;
				} catch (...) {
					fclose(f);
					throw;
				}

				fclose(f);

				file_utime(file, i->time_get());
			}
		} catch (...) {
			operator delete(data);
			throw;
		}

		operator delete(data);
	}

	z.close();
}

void add_single(zip& z, const string& local, const string& common, bool quiet, bool standard, shrink_t level)
{
	struct stat st;
	string file = local + common;
	string name = file_name(file);

	// ignore special file
	if (name == "." || name == "..")
		return;

	if (stat(file.c_str(), &st)!=0)
		throw error() << "Failed stat file " << file;

	if (S_ISDIR(st.st_mode)) {
		DIR* d = opendir(file.c_str());
		if (!d)
			throw error() << "Failed open dir " << file;
		try {
			struct dirent* dd;
			while ((dd = readdir(d)) != 0) {
				add_single(z, local, common + "/" + dd->d_name, quiet, standard, level);
			}
		} catch (...) {
			closedir(d);
			throw;
		}
		closedir(d);
	} else {
		unsigned char* data = (unsigned char*)operator new(st.st_size);

		try {
			if (!quiet)
				cout << file << endl;

			FILE* f = fopen(file.c_str(), "rb");
			if (!f)
				throw error() << "Failed open for reading file " << file;

			if (fread(data, st.st_size, 1, f)!=1)
				throw error() << "Failed read file " << file;

			fclose(f);

			zip::iterator i = z.insert_uncompressed(common, data, st.st_size, crc32(0, (unsigned char*)data, st.st_size), st.st_mtime, false);

			if (level != shrink_none)
				i->shrink(standard, level);
		} catch (...) {
			operator delete(data);
			throw;
		}

		operator delete(data);
	}
}

void add_all(int argc, char* argv[], bool quiet, bool standard, shrink_t level)
{
	if (argc < 1)
		throw error() << "No archive specified";
	if (argc < 2)
		throw error() << "No files specified";

	zip z(argv[0]);

	z.create();

	for(int i=1;i<argc;++i) {
		string file = argv[i];

		string local = file_dir(file);
		string common = file_name(file);

		add_single(z, local, common, quiet, standard, level);
	}

	z.save();
	z.close();
}

#if HAVE_GETOPT_LONG
struct option long_options[] = {
	{"add", 0, 0, 'a'},
	{"extract", 0, 0, 'x'},
	{"recompress", 0, 0, 'z'},
	{"test", 0, 0, 't'},
	{"list", 0, 0, 'l'},
	{"list-crc", 0, 0, 'L'},

	{"not-zip", 0, 0, 'N'},
	{"pedantic", 0, 0, 'p'},

	{"shrink-store", 0, 0, '0'},
	{"shrink-fast", 0, 0, '1'},
	{"shrink-normal", 0, 0, '2'},
	{"shrink-extra", 0, 0, '3'},
	{"shrink-insane", 0, 0, '4'},

	{"verbose", 0, 0, 'v'},
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'V'},
	{0, 0, 0, 0}
};
#endif

#define OPTIONS "axztlLNp01234qhV"

void version()
{
	cout << PACKAGE " v" VERSION " by Andrea Mazzoleni" << endl;
}

void usage()
{
	version();

	cout << "Usage: advzip [options] ARCHIVES... [FILES...]" << endl;
	cout << endl;
	cout << "Modes:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-a, --add           ", "-a") "  Create a new archive with the specified files" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-x, --extract       ", "-x") "  Extract the content of an archive" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-l, --list          ", "-l") "  List the content of the archives" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-t, --test          ", "-t") "  Test the specified archives" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-z, --recompress    ", "-z") "  Recompress the specified archives" << endl;
	cout << "Options:" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-p, --pedantic      ", "-p") "  Be pedantic on the zip tests" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-0, --shrink-store  ", "-0") "  Don't compress" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-1, --shrink-fast   ", "-1") "  Compress fast" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-2, --shrink-normal ", "-2") "  Compress normal" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-3, --shrink-extra  ", "-3") "  Compress extra" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-4, --shrink-insane ", "-4") "  Compress extreme" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-q, --quiet         ", "-q") "  Don't print on the console" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-h, --help          ", "-h") "  Help of the program" << endl;
	cout << "  " SWITCH_GETOPT_LONG("-V, --version       ", "-V") "  Version of the program" << endl;
}

void process(int argc, char* argv[])
{
	enum cmd_t {
		cmd_unset, cmd_add, cmd_extract, cmd_recompress, cmd_test, cmd_list
	} cmd = cmd_unset;
	bool quiet = false;
	bool notzip = false;
	bool pedantic = false;
	bool crc = false;
	shrink_t level = shrink_normal;

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
		case 'a' :
			if (cmd != cmd_unset)
				throw error() << "Too many commands";
			cmd = cmd_add;
			break;
		case 'x' :
			if (cmd != cmd_unset)
				throw error() << "Too many commands";
			cmd = cmd_extract;
			break;
		case 'z' :
			if (cmd != cmd_unset)
				throw error() << "Too many commands";
			cmd = cmd_recompress;
			break;
		case 't' :
			if (cmd != cmd_unset)
				throw error() << "Too many commands";
			cmd = cmd_test;
			break;
		case 'l' :
			if (cmd != cmd_unset)
				throw error() << "Too many commands";
			cmd = cmd_list;
			break;
		case 'L' :
			crc = true;
			if (cmd != cmd_unset)
				throw error() << "Too many commands";
				cmd = cmd_list;
			break;
		case 'N' :
			notzip = true;
			break;
		case 'p' :
			pedantic = true;
			break;
		case '0' :
			level = shrink_none;
			break;
		case '1' :
			level = shrink_fast;
			break;
		case '2' :
			level = shrink_normal;
			break;
		case '3' :
			level = shrink_extra;
			break;
		case '4' :
			level = shrink_extreme;
			break;
		case 'q' :
			quiet = true;
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

	if (pedantic)
		zip::pedantic_set(pedantic);

	switch (cmd) {
	case cmd_recompress :
		rezip_all(argc - optind, argv + optind, quiet, !notzip, level);
		break;
	case cmd_extract :
		extract_all(argc - optind, argv + optind, quiet);
		break;
	case cmd_add :
		add_all(argc - optind, argv + optind, quiet, !notzip, level);
		break;
	case cmd_test :
		test_all(argc - optind, argv + optind, quiet);
		break;
	case cmd_list :
		list_all(argc - optind, argv + optind, crc);
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

