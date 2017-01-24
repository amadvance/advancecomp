dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.65])
dnl Get version number from git
m4_define([git_revision], m4_esyscmd_s([./autover.sh]))
AC_INIT([AdvanceCOMP], [git_revision], [], [], [http://www.advancemame.it])
AM_INIT_AUTOMAKE([foreign no-dependencies subdir-objects])
AC_CONFIG_SRCDIR([rezip.cc])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

dnl Checks for programs.
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_AWK
AC_CHECK_PROGS(TAR, tar)
AC_CHECK_PROGS(GZIP, gzip)
AC_CHECK_PROGS(GROFF, groff)
AC_CHECK_PROGS(COL, col)
AC_CHECK_PROG([VALGRIND],[valgrind],[valgrind --leak-check=full --track-fds=yes --error-exitcode=1],[])
AC_CHECK_PROG([WINE],[wine],[wine],[])
AC_CHECK_PROG([ADVD2],[advd2],[advd2],[])
AM_CONDITIONAL(HAVE_ADVD2, [test x"$ADVD2" != x])

dnl Checks for libraries.
AC_SYS_LARGEFILE
AC_CHECK_LIB([z], [adler32], [], [AC_MSG_ERROR([the libz library is missing])])

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS([unistd.h getopt.h utime.h stdarg.h varargs.h stdint.h])
AC_CHECK_HEADERS([sys/types.h sys/stat.h sys/time.h sys/utime.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

dnl Checks for library functions.
AC_CHECK_FUNCS([getopt getopt_long snprintf vsnprintf])

AC_ARG_ENABLE(
	bzip2,
	[  --enable-bzip2       Use the bzip2 compression],
	AC_DEFINE(USE_BZIP2,1,[Use the bzip2 compression])
	AC_CHECK_LIB([bz2], [BZ2_bzBuffToBuffCompress], [], [AC_MSG_ERROR([the libbz2 library is missing])])
)

dnl Checks for test environment
AS_CASE([$host], [*-*-mingw*], [TESTENV="$WINE"], [])

AC_ARG_ENABLE([valgrind],
	[AS_HELP_STRING([--enable-valgrind],[enable the use of valgrind in testing])],
	[
	TESTENV="$VALGRIND"
	CFLAGS="$CFLAGS -DCHECKER"
	CXXFLAGS="$CXXFLAGS -DCHECKER"
	],
	[])

AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],[enable debugging])],
	[
	TESTENV=""
	CFLAGS="-O0 -g -Wall -Wextra"
	CXXFLAGS="-O0 -g -Wall -Wextra"
	],
	[])

dnl Configure the library
CFLAGS="$CFLAGS -DUSE_ERROR_SILENT -DUSE_COMPRESS"
CXXFLAGS="$CXXFLAGS -DUSE_ERROR_SILENT -DUSE_COMPRESS"

AC_ARG_VAR([TESTENV], [Test environment])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

