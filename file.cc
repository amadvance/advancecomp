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

#include "file.h"

#include <zlib.h>

using namespace std;

crc_t crc_compute(const char* data, unsigned len)
{
	return crc32(0, (unsigned char*)data, len);
}

crc_t crc_compute(crc_t pred, const char* data, unsigned len)
{
	return crc32(pred, (unsigned char*)data, len);
}

filepath::filepath()
{
}

filepath::filepath(const filepath& A)
	: file(A.file)
{
}

filepath::filepath(const string& Afile)
	: file(Afile)
{
}

filepath::~filepath()
{
}

void filepath::file_set(const string& Afile)
{
	file = Afile;
}

infopath::infopath()
{
	readonly = true;
	good = false;
	size = 0;
}

infopath::infopath(const infopath& A)
	: filepath(A), good(A.good), size(A.size), readonly(A.readonly)
{
}

infopath::infopath(const string& Afile, bool Agood, unsigned Asize, bool Areadonly)
	: filepath(Afile), good(Agood), size(Asize), readonly(Areadonly)
{
}

infopath::~infopath()
{
}

void infopath::good_set(bool Agood)
{
	good = Agood;
}

void infopath::size_set(unsigned Asize)
{
	size = Asize;
}

void infopath::readonly_set(bool Areadonly)
{
	readonly = Areadonly;
}

/**
 * Check if a file exists.
 */
bool file_exists(const string& path) throw (error)
{
	struct stat s;
	if (stat(path.c_str(), &s) != 0) {
		if (errno!=ENOENT)
			throw error() << "Failed stat file " << path;

		return false;
	}

	return !S_ISDIR(s.st_mode);
}

/**
 * Write a whole file.
 */
void file_write(const string& path, const char* data, unsigned size) throw (error)
{
	FILE* f = fopen(path.c_str(), "wb");
	if (!f)
		throw error() << "Failed open for write file " << path;

	if (fwrite(data, size, 1, f)!=1) {
		fclose(f);

		remove(path.c_str());

		throw error() << "Failed write file " << path;
	}

	fclose(f);
}

/**
 * Read a whole file.
 */
void file_read(const string& path, char* data, unsigned size) throw (error)
{
	file_read(path, data, 0, size);
}

/**
 * Read a whole file.
 */
void file_read(const string& path, char* data, unsigned offset, unsigned size) throw (error)
{
	FILE* f = fopen(path.c_str(), "rb");
	if (!f)
		throw error() << "Failed open for read file " << path;

	if (fseek(f, offset, SEEK_SET)!=0) {
		fclose(f);

		throw error() << "Failed seek file " << path;
	}

	if (fread(data, size, 1, f)!=1) {
		fclose(f);

		throw error() << "Failed read file " << path;
	}

	fclose(f);
}

/**
 * Get the time of a file.
 */
time_t file_time(const string& path) throw (error)
{
	struct stat s;
	if (stat(path.c_str(), &s)!=0)
		throw error() << "Failed stat file " << path;

	return s.st_mtime;
}

/**
 * Set the time of a file.
 */
void file_utime(const string& path, time_t tod) throw (error)
{
	struct utimbuf u;

	u.actime = tod;
	u.modtime = tod;

	if (utime(path.c_str(), &u) != 0)
		throw error() << "Failed utime file " << path;
}

/**
 * Get the size of a file.
 */
unsigned file_size(const string& path) throw (error)
{
	struct stat s;
	if (stat(path.c_str(), &s)!=0)
		throw error() << "Failed stat file " << path;

	return s.st_size;
}

/**
 * Get the crc of a file.
 */
crc_t file_crc(const string& path) throw (error)
{
	unsigned size = file_size(path);

	char* data = (char*)operator new(size);

	try {
		file_read(path, data, size);
	} catch (...) {
		operator delete(data);
		throw;
	}

	crc_t crc = crc_compute(data, size);

	operator delete(data);

	return crc;
}

/**
 * Copy a file.
 */
