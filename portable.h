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

#ifndef __PORTABLE_H
#define __PORTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_CONFIG_H
#include "config.h" /* Use " to include first in the same directory of this file */
#endif

/* Include some standard headers */
#include <stdio.h>
#include <stdlib.h> /* On many systems (e.g., Darwin), `stdio.h' is a prerequisite. */
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

#if !HAVE_GETOPT
int getopt(int argc, char * const *argv, const char *options);
extern char *optarg;
extern int optind, opterr, optopt;
#endif

#if HAVE_GETOPT_LONG
#define SWITCH_GETOPT_LONG(a, b) a
#else
#define SWITCH_GETOPT_LONG(a, b) b
#endif

#if HAVE_UTIME_H
#include <utime.h>
#endif
#if HAVE_SYS_UTIME_H
#include <sys/utime.h>
#endif

#if defined(__MSDOS__) || defined(__WIN32__)
#define HAVE_LONG_FNAME 0
#define DIR_SEP ';'
#else
#define HAVE_LONG_FNAME 1
#define DIR_SEP ':'
#endif

#if defined(__WIN32__)
#define HAVE_FUNC_MKDIR_ONEARG 1
#else
#define HAVE_FUNC_MKDIR_ONEARG 0
#endif

#if defined(__WIN32__)
#define HAVE_SIGHUP 0
#define HAVE_SIGQUIT 0
#else
#define HAVE_SIGHUP 1
#define HAVE_SIGQUIT 1
#endif

#if !HAVE_SNPRINTF
int snprintf(char *str, size_t count, const char *fmt, ...);
#endif

#if !HAVE_VSNPRINTF
#if HAVE_STDARG_H
#include <stdarg.h>
#else
#if HAVE_VARARGS_H
#include <varargs.h>
#endif
#endif
int vsnprintf(char *str, size_t count, const char *fmt, va_list arg);
#endif

#ifdef __cplusplus
}
#endif

#endif

