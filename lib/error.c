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

#include "portable.h"

#include "extra.h"

#ifndef USE_ERROR_SILENT
#include "log.h"
#endif

#include "error.h"
#include "snstring.h"

/****************************************************************************/
/* Error */

/**
 * Max length of the error description.
 */
#define ERROR_DESC_MAX 2048

/**
 * Last error description.
 */
static char error_desc_buffer[ERROR_DESC_MAX];

/**
 * Flag set if an unsupported feature is found.
 */
static adv_bool error_unsupported_flag;

/**
 * Flag for cat mode.
 */
static adv_bool error_cat_flag;

/**
 * Prefix for cat mode.
 */
static char error_cat_prefix_buffer[ERROR_DESC_MAX];

/**
 * Set the error cat mode.
 * If enabled the error text is appended at the end of the previous error.
 */
void error_cat_set(const char* prefix, adv_bool mode)
{
	if (prefix)
		sncpy(error_cat_prefix_buffer, sizeof(error_cat_prefix_buffer), prefix);
	else
		error_cat_prefix_buffer[0] = 0;
	error_cat_flag = mode;
}

/**
 * Get the current error description.
 */
const char* error_get(void)
{
	/* remove the trailing \n */
	while (error_desc_buffer[0] && isspace(error_desc_buffer[strlen(error_desc_buffer)-1]))
		error_desc_buffer[strlen(error_desc_buffer)-1] = 0;
	return error_desc_buffer;
}

/**
 * Reset the description of the last error.
 */
void error_reset(void)
{
	error_unsupported_flag = 0;
	error_desc_buffer[0] = 0;
}

/**
 * Set the description of the last error.
 * The previous description is overwritten.
 * \note The description IS logged.
 */
void error_set(const char* text, ...)
{
	va_list arg;
	char* p;
	unsigned size;

	error_unsupported_flag = 0;

	if (error_cat_flag) {
		if (error_cat_prefix_buffer[0]) {
			sncat(error_desc_buffer, sizeof(error_desc_buffer), error_cat_prefix_buffer);
			sncat(error_desc_buffer, sizeof(error_desc_buffer), ": ");
		}
		p = error_desc_buffer + strlen(error_desc_buffer);
		size = sizeof(error_desc_buffer) - strlen(error_desc_buffer);
	} else {
		p = error_desc_buffer;
		size = sizeof(error_desc_buffer);
	}

	va_start(arg, text);
	vsnprintf(p, size, text, arg);

#ifndef USE_ERROR_SILENT
	log_std(("advance:msg:"));
	if (error_cat_flag && error_cat_prefix_buffer[0]) {
		log_std(("%s:", error_cat_prefix_buffer));
	}
	log_std((" "));
	log_va(text, arg);
	if (!text[0] || text[strlen(text)-1] != '\n')
		log_std(("\n"));
#endif

	va_end(arg);
}

/**
 * Set the description of the last error due unsupported feature.
 * \note The description IS logged.
 */
void error_unsupported_set(const char* text, ...)
{
	va_list arg;

	error_unsupported_flag = 1;

	va_start(arg, text);
	vsnprintf(error_desc_buffer, sizeof(error_desc_buffer), text, arg);

#ifndef USE_ERROR_SILENT
	log_std(("advance: set_error_description \""));
	log_va(text, arg);
	log_std(("\"\n"));
#endif

	va_end(arg);
}

/**
 * Check if a unsupported feature is found.
 * This function can be called only if another function returns with error.
 * \return
 *  - ==0 Not found.
 *  - !=0 Unsupported feature found.
 */
adv_bool error_unsupported_get(void)
{
	return error_unsupported_flag;
}

#ifndef USE_ERROR_SILENT
/**
 * Set the error description.
 * The previous description is overwritten.
 * The description is not logged.
 */
void error_nolog_set(const char* text, ...)
{
	va_list arg;
	char* p;
	unsigned size;

	error_unsupported_flag = 0;

	if (error_cat_flag) {
		if (error_cat_prefix_buffer[0]) {
			sncat(error_desc_buffer, sizeof(error_desc_buffer), error_cat_prefix_buffer);
			sncat(error_desc_buffer, sizeof(error_desc_buffer), ": ");
		}
		p = error_desc_buffer + strlen(error_desc_buffer);
		size = sizeof(error_desc_buffer) - strlen(error_desc_buffer);
	} else {
		p = error_desc_buffer;
		size = sizeof(error_desc_buffer);
	}

	va_start(arg, text);
	vsnprintf(p, size, text, arg);
	va_end(arg);
}

#endif

