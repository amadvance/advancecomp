/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002 Andrea Mazzoleni
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

#ifndef __ZIP_H
#define __ZIP_H

#include "except.h"

#ifdef USE_COMPRESS
#include "compress.h"
#endif

#include <list>
#include <sstream>

// --------------------------------------------------------------------------
// Zip format

// Methods
#define ZIP_METHOD_STORE 0x00
#define ZIP_METHOD_SHRUNK 0x01
#define ZIP_METHOD_REDUCE1 0x02
#define ZIP_METHOD_REDUCE2 0x03
#define ZIP_METHOD_REDUCE3 0x04
#define ZIP_METHOD_REDUCE4 0x05
#define ZIP_METHOD_IMPLODE 0x06
#define ZIP_METHOD_TOKENIZE 0x07
#define ZIP_METHOD_DEFLATE 0x08

// Not standard methods
#define ZIP_METHOD_ENHDEFLATE 0x09
#define ZIP_METHOD_BZIP2 0x0C
#define ZIP_METHOD_LZMA 0x0F

// Generic flags
// If bit 0 is set, indicates that the file is encrypted.
#define ZIP_GEN_FLAGS_ENCRYPTED 0x01

// Compression method for deflate
#define ZIP_GEN_FLAGS_DEFLATE_NORMAL 0x00
#define ZIP_GEN_FLAGS_DEFLATE_MAXIMUM 0x02
#define ZIP_GEN_FLAGS_DEFLATE_FAST 0x04
#define ZIP_GEN_FLAGS_DEFLATE_SUPERFAST 0x06
#define ZIP_GEN_FLAGS_DEFLATE_MASK 0x06

// If bit 3 is set, the fields crc-32, compressed size
// and uncompressed size are set to zero in the local
// header. The correct values are put in the data descriptor
// immediately following the compressed data and in the central directory.
#define ZIP_GEN_FLAGS_DEFLATE_ZERO 0x08

// If bit 1 is set indicates an 8K sliding dictionary was used.
// If clear, then a 4K sliding dictionary was used.
// If bit 2 is set, indicates an 3 Shannon-Fano trees were used
// to encode the sliding dictionary output. If clear, then 2
// Shannon-Fano trees were used.
#define ZIP_GEN_FLAGS_IMPLODE_4KD2T 0x00
#define ZIP_GEN_FLAGS_IMPLODE_8KD2T 0x02
#define ZIP_GEN_FLAGS_IMPLODE_4KD3T 0x04
#define ZIP_GEN_FLAGS_IMPLODE_8KD3T 0x06
#define ZIP_GEN_FLAGS_IMPLODE_MASK 0x06

// Internal file attributes
// If set, that the file is apparently an ASCII or text file.
#define ZIP_INT_ATTR_TEXT 0x01

// Signature
#define ZIP_L_signature 0x04034b50
#define ZIP_C_signature 0x02014b50
#define ZIP_E_signature 0x06054b50

// Offsets in end of central directory structure
#define ZIP_EO_end_of_central_dir_signature 0x00
#define ZIP_EO_number_of_this_disk 0x04
#define ZIP_EO_number_of_disk_start_cent_dir 0x06
#define ZIP_EO_total_entries_cent_dir_this_disk 0x08
#define ZIP_EO_total_entries_cent_dir 0x0A
#define ZIP_EO_size_of_cent_dir 0x0C
#define ZIP_EO_offset_to_start_of_cent_dir 0x10
#define ZIP_EO_zipfile_comment_length 0x14
#define ZIP_EO_FIXED 0x16 // size of fixed data structure
#define ZIP_EO_zipfile_comment 0x16

// Offsets in central directory entry structure
#define ZIP_CO_central_file_header_signature 0x00
#define ZIP_CO_version_made_by 0x04
#define ZIP_CO_host_os 0x05
#define ZIP_CO_version_needed_to_extract 0x06
#define ZIP_CO_os_needed_to_extract 0x07
#define ZIP_CO_general_purpose_bit_flag 0x08
#define ZIP_CO_compression_method 0x0A
#define ZIP_CO_last_mod_file_time 0x0C
#define ZIP_CO_last_mod_file_date 0x0E
#define ZIP_CO_crc32 0x10
#define ZIP_CO_compressed_size 0x14
#define ZIP_CO_uncompressed_size 0x18
#define ZIP_CO_filename_length 0x1C
#define ZIP_CO_extra_field_length 0x1E
#define ZIP_CO_file_comment_length 0x20
#define ZIP_CO_disk_number_start 0x22
#define ZIP_CO_internal_file_attrib 0x24
#define ZIP_CO_external_file_attrib 0x26
#define ZIP_CO_relative_offset_of_local_header 0x2A
#define ZIP_CO_FIXED 0x2E // size of fixed data structure
#define ZIP_CO_filename 0x2E

// Offsets in data descriptor structure
#define ZIP_DO_crc32 0x00
#define ZIP_DO_compressed_size 0x04
#define ZIP_DO_uncompressed_size 0x08
#define ZIP_DO_FIXED 0x0C // size of fixed data structure

// Offsets in local file header structure
#define ZIP_LO_local_file_header_signature 0x00
#define ZIP_LO_version_needed_to_extract 0x04
#define ZIP_LO_os_needed_to_extract 0x05
#define ZIP_LO_general_purpose_bit_flag 0x06
#define ZIP_LO_compression_method 0x08
#define ZIP_LO_last_mod_file_time 0x0A
#define ZIP_LO_last_mod_file_date 0x0C
#define ZIP_LO_crc32 0x0E
#define ZIP_LO_compressed_size 0x12
#define ZIP_LO_uncompressed_size 0x16
#define ZIP_LO_filename_length 0x1A
#define ZIP_LO_extra_field_length 0x1C
#define ZIP_LO_FIXED 0x1E // size of fixed data structure
#define ZIP_LO_filename 0x1E

