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

#include "portable.h"

#include "data.h"

#include <new>

using namespace std;

/**
 * Duplicate a memory buffer.
 */
unsigned char* data_dup(const unsigned char* Adata, unsigned Asize)
{
	if (Adata) {
		unsigned char* data = (unsigned char*)malloc(Asize);
		if (!data)
			throw std::bad_alloc();
		if (Asize)
			memcpy(data, Adata, Asize);
		return data;
	} else {
		return 0;
	}
}

/**
 * Allocate a memory buffer.
 */
unsigned char* data_alloc(unsigned size)
{
	unsigned char* data = (unsigned char*)malloc(size);
	if (!data)
		throw std::bad_alloc();
	return data;
}

/**
 * Free a memory buffer.
 */
void data_free(unsigned char* data)
{
	free(data);
}

