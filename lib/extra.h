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

/** \defgroup Functionality Functionalities */
/*@{*/
/** \defgroup Error Error */
/** \defgroup Log Log */
/** \defgroup Color Color */
/** \defgroup BitMap BitMap */
/** \defgroup Configuration Configuration */
/** \defgroup Crtc Crtc */
/** \defgroup Generate Crtc Generation */
/** \defgroup Font Font */
/** \defgroup Blit Blit */
/** \defgroup Monitor Monitor */
/** \defgroup Update Update */
/** \defgroup Mode Mode */
/** \defgroup String String */
/** \defgroup Info Info */
/** \defgroup Mixer Mixer */
/*@}*/
/** \defgroup Stream Streams */
/*@{*/
/** \defgroup AudioFile Audio */
/** \defgroup VideoFile Video */
/** \defgroup CompressedFile Compressed */
/** \defgroup ZIPFile ZIP */
/*@}*/
/** \defgroup Driver Drivers */
/*@{*/
/** \defgroup Device Device */
/** \defgroup Video Video */
/** \defgroup Input Input */
/** \defgroup Joystick Joystick */
/** \defgroup Keyboard Keyboard */
/** \defgroup Mouse Mouse */
/** \defgroup Sound Sound */
/*@}*/
/** \defgroup Portable Portable */
/*@{*/
/** \defgroup Type Type */
/** \defgroup System System */
/** \defgroup Target Target */
/** \defgroup File File */
/** \defgroup Endian Endian */
/*@}*/

#ifndef __EXTRA_H
#define __EXTRA_H

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
typedef unsigned short uint16; /**< Unsigned 16 bit integer. */
typedef signed short int16; /**< Signed 16 bit integer. */
typedef unsigned int uint32; /**< Unsigned 32 bit integer. */
typedef signed int int32; /**< Signed 32 bit integer. */
typedef unsigned long long int uint64; /**< Unsigned 64 bit integer. */
typedef signed long long int int64; /**< Signed 64 bit integer. */

/** \name Align
 * Alignment.
 */
/*@{*/

#define ALIGN_BIT 3 /**< Number of bit of alignment required. */
#define ALIGN (1U << ALIGN_BIT) /**< Alignment multiplicator. */
#define ALIGN_MASK (ALIGN - 1U) /**< Alignment mask. */

#define ALIGN_UNCHAINED_BIT 4 /**< Number of bit of alignment required for unchained images. */
#define ALIGN_UNCHAINED (1U << ALIGN_UNCHAINED_BIT) /**< Alignment multiplicator for unchained images. */
#define ALIGN_UNCHAINED_MASK (ALIGN_UNCHAINED - 1U) /**< Alignment mask for unchained images. */

/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
