/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999, 2000, 2001, 2002, 2003 Andrea Mazzoleni
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

#ifndef __ENDIANRW_H
#define __ENDIANRW_H

#include "extra.h"

/***************************************************************************/
/* Endian */

/** \name Make
 * Make a value in the CPU format from a set of nibble.
 * The nibbles are from the value at lower address to the value at higher address.
 * Note that the values are not masked, if the input values are in overflow the
 * output may be wrong.
 */
/*@{*/
static inline unsigned cpu_uint8_make_uint8(unsigned v0)
{
	return v0;
}

static inline unsigned cpu_uint32_make_uint32(unsigned v0)
{
	return v0;
}

static inline unsigned cpu_uint16_make_uint16(unsigned v0)
{
	return v0;
}

static inline unsigned cpu_uint16_make_uint8(unsigned v0, unsigned v1)
{
#ifdef USE_MSB
	return v1 | v0 << 8;
#else
	return v0 | v1 << 8;
#endif
}

static inline unsigned cpu_uint32_make_uint16(unsigned v0, unsigned v1)
{
#ifdef USE_MSB
	return v1 | v0 << 16;
#else
	return v0 | v1 << 16;
#endif
}

static inline unsigned cpu_uint32_make_uint8(unsigned v0, unsigned v1, unsigned v2, unsigned v3)
{
#ifdef USE_MSB
	return v3 | v2 << 8 | v1 << 16 | v0 << 24;
#else
	return v0 | v1 << 8 | v2 << 16 | v3 << 24;
#endif
}
/*@}*/

/** \name Read
 * Read a value in the Little or Big endian format.
 */
/*@{*/
static inline unsigned le_uint8_read(const void* ptr)
{
	return *(const unsigned char*)ptr;
}

static inline unsigned be_uint8_read(const void* ptr)
{
	return *(const unsigned char*)ptr;
}

static inline unsigned cpu_uint8_read(const void* ptr)
{
	return *(const unsigned char*)ptr;
}

static inline unsigned cpu_uint16_read(const void* ptr)
{
	return *(const uint16*)ptr;
}

static inline unsigned le_uint16_read(const void* ptr)
{
#ifdef USE_LSB
	return cpu_uint16_read(ptr);
#else
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[0] | (unsigned)ptr8[1] << 8;
#endif
}

static inline unsigned be_uint16_read(const void* ptr)
{
#ifdef USE_MSB
	return cpu_uint16_read(ptr);
#else
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[1] | (unsigned)ptr8[0] << 8;
#endif
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

static inline unsigned cpu_uint24_read(const void* ptr)
{
#ifdef USE_LSB
	return le_uint24_read(ptr);
#else
	return be_uint24_read(ptr);
#endif
}

static inline unsigned cpu_uint32_read(const void* ptr)
{
	return *(const uint32*)ptr;
}

static inline unsigned le_uint32_read(const void* ptr)
{
#ifdef USE_LSB
	return cpu_uint32_read(ptr);
#else
	const unsigned char* ptr8 = (const unsigned char*)ptr;
	return (unsigned)ptr8[0] | (unsigned)ptr8[1] << 8 | (unsigned)ptr8[2] << 16 | (unsigned)ptr8[3] << 24;
#endif
}

static inline unsigned be_uint32_read(const void* ptr)
{
#ifdef USE_MSB
	return cpu_uint32_read(ptr);
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
static inline void cpu_uint8_write(void* ptr, unsigned v)
{
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[0] = v;
}

static inline void le_uint8_write(void* ptr, unsigned v)
{
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[0] = v;
}

static inline void be_uint8_write(void* ptr, unsigned v)
{
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[0] = v;
}

static inline void cpu_uint16_write(void* ptr, unsigned v)
{
	uint16* ptr16 = (uint16*)ptr;
	ptr16[0] = v;
}

static inline void le_uint16_write(void* ptr, unsigned v)
{
#ifdef USE_LSB
	cpu_uint16_write(ptr, v);
#else
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[0] = v & 0xFF;
	ptr8[1] = (v >> 8) & 0xFF;
#endif
}

static inline void be_uint16_write(void* ptr, unsigned v)
{
#ifdef USE_MSB
	cpu_uint16_write(ptr, v);
#else
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[1] = v & 0xFF;
	ptr8[0] = (v >> 8) & 0xFF;
#endif
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

static inline void cpu_uint24_write(void* ptr, unsigned v)
{
#ifdef USE_LSB
	le_uint24_write(ptr, v);
#else
	be_uint24_write(ptr, v);
#endif
}

static inline void cpu_uint32_write(void* ptr, unsigned v)
{
	uint32* ptr32 = (uint32*)ptr;
	ptr32[0] = v;
}

static inline void le_uint32_write(void* ptr, unsigned v)
{
#ifdef USE_LSB
	cpu_uint32_write(ptr, v);
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
	cpu_uint32_write(ptr, v);
#else
	unsigned char* ptr8 = (unsigned char*)ptr;
	ptr8[3] = v & 0xFF;
	ptr8[2] = (v >> 8) & 0xFF;
	ptr8[1] = (v >> 16) & 0xFF;
	ptr8[0] = (v >> 24) & 0xFF;
#endif
}

static inline unsigned cpu_uint_read(const void* ptr, unsigned size)
{
	switch (size) {
	default:
	case 1 : return cpu_uint8_read(ptr);
	case 2 : return cpu_uint16_read(ptr);
	case 3 : return cpu_uint24_read(ptr);
	case 4 : return cpu_uint32_read(ptr);
	}
}

static inline void cpu_uint_write(void* ptr, unsigned size, unsigned v)
{
	switch (size) {
	default:
	case 1 : cpu_uint8_write(ptr, v); break;
	case 2 : cpu_uint16_write(ptr, v); break;
	case 3 : cpu_uint24_write(ptr, v); break;
	case 4 : cpu_uint32_write(ptr, v); break;
	}
}

/*@}*/

#endif

/*@}*/

