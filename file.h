/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2004 Andrea Mazzoleni
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

#ifndef __FILE_H
#define __FILE_H

#include "except.h"

#include <string>
#include <list>

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

class infopath : public filepath {
	bool good;
	unsigned size;
	bool readonly;
public:
	infopath();
	infopath(const infopath& A);
	infopath(const std::string& Afile, bool Agood, unsigned Asize, bool Areadonly);
	~infopath();

	bool good_get() const { return good; }
	void good_set(bool Agood);

	unsigned size_get() const { return size; }
	void size_set(unsigned Asize);
	
	bool readonly_get() const { return readonly; }
	void readonly_set(bool Areadonly);
};

typedef std::list<infopath> zippath_container;

typedef unsigned crc_t;

crc_t crc_compute(const char* data, unsigned len);
crc_t crc_compute(crc_t pred, const char* data, unsigned len);

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

#endif

