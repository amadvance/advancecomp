# Process this file with autoconf to produce a configure script.
AC_INIT([AdvanceCOMP],[1.15])
AC_CONFIG_SRCDIR([rezip.cc])
AM_INIT_AUTOMAKE([foreign no-dependencies])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_AWK
AC_CHECK_PROGS(TAR, tar)
AC_CHECK_PROGS(GZIP, gzip)
AC_CHECK_PROGS(GROFF, groff)
AC_CHECK_PROGS(COL, col)

# Checks for libraries.
AC_CHECK_LIB([z], [adler32], [], [
AC_MSG_ERROR([the zlib library is required])
])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS([unistd.h getopt.h utime.h stdarg.h varargs.h stdint.h])
AC_CHECK_HEADERS([sys/types.h sys/stat.h sys/time.h sys/utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([getopt getopt_long snprintf vsnprintf])

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

