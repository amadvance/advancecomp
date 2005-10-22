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

#ifndef __EXCEPT_H
#define __EXCEPT_H

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

class error {
	std::string function;
	std::string file;
	unsigned line;
	std::string desc;
public:
	error() : line(0)
	{
	}

	error(const char* Afunction, const char* Afile, unsigned Aline) : function(Afunction), file(Afile), line(Aline)
	{
	}

	const std::string& desc_get() const
	{
		return desc;
	}

	const std::string& function_get() const
	{
		return function;
	}

	const std::string& file_get() const
	{
		return file;
	}

	unsigned line_get() const
	{
		return line;
	}

	error& operator<<(const char* A)
	{
		desc += A;
		return *this;
	}

	error& operator<<(const std::string& A)
	{
		desc += A;
		return *this;
	}

	error& operator<<(const unsigned A)
	{
		std::ostringstream s;
		s << A /* << " (" << std::hex << A << "h)" */ ;
		desc += s.str();
		return *this;
	}
};

class error_invalid : public error {
public:
	error_invalid() : error()
	{
	}

	error_invalid& operator<<(const char* A)
	{
		error::operator<<(A);
		return *this;
	}

	error_invalid& operator<<(const std::string& A)
	{
		error::operator<<(A);
		return *this;
	}

	error_invalid& operator<<(const unsigned A)
	{
		error::operator<<(A);
		return *this;
	}
};

class error_unsupported : public error {
public:
	error_unsupported() : error()
	{
	}

	error_unsupported& operator<<(const char* A)
	{
		error::operator<<(A);
		return *this;
	}

	error_unsupported& operator<<(const std::string& A)
	{
		error::operator<<(A);
		return *this;
	}

	error_unsupported& operator<<(const unsigned A)
	{
		error::operator<<(A);
		return *this;
	}
};

#define error() \
	error(__PRETTY_FUNCTION__, __FILE__, __LINE__)

static inline std::ostream& operator<<(std::ostream& os, const error& e)
{
	os << e.desc_get();

	if (e.function_get().length() || e.file_get().length() || e.line_get())
		os << " [at " << e.function_get() << ":" << e.file_get() << ":" << e.line_get() << "]";

	return os;
}

#endif