void time2zip(time_t tod, unsigned& date, unsigned& time);

time_t zip2time(unsigned date, unsigned time);

class zip;

class zip_entry {
public:
	enum method_t {
		unknown,
		store, shrunk, reduce1, reduce2, reduce3, reduce4,
		implode_4kdict_2tree, implode_8kdict_2tree, implode_4kdict_3tree, implode_8kdict_3tree,
		deflate0, deflate1, deflate2, deflate3, deflate4, deflate5, deflate6, deflate7, deflate8, deflate9,
		bzip2, lzma
	};
private:
	struct {
		unsigned version_made_by;
		unsigned host_os;
		unsigned version_needed_to_extract;
		unsigned os_needed_to_extract;
		unsigned general_purpose_bit_flag;
		unsigned compression_method;
		unsigned last_mod_file_time;
		unsigned last_mod_file_date;
		unsigned crc32;
		unsigned compressed_size;
		unsigned uncompressed_size;
		unsigned filename_length;
		unsigned central_extra_field_length;
		unsigned local_extra_field_length;
		unsigned file_comment_length;
		unsigned internal_file_attrib;
		unsigned external_file_attrib;
		unsigned relative_offset_of_local_header;
	} info;

	std::string parent_name; // parent
	unsigned char* file_name;
	unsigned char* file_comment;
	unsigned char* local_extra_field;
	unsigned char* central_extra_field;
	unsigned char* data;

	void check_cent(const unsigned char* buf) const;
	void check_local(const unsigned char* buf) const;
	void check_descriptor(const unsigned char* buf) const;

	zip_entry();
	zip_entry& operator=(const zip_entry&);
	bool operator==(const zip_entry&) const;
	bool operator!=(const zip_entry&) const;

public:
	zip_entry(const zip& Aparent);
	zip_entry(const zip_entry& A);
	~zip_entry();

	void load_local(const unsigned char* buf, FILE* f, unsigned size);
	void save_local(FILE* f);
	void load_cent(const unsigned char* buf, unsigned& skip);
	void save_cent(FILE* f);
	void unload();

	method_t method_get() const;
	void set(method_t method, const std::string& name, const unsigned char* compdata, unsigned compsize, unsigned size, unsigned crc, unsigned date, unsigned time, bool is_text);

	unsigned compressed_size_get() const { return info.compressed_size; }
	unsigned uncompressed_size_get() const { return info.uncompressed_size; }
	unsigned crc_get() const { return info.crc32; }
	bool is_text() const;

	void compressed_seek(FILE* f) const;
	void compressed_read(unsigned char* outdata) const;
	void uncompressed_read(unsigned char* outdata) const;

	const std::string& parentname_get() const { return parent_name; }
	void name_set(const std::string& Aname);
	std::string name_get() const;
	unsigned offset_get() const { return info.relative_offset_of_local_header; }

	unsigned zipdate_get() const { return info.last_mod_file_date; }
	unsigned ziptime_get() const { return info.last_mod_file_time; }
	time_t time_get() const;
	void time_set(time_t tod);

#ifdef USE_COMPRESS
	bool shrink(bool standard, shrink_t level);
#endif

	void test() const;
};

typedef std::list<zip_entry> zip_entry_list;

class zip {
	struct {
		bool open; // zip is opened
		bool read; // zip is loaded (valid only if flag_open==true)
		bool modify; // zip is modified (valid only if flag_read==true)
	} flag;

	struct {
		unsigned offset_to_start_of_cent_dir;
		unsigned zipfile_comment_length;
	} info;

	unsigned char* zipfile_comment;
	zip_entry_list map;
	std::string path;

	zip& operator=(const zip&);
	bool operator==(const zip&) const;
	bool operator!=(const zip&) const;

	void skip_local(const unsigned char* buf, FILE* f);

	static bool pedantic;

	friend class zip_entry;
public:
	static void pedantic_set(bool Apedantic) { pedantic = Apedantic; }

	zip(const std::string& Apath);
	zip(const zip& A);
	~zip();

	typedef zip_entry_list::const_iterator const_iterator;
	typedef zip_entry_list::iterator iterator;

	const_iterator begin() const { assert(flag.open); return map.begin(); }
	const_iterator end() const { assert(flag.open); return map.end(); }
	iterator begin() { assert(flag.open); return map.begin(); }
	iterator end() { assert(flag.open); return map.end(); }

	unsigned size() const { assert(flag.open); return map.size(); }
	unsigned size_not_zero() const;
	bool empty() const { return size_not_zero() == 0; }

	std::string file_get() const { return path; }

	void open();
	void create();
	void close();
	void reopen();
	void save();
	void load();
	void unload();

	bool is_open() const { return flag.open; }
	bool is_load() const { assert(flag.open); return flag.read; }
	bool is_modify() const { assert(flag.open && flag.read); return flag.modify; }

	void erase(iterator i);
	void rename(iterator i, const std::string& Aname);

	iterator insert(const zip_entry& A, const std::string& Aname);
	iterator insert_uncompressed(const std::string& Aname, const unsigned char* data, unsigned size, unsigned crc, time_t tod, bool is_text);

#ifdef USE_COMPRESS
	void shrink(bool standard, shrink_t level);
#endif

	void test() const;
};

#endif

