#!/bin/sh
#

make distclean

if ! ./configure.windows-x86; then
	exit 1
fi

if ! make check distwindows distclean; then
	exit 1
fi

if ! ./configure.dos; then
	exit 1
fi

if ! make distdos distclean; then
	exit 1
fi

if ! ./configure ; then
	exit 1
fi

if ! make distcheck; then
	exit 1
fi

