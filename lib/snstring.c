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

#include "snstring.h"

/**
 * Copy a string with a size limit.
 * The destination string always has the terminating 0.
 */
void sncpy(char* dst, size_t len, const char* src)
{
	if (len) {
		--len;
		while (len && *src) {
			*dst++ = *src++;
			--len;
		}
		*dst = 0;
#ifndef NDEBUG
		++dst;
		while (len) {
			*dst++ = 0x5A;
			--len;
		}
#endif
	}
}

/**
 * Copy a string with a size limit.
 * The destination string always has the terminating 0.
 */
void sncpyc(char* dst, size_t len, char src)
{
	sncpyn(dst, len, &src, 1);
}

/**
 * Copy a string with a size limit and no more than the specified number of chars.
 * The destination string always has the terminating 0.
 */
void sncpyn(char* dst, size_t len, const char* src, size_t src_len)
{
	if (len < src_len + 1)
		sncpy(dst, len, src);
	else
		sncpy(dst, src_len + 1, src);
}

/**
 * Cat a string with a size limit.
 * The destination string always has the terminating 0.
 */
void sncat(char* dst, size_t len, const char* src)
{
	while (len && *dst) {
		++dst;
		--len;
	}
	sncpy(dst, len, src);
}

/**
 * Cat a string with a size limit.
 * The destination string always has the terminating 0.
 */
void sncatc(char* dst, size_t len, char src)
{
	while (len && *dst) {
		++dst;
		--len;
	}
	sncpyc(dst, len, src);
}


/**
 * Print at the end of a string with a size limit.
 * The destination string always has the terminating 0.
 */
void sncatf(char* str, size_t count, const char* fmt, ...)
{
	unsigned l;
	va_list arg;
	va_start(arg, fmt);

	l = 0;
	while (l<count && str[l])
		++l;

	if (count > l)
		vsnprintf(str + l, count - l, fmt, arg);

	va_end(arg);
}

/**
 * Skip a subset of chars.
 * \param p Index on the string.
 * \param s String to scan.
 * \param sep Set of chars to skip.
 */
void sskip(int* p, const char* s, const char* sep)
{
	while (s[*p] && strchr(sep, s[*p])!=0)
		++*p;
}

/**
 * Extract a token from a string.
 * \param c Separator character. It's cleared in the string to ensure a 0 termination of the token. It's 0 if the token end the string.
 * \param p Index on the string. The index is increased at the end of the token or after the separator if present.
 * \param s String to scan. The string is modified to put the terminating 0 at the end of the token.
 * \param sep Set of chars to use as separators.
 * \param ignore Set of chars to ignore. They are removed at the start and at the end of the token. They are not removed after the separator.
 * \return The extratect token, always 0 terminated.
 */
const char* stoken(char* c, int* p, char* s, const char* sep, const char* ignore)
{
	unsigned begin;
	unsigned end;

	begin = *p;

	while (s[*p] && strchr(sep, s[*p])==0)
		++*p;

	end = *p;

	*c = s[*p];
	if (s[*p]) {
		s[*p] = 0;
		++*p;
	}

	while (begin < end && strchr(ignore, s[begin])!=0) {
		++begin;
	}

	while (begin < end && strchr(ignore, s[end-1])!=0) {
		--end;
		s[end] = 0;
	}

	return s + begin;
}

/**
 * Match a glob pattern to a string.
 * \param s String to compare.
 * \param glob Pattern to use. The glob chars * and ? are allowed. You can prefix
 * these char with a backslash to prevent globbing expansion.
 * \return If the pattern match.
 */
adv_bool sglob(const char* s, const char* glob)
{
	while (*s && *glob) {
		if (*glob == '*') {
			if (sglob(s, glob+1))
				return 1;
			++s;
			continue;
		}

		if (*glob == '?') {
			++glob;
			++s;
			continue;
		}

		if (glob[0] == '\\' && (glob[1] == '\\' || glob[1] == '*' || glob[1] == '?')) {
			++glob;
		}

		if (*glob != *s) {
			return 0;
		}

		++glob;
		++s;
	}

	while (*glob == '*')
		++glob;

	return !*s && !*glob;
}

