/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
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

/** \file
 * Endinaess.
 */

/** \addtogroup Endian */
/*@{*/

#ifndef __ENDIAN_H
#define __ENDIAN_H

#include "extra.h"

/***************************************************************************/
/* Endian */

/** \name Read
 * Read a value in the Little or Big endian format.
 */
/*@{*/
static inline unsigned char le_uint8_read(const void* ptr)
{
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned char)ptr8[0];
}

static inline unsigned le_uint16_read(const void* ptr)
{
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[0] | (unsigned)ptr8[1] << 8;
}

static inline unsigned be_uint16_read(const void* ptr)
{
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[1] | (unsigned)ptr8[0] << 8;
}

static inline unsigned le_uint24_read(const void* ptr)
{
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[0] | (unsigned)ptr8[1] << 8 | (unsigned)ptr8[2] << 16;
}

static inline unsigned be_uint24_read(const void* ptr)
{
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[2] | (unsigned)ptr8[1] << 8 | (unsigned)ptr8[0] << 16;
}

static inline unsigned le_uint32_read(const void* ptr)
{
#ifdef USE_LSB
	return *(uint32*)ptr;
#else
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[0] | (unsigned)ptr8[1] << 8 | (unsigned)ptr8[2] << 16 | (unsigned)ptr8[3] << 24;
#endif
}

static inline unsigned be_uint32_read(const void* ptr)
{
#ifdef USE_MSB
	return *(uint32*)ptr;
#else
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[3] | (unsigned)ptr8[2] << 8 | (unsigned)ptr8[1] << 16 | (unsigned)ptr8[0] << 24;
#endif
}
/*@}*/

/** \name Write
 * Write a value in the Little or Big endian format.
 */
/*@{*/
static inline void le_uint8_write(void* ptr, unsigned char v)
{
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[0] = v;
}

static inline void le_uint16_write(void* ptr, unsigned v)
{
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[0] = v & 0xFF;
	ptr8[1] = (v >> 8) & 0xFF;
}

static inline void be_uint16_write(void* ptr, unsigned v)
{
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[1] = v & 0xFF;
	ptr8[0] = (v >> 8) & 0xFF;
}

static inline void le_uint24_write(void* ptr, unsigned v)
{
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[0] = v & 0xFF;
	ptr8[1] = (v >> 8) & 0xFF;
	ptr8[2] = (v >> 16) & 0xFF;
}

static inline void be_uint24_write(void* ptr, unsigned v)
{
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[2] = v & 0xFF;
	ptr8[1] = (v >> 8) & 0xFF;
	ptr8[0] = (v >> 16) & 0xFF;
}

static inline void le_uint32_write(void* ptr, unsigned v)
{
#ifdef USE_LSB
	*(uint32*)ptr = v;
#else
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[0] = v & 0xFF;
	ptr8[1] = (v >> 8) & 0xFF;
	ptr8[2] = (v >> 16) & 0xFF;
	ptr8[3] = (v >> 24) & 0xFF;
#endif
}

static inline void be_uint32_write(void* ptr, unsigned v)
{
#ifdef USE_MSB
	*(uint32*)ptr = v;
#else
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[3] = v & 0xFF;
	ptr8[2] = (v >> 8) & 0xFF;
	ptr8[1] = (v >> 16) & 0xFF;
	ptr8[0] = (v >> 24) & 0xFF;
#endif
}
/*@}*/

#endif

/*@}*/