void file_copy(const string& path1, const string& path2) throw (error)
{
	unsigned size;

	size = file_size(path1);

	char* data = (char*)operator new(size);

	try {
		file_read(path1, data, size);
		file_write(path2, data, size);
	} catch (...) {
		operator delete(data);
		throw;
	}

	operator delete(data);
}

/**
 * Move a file.
 */
void file_move(const string& path1, const string& path2) throw (error)
{
	if (rename(path1.c_str(), path2.c_str())!=0
		&& errno==EXDEV) {

		try {
			file_copy(path1, path2);
		} catch (...) {
			try {
				file_remove(path2);
			} catch (...) {
			}
			throw error() << "Failed move of " << path1 << " to " << path2;
		}

		file_remove(path1);
	}
}

/**
 * Remove a file.
 */
void file_remove(const string& path1) throw (error)
{
	if (remove(path1.c_str())!=0) {
		throw error() << "Failed remove of " << path1;
	}
}

/**
 * Rename a file.
 */
void file_rename(const string& path1, const string& path2) throw (error)
{
	if (rename(path1.c_str(), path2.c_str())!=0) {
		throw error() << "Failed rename of " << path1 << " to " << path2;
	}
}

/**
 * Randomize a name file.
 */
string file_randomize(const string& path, int n) throw ()
{
	ostringstream os;

	size_t pos = path.rfind('.');

	if (pos == string::npos) 
		os << path << ".";
	else
		os << string(path, 0, pos+1);
	
	os << n << ends;
	
	return os.str();
}

/**
 * Get the directory from a path.
 */
string file_dir(const string& path) throw ()
{
	size_t pos = path.rfind('/');
	if (pos == string::npos) {
		return "";
	} else {
		return string(path, 0, pos+1);
	}
}

/**
 * Get the file name from a path.
 */
string file_name(const string& path) throw ()
{
	size_t pos = path.rfind('/');
	if (pos == string::npos) {
		return path;
	} else {
		return string(path, pos+1);
	}
}

/**
 * Get the basepath (path without extension) from a path.
 */
string file_basepath(const string& path) throw ()
{
	size_t dot = path.rfind('.');
	if (dot == string::npos)
		return path;
	else
		return string(path, 0, dot);
}

/**
 * Get the basename (name without extension) from a path.
 */
string file_basename(const string& path) throw ()
{ 
	string name = file_name(path);
	size_t dot = name.rfind('.');
	if (dot == string::npos)
		return name;
	else
		return string(name, 0, dot);
}

/**
 * Get the extension from a path.
 */
string file_ext(const string& path) throw ()
{ 
	string name = file_name(path);
	size_t dot = name.rfind('.');
	if (dot == string::npos)
		return "";
	else
		return string(name, dot);
} 
 
/**
 * Compare two path.
 */
int file_compare(const string& path1, const string& path2) throw ()
{
	return strcasecmp(path1.c_str(), path2.c_str());
}

/**
 * Convert a path to the C format.
 */
string file_adjust(const string& path) throw ()
{
	string r;
	for(unsigned i=0;i<path.length();++i) {
		if (path[i]=='\\' || path[i]=='/') {
			if (i+1<path.length())
				r.insert(r.length(), 1, '/');
		} else {
			r.insert(r.length(), 1, path[i]);
		}
		
	}
	return r;
}

/**
 * Make a drectory tree.
 */
void file_mktree(const std::string& path) throw (error)
{
	string dir = file_dir(path);
	string name = file_name(path);

	if (dir.length() && dir[dir.length() - 1] == '/')
		dir.erase(dir.length() - 1, 1);

	if (dir.length()) {
		file_mktree(dir);

		struct stat s;
		if (stat(dir.c_str(), &s) != 0) {
			if (errno!=ENOENT)
				throw error() << "Failed stat dir " << dir;
#if HAVE_FUNC_MKDIR_ONEARG
			if (mkdir(dir.c_str()) != 0)
#else
			if (mkdir(dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
#endif
				throw error() << "Failed mkdir " << dir;
		} else {
			if (!S_ISDIR(s.st_mode))
				throw error() << "Failed mkdir " << dir << " because a file with the same name exists";
		}
	}
}


