/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1998-2002 Andrea Mazzoleni
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

#ifndef __PORTABLE_H
#define __PORTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* ------------------------------------------------------------------------ */
/* getopt */

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if !HAVE_GETOPT
int getopt(int argc, char * const *argv, const char *options);
extern char *optarg;
extern int optind, opterr, optopt;
#endif

#if HAVE_GETOPT_LONG
#define SWITCH_GETOPT_LONG(a,b) a
#else
#define SWITCH_GETOPT_LONG(a,b) b
#endif

/* ------------------------------------------------------------------------ */
/* utime */

#include <time.h>

#if HAVE_UTIME_H
#include <utime.h>
#endif

#if HAVE_SYS_UTIME_H
#include <sys/utime.h>
#endif

/* ------------------------------------------------------------------------ */
/* directory separator */

#if defined(__MSDOS__) || defined(__WIN32__)
#define DIR_SEP ';'
#else
#define DIR_SEP ':'
#endif

/* ------------------------------------------------------------------------ */
/* mkdir */

#if defined(__WIN32__)
#define HAVE_FUNC_MKDIR_ONEARG
#endif

// ------------------------------------------------------------------------
// signal

#if !defined(__WIN32__)
#define HAVE_SIGHUP
#define HAVE_SIGQUIT
#endif

// ------------------------------------------------------------------------
// safestring

#if !HAVE_SNPRINTF
#include <sys/types.h>
int snprintf(char *str, size_t count, const char *fmt, ...);
#else
#include <stdio.h>
#endif

#if !HAVE_VSNPRINTF
#if HAVE_STDARG_H
#include <stdarg.h>
#else
#if HAVE_VARARGS_H
#include <varargs.h>
#endif
#endif
#include <sys/types.h>
int vsnprintf(char *str, size_t count, const char *fmt, va_list arg);
#else
#include <stdio.h>
#endif

#ifdef __cplusplus
}
#endif

#endif

