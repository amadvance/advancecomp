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

#include "portable.h"

#include "utility.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <cstdio>
#include <cerrno>

#include <sys/stat.h>

using namespace std;

// ------------------------------------------------------------------------
// Generic utility

string strip_space(const string& s) {
	string r = s;
	while (r.length() && isspace(r[0]))
		r.erase(0,1);
	while (r.length() && isspace(r[r.length()-1]))
		r.erase(r.length()-1,1);
	return r;
}

string token_get(const string& s, unsigned& ptr, const char* sep) {
	unsigned start = ptr;
	while (ptr < s.length() && strchr(sep,s[ptr])==0)
		++ptr;
	return string(s,start,ptr-start);
}

void token_skip(const string& s, unsigned& ptr, const char* sep) {
	while (ptr < s.length() && strchr(sep,s[ptr])!=0)
		++ptr;
}

std::string token_get(const std::string& s, unsigned& ptr, char sep) {
	char sep_string[2];
	sep_string[0] = sep;
	sep_string[1] = 0;
	return token_get(s,ptr,sep_string);
}

void token_skip(const std::string& s, unsigned& ptr, char sep) {
	char sep_string[2];
	sep_string[0] = sep;
	sep_string[1] = 0;
	token_skip(s,ptr,sep_string);
}

string numhex(unsigned v)
{
	ostringstream s;
	s << setw(8) << setfill('0') << hex << v;
	return s.str();
}

string numdec(unsigned v)
{
	ostringstream s;
	s << v;
	return s.str();
}

/* Match one string with a shell pattern expression
   in:
     pattern pattern with
       *      any string, including the null string
       ?      any single char
   return
      ==0 match
      !=0 don't match
*/
int striwildcmp(const char* pattern, const char* str) {
	while (*str && *pattern) {
		if (*pattern == '*') {
			++pattern;
			while (*str) {
				if (striwildcmp( pattern, str )==0) return 0;
				++str;
			}
		} else if (*pattern == '?') {
			++str;
			++pattern;
		} else {
			if (toupper(*str) != toupper(*pattern))
				return 1;
			++str;
			++pattern;
		}
	}
	while (*pattern == '*')
		++pattern;
	if (!*str && !*pattern)
		return 0;
	return 1;
}

// Convert a string to a unsigned
// return:
//   0 if string contains a non digit char
unsigned strdec(const char* s) {
	unsigned v = 0;
	while (*s) {
		if (!isdigit(*s)) return 0;
		v *= 10;
		v += *s - '0';
		++s;
	}
	return v;
}

// Convert a hex string to a unsigned
// return:
//   0 if string contains a non hex digit char
unsigned strhex(const char* s) {
	unsigned v = 0;
	while (*s) {
		if (!isxdigit(*s)) return 0;
		v *= 16;
		if (*s>='0' && *s<='9')
			v += *s - '0';
		else
			v += toupper(*s) - 'A' + 10;
		++s;
	}
	return v;
}

// ------------------------------------------------------------------------
// filepath

filepath::filepath() {
}

filepath::filepath(const filepath& A) : 
	file(A.file) {
}

filepath::filepath(const string& Afile) : 
	file(Afile) {
}

filepath::~filepath() {
}

void filepath::file_set(const string& Afile) {
	file = Afile;
}

// ------------------------------------------------------------------------
// zippath

zippath::zippath() {
	readonly = true;
	good = false;
	size = 0;
}

zippath::zippath(const zippath& A) : 
	filepath(A), good(A.good), size(A.size), readonly(A.readonly) {
}

zippath::zippath(const string& Afile, bool Agood, unsigned Asize, bool Areadonly) : 
	filepath(Afile), good(Agood), size(Asize), readonly(Areadonly) {
}

zippath::~zippath() {
}

void zippath::good_set(bool Agood) {
	good = Agood;
}

void zippath::size_set(unsigned Asize) {
	size = Asize;
}

void zippath::readonly_set(bool Areadonly) {
	readonly = Areadonly;
}

// ------------------------------------------------------------------------
// File utility

bool file_exists(const string& path) throw (error) {
	struct stat s;
	if (stat(path.c_str(),&s) != 0) {
		if (errno!=ENOENT)
			throw error() << "Failed stat file " << path;

		return false;
	}

	return !S_ISDIR(s.st_mode);
}

void file_write(const string& path, const char* data, unsigned size) throw (error) {
	FILE* f = fopen( path.c_str(), "wb" );
	if (!f)
		throw error() << "Failed open for write file " << path;

	if (fwrite(data,size,1,f)!=1) {
		fclose(f);

		remove(path.c_str());

		throw error() << "Failed write file " << path;
	}

	fclose(f);
}

void file_read(const string& path, char* data, unsigned size) throw (error) {
	file_read(path, data, 0, size);
}

