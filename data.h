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

#ifndef __DATA_H
#define __DATA_H

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
