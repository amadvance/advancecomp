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
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

/** \file
 * Extra types.
 */

#ifndef __EXTRA_H
#define __EXTRA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/* Types */

/** \addtogroup Type */
/*@{*/

/**
 * Type used to check the result of operation.
 *  - ==0 is ok
 *  - <0 is not ok
 *  - >0 special conditions
 */
typedef int adv_error;

/**
 * Type used to check the result of a adv_bool operation.
 *  - ==0 false
 *  - !=0 true
 */
typedef int adv_bool;

typedef unsigned char uint8; /**< Unsigned 8 bit integer. */
typedef signed char int8; /**< Signed 8 bit integer. */
typedef uint16_t uint16; /**< Unsigned 16 bit integer. */
typedef int16_t int16; /**< Signed 16 bit integer. */
typedef uint32_t uint32; /**< Unsigned 32 bit integer. */
typedef int32_t int32; /**< Signed 32 bit integer. */
typedef uint64_t uint64; /**< Unsigned 64 bit integer. */
typedef int64_t int64; /**< Signed 64 bit integer. */
typedef uintptr_t uintptr; /**< Unsigned integer with pointer size. */

/** \name Align
 * Alignment.
 */
/*@{*/
#define ALIGN_BIT 4 /**< Number of bit of alignment required for SSE2. */
#define ALIGN (1U << ALIGN_BIT) /**< Alignment multiplicator. */
#define ALIGN_MASK (ALIGN - 1U) /**< Alignment mask. */

/**
 * Align a unsigned integer at the specified byte size.
 */
static inline uintptr ALIGN_UNSIGNED(uintptr v, unsigned a)
{
	uintptr mask = a - 1;

	return (v + mask) & ~mask;
}

/**
 * Align a void pointer at the specified byte size.
 */
static inline void* ALIGN_PTR(void* v, unsigned a)
{
	return (void*)ALIGN_UNSIGNED((uintptr)v, a);
}
/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