void file_read(const string& path, char* data, unsigned offset, unsigned size) throw (error) {
	FILE* f = fopen( path.c_str(), "rb" );
	if (!f)
		throw error() << "Failed open for read file " << path;

	if (fseek(f,offset,SEEK_SET)!=0) {
		fclose(f);

		throw error() << "Failed seek file " << path;
	}

	if (fread(data,size,1,f)!=1) {
		fclose(f);

		throw error() << "Failed read file " << path;
	}

	fclose(f);
}

time_t file_time(const string& path) throw (error) {
	struct stat s;
	if (stat(path.c_str(), &s)!=0)
		throw error() << "Failed stat file " << path;

	return s.st_mtime;
}

void file_utime(const string& path, time_t tod) throw (error) {
	struct utimbuf u;

	u.actime = tod;
	u.modtime = tod;

	if (utime(path.c_str(),&u) != 0)
		throw error() << "Failed utime file " << path;
}

unsigned file_size(const string& path) throw (error) {
	struct stat s;
	if (stat(path.c_str(), &s)!=0)
		throw error() << "Failed stat file " << path;

	return s.st_size;
}

crc_t file_crc(const string& path) throw (error) {
	unsigned size = file_size(path);

	char* data = (char*)operator new(size);

	try {
		file_read(path,data,size);
	} catch (...) {
		operator delete(data);
		throw;
	}

	crc_t crc = crc_compute(data, size);

	operator delete(data);

	return crc;
}

void file_copy(const string& path1, const string& path2) throw (error) {
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

void file_move(const string& path1, const string& path2) throw (error) {
	if (rename(path1.c_str(),path2.c_str())!=0
		&& errno==EXDEV) {

		try {
			file_copy(path1,path2);
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

/// Remove a file
void file_remove(const string& path1) throw (error) {
	if (remove(path1.c_str())!=0) {
		throw error() << "Failed remove of " << path1;
	}
}

/// Rename a file
void file_rename(const string& path1, const string& path2) throw (error) {
	if (rename(path1.c_str(),path2.c_str())!=0) {
		throw error() << "Failed rename of " << path1 << " to " << path2;
	}
}

/// Randomize a name file
string file_randomize(const string& path, int n) throw () {
	ostringstream os;

	unsigned pos = path.rfind('.');

	if (pos == string::npos) 
		os << path << ".";
	else
		os << string(path,0,pos+1);
	
	os << n << ends;
	
	return os.str();
}

// Estrae la directory da un path
string file_dir(const string& path) throw () {
	unsigned pos = path.rfind('/');
	if (pos == string::npos) {
		return "";
	} else {
		return string(path,0,pos+1);
	}
}

// Estrae il nome file da un path
string file_name(const string& path) throw () {
	unsigned pos = path.rfind('/');
	if (pos == string::npos) {
		return path;
	} else {
		return string(path,pos+1);
	}
}

string file_basepath(const string& path) throw () {
	unsigned dot = path.rfind('.');
	if (dot == string::npos)
		return path;
	else
		return string(path,0,dot);
}

string file_basename(const string& path) throw () { 
	string name = file_name(path);
	unsigned dot = name.rfind('.');
	if (dot == string::npos)
		return name;
	else
		return string(name,0,dot);
}
 
string file_ext(const string& path) throw () { 
	string name = file_name(path);
	unsigned dot = name.rfind('.');
	if (dot == string::npos)
		return "";
	else
		return string(name,dot);
} 
 
// Compare two file name
int file_compare(const string& path1,const string& path2) throw () {
	return strcasecmp(path1.c_str(),path2.c_str());
}

string file_adjust(const string& path) throw () {
	string r;
	for(unsigned i=0;i<path.length();++i) {
		if (path[i]=='\\' || path[i]=='/') {
			if (i+1<path.length())
				r.insert( r.length(), 1, '/');
		} else {
			r.insert( r.length(), 1, path[i]);
		}
		
	}
	return r;
}

void file_mktree(const std::string& path) throw (error) {
	string dir = file_dir(path);
	string name = file_name(path);

	if (dir.length() && dir[dir.length() - 1] == '/')
		dir.erase(dir.length() - 1,1);

	if (dir.length()) {
		file_mktree(dir);

		struct stat s;
		if (stat(dir.c_str(),&s) != 0) {
			if (errno!=ENOENT)
				throw error() << "Failed stat dir " << dir;
#ifdef HAVE_FUNC_MKDIR_ONEARG
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

// ------------------------------------------------------------------------
// data

unsigned char* data_dup(const unsigned char* Adata, unsigned Asize) {
	if (Adata) {
		unsigned char* data = (unsigned char*)operator new(Asize);
		if (Asize)
			memcpy(data,Adata,Asize);
		return data;
	} else {
		return 0;
	}
}

unsigned char* data_alloc(unsigned size) {
	return (unsigned char*)operator new(size);
}

void data_free(unsigned char* data) {
	operator delete(data);
}

