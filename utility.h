/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998-2002 Andrea Mazzoleni
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

#ifndef __UTILITY_H
#define __UTILITY_H

#include "except.h"

#include <iostream>
#include <string>
#include <list>
#include <ctime>

#include <zlib.h>

#define MESSAGE "log: "
#define cmessage cerr

#define WARNING "warning: "
#define cwarning cerr

// ------------------------------------------------------------------------
// Generic utility

int striwildcmp(const char* pattern, const char* str);
unsigned strdec(const char* s);
unsigned strhex(const char* s);

std::string numhex(unsigned v);
std::string numdec(unsigned v);

std::string token_get(const std::string& s, unsigned& ptr, const char* sep);
void token_skip(const std::string& s, unsigned& ptr, const char* sep);
std::string token_get(const std::string& s, unsigned& ptr, char sep);
void token_skip(const std::string& s, unsigned& ptr, char sep);
std::string strip_space(const std::string& s);

// ------------------------------------------------------------------------
// pathspec

class filepath {
	std::string file;
public:
	filepath();
	filepath(const filepath& A);
	filepath(const std::string& Afile);
	~filepath();
	
	const std::string& file_get() const { return file; }
	void file_set(const std::string& Afile);
};

typedef std::list<filepath> filepath_container;

class zippath : public filepath {
	bool good;
	unsigned size;
	bool readonly;
public:
	zippath();
	zippath(const zippath& A);
	zippath(const std::string& Afile, bool Agood, unsigned Asize, bool Areadonly);
	~zippath();

	bool good_get() const { return good; }
	void good_set(bool Agood);

	unsigned size_get() const { return size; }
	void size_set(unsigned Asize);
	
	bool readonly_get() const { return readonly; }
	void readonly_set(bool Areadonly);
};

typedef std::list<zippath> zippath_container;

// ------------------------------------------------------------------------
// crc

typedef unsigned crc_t;

inline crc_t crc_compute(const char* data, unsigned len) {
	return crc32(0, (unsigned char*)data, len);
}

inline crc_t crc_compute(crc_t pred, const char* data, unsigned len) {
	return crc32(pred, (unsigned char*)data, len);
}

// ------------------------------------------------------------------------
// File utility

bool file_exists(const std::string& file) throw (error);
void file_write(const std::string& path, const char* data, unsigned size) throw (error);
void file_read(const std::string& path, char* data, unsigned size) throw (error);
void file_read(const std::string& path, char* data, unsigned offset, unsigned size) throw (error);
time_t file_time(const std::string& path) throw (error);
void file_utime(const std::string& path, time_t tod) throw (error);
unsigned file_size(const std::string& path) throw (error);
crc_t file_crc(const std::string& path) throw (error);
void file_copy(const std::string& path1, const std::string& path2) throw (error);
void file_move(const std::string& path1, const std::string& path2) throw (error);
void file_remove(const std::string& path1) throw (error);
void file_mktree(const std::string& path1) throw (error);

std::string file_randomize(const std::string& path, int n) throw ();
std::string file_name(const std::string& file) throw ();
std::string file_dir(const std::string& file) throw ();
std::string file_basename(const std::string& file) throw ();
std::string file_basepath(const std::string& file) throw ();
std::string file_ext(const std::string& file) throw ();
int file_compare(const std::string& path1, const std::string& path2) throw ();
std::string file_adjust(const std::string& path) throw ();

// ------------------------------------------------------------------------
// data

unsigned char* data_dup(const unsigned char* Adata, unsigned Asize);
unsigned char* data_alloc(unsigned size);
void data_free(unsigned char* data);

class data_ptr {
	unsigned char* data;
	bool own;
public:
	data_ptr() : data(0), own(false) { }

	data_ptr(data_ptr& A)
	{
		data = A.data;
		own = A.own;
		A.own = false;
	}

	data_ptr(unsigned char* Adata, bool Aown = true) : data(Adata), own(Aown) { }

	~data_ptr() { if (own) data_free(data); }

	void operator=(data_ptr& A)
	{
		if (own) data_free(data);
		data = A.data;
		own = A.own;
		A.own = false;
	}

	void operator=(unsigned char* Adata)
	{
		if (own) data_free(data);
		data = Adata;
		own = true;
	}

	operator unsigned char*() { return data; }
};

#endif
