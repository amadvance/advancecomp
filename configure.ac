# Process this file with autoconf to produce a configure script.
AC_INIT([AdvanceCOMP],[1.6])
AC_CONFIG_SRCDIR([rezip.cc])
AM_INIT_AUTOMAKE([foreign no-dependencies])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_CHECK_PROGS(TAR, tar)
AC_CHECK_PROGS(GZIP, gzip)
AC_CHECK_PROGS(GROFF, groff)
AC_CHECK_PROGS(COL, col)

# Checks for libraries.
AC_CHECK_LIB([z], [adler32])

# Checks for header files.
AC_CHECK_HEADERS([getopt.h])
AC_CHECK_HEADERS([utime.h])
AC_CHECK_HEADERS([sys/utime.h])
AC_CHECK_HEADERS([stdarg.h])
AC_CHECK_HEADERS([varargs.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([getopt])
AC_CHECK_FUNCS([getopt_long])
AC_CHECK_FUNCS([snprintf])
AC_CHECK_FUNCS([vsnprintf])

AC_DEFINE(USE_7Z,1,[Use the 7z library])
AC_DEFINE(USE_LZMA,1,[Use the lzma compression])

AC_ARG_ENABLE(
	bzip2,
	[  --enable-bzip2       Use the bzip2 compression],
	AC_DEFINE(USE_BZIP2,1,[Use the bzip2 compression])
	AC_CHECK_LIB([bz2],[BZ2_bzBuffToBuffCompress])
)

# Configure the library
CFLAGS="$CFLAGS -DUSE_ERROR_SILENT -DUSE_COMPRESS"
CXXFLAGS="$CXXFLAGS -DUSE_ERROR_SILENT -DUSE_COMPRESS"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

